#include "./headers/pcb.h"

static struct list_head pcbFree_h;
static pcb_t pcbFree_table[MAXPROC];
static int next_pid = 1;

void initPcbs()
{
  //initiallizzo pcbFree_h
  INIT_LIST_HEAD(&pcbFree_h);

  /*
   *aggiungo ogni puntatore p_list dei pcb contenuti in pcbFree_table alla lista
   dei pcb inattivi
  */
  for(int c=0; c<MAXPROC; c++)
  {
    list_add(&pcbFree_table[c].p_list, &pcbFree_h);
  }
}

void freePcb(pcb_t* p)
{
  //aggiungo il processo p alla lista dei processi inattivi
  list_add(&p->p_list, &pcbFree_h);
}

pcb_t* allocPcb()
{
  /*
   * se non ci sono pcb intattivi non posso allocarne uno nuovo
   * se c'è almeno un pcb inattivo, lo rimuovo dalla lista e lo inizializzo
   * lo rimuovo dalla lista dei pcb inattivi e ritorno un puntatore ad esso
  */

  if(list_empty(&pcbFree_h))
    return NULL;
  else
  {
    struct list_head *entry=pcbFree_h.next;

    list_del(entry);
    pcb_t *p=container_of(entry, pcb_t, p_list);

    p->p_pid=next_pid++;
    p->p_parent=NULL;

    INIT_LIST_HEAD(&p->p_child);
    INIT_LIST_HEAD(&p->p_sib);

    p->p_time=0;
    p->p_semAdd=NULL;
    p->p_supportStruct=NULL;
    p->p_prio=0;

    p->p_s.entry_hi=0;
    p->p_s.cause=0;
    p->p_s.status=0;
    p->p_s.pc_epc=0;
    p->p_s.mie=0;

    for(int c=0; c<STATE_GPR_LEN; c++)
    {
      p->p_s.gpr[c]=0;
    }

    return p;
  }
}

void mkEmptyProcQ(struct list_head* head)
{
  /*
   * inizializzo semplicemente la variabile head in modo che possa diventare l'elemento sentinel
   * di una nuova lista
  */
  INIT_LIST_HEAD(head);
}

int emptyProcQ(struct list_head* head)
{
  //ritorna true se head è una lista vuota, altrimenti ritorna false
  return list_empty(head);
}

void insertProcQ(struct list_head* head, pcb_t* p)
{
  /*
   * Esamina la lista cercando un elemento con priorità più bassa.
   * Se trovato, inserisce il PCB dato prima di quest'ultimo.
   */
  struct list_head* iter;
  list_for_each(iter, head)
  {
    /* Recupera il PCB che contiene l'elemento corrente della lista */
    pcb_t* item = container_of(iter, pcb_t, p_list);
    if (item->p_prio < p->p_prio)
    {
        list_add(&p->p_list, iter->prev);
        return;
    }
  }
  /* Se non viene trovato alcun elemento con priorità inferiore, aggiunge il PCB in coda */
  list_add_tail(&p->p_list, head);
}

pcb_t* headProcQ(struct list_head* head)
{
  /* Se la lista è vuota, restituisce NULL */
  if(list_empty(head)){return NULL;}
  /* Altrimenti, restituisce il puntatore al primo PCB */
  pcb_t* p = container_of(head->next, pcb_t, p_list);
  return p;
}

pcb_t* removeProcQ(struct list_head* head)
{
  /* Se la lista è vuota, restituisce NULL */
  if(list_empty(head)){return NULL;}
  /* Salva un puntatore al primo elemento prima di rimuoverlo */
  struct list_head *entry=head->next;
  list_del(entry);
  /* Recupera il PCB dell'elemento rimosso */
  pcb_t *p=container_of(entry, pcb_t, p_list);
  return p;
}

pcb_t* outProcQ(struct list_head* head, pcb_t* p)
{
  /*
   * Scorre la lista alla ricerca di un PCB uguale a quello fornito.
   * Se lo trova, rimuove l'elemento dalla lista.
   */
  struct list_head* iter;
  list_for_each(iter, head)
  {
    pcb_t* item = container_of(iter, pcb_t, p_list);
    if (item==p)
    {
      list_del(iter);
      return p;
    }
  }
  /* Se non viene trovato, restituisce NULL */
  return NULL;
}

int emptyChild(pcb_t* p)
{
  if(p->p_child.next == &(p->p_child)){
    return TRUE;
  }
  return FALSE;
}

void insertChild(pcb_t* prnt, pcb_t* p)
{
  if(prnt == NULL || p == NULL){
    //caso NULL
    return;
  }  

  p->p_parent = prnt;
  struct list_head *sblng = prnt->p_child.next;
  p->p_sib.next = sblng; 
  p->p_sib.prev = &(prnt->p_child); 
  prnt->p_child.next = &(p->p_sib); 
  sblng->prev = &(p->p_sib); 
}

pcb_t* removeChild(pcb_t* p)
{
  if(p == NULL || emptyChild(p)){
    return NULL;
  }

  pcb_t *removed_child = container_of(p->p_child.next, pcb_t, p_sib);
  struct list_head *prnt = removed_child->p_sib.prev;
  struct list_head *sblng = removed_child->p_sib.next;
  prnt->next = sblng;
  sblng->prev = prnt;

  removed_child->p_parent = NULL;
  removed_child->p_sib.next = &(removed_child->p_sib);
  removed_child->p_sib.prev = &(removed_child->p_sib);
  
  return removed_child;
}

pcb_t* outChild(pcb_t* p)
{ 
  if(p == NULL || p->p_parent == NULL){ 
    return NULL;
  }

  struct list_head *prnt = p->p_sib.prev;
  struct list_head *sblng = p->p_sib.next;
  prnt->next = sblng;
  sblng->prev = prnt;

  p->p_parent = NULL;
  p->p_sib.next = &(p->p_sib);
  p->p_sib.prev = &(p->p_sib);

  return p;
}
