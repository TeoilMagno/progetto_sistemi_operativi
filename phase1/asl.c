#include "./headers/asl.h"
#include "./headers/pcb.h"

static semd_t semd_table[MAXPROC];
static struct list_head semdFree_h;
static struct list_head semd_h;

void initASL()
{
  /*
   * inizializzo le due liste semd_h e semdFree_h
  */
  INIT_LIST_HEAD(&semdFree_h);
  INIT_LIST_HEAD(&semd_h);

  /*
   * aggiungo ogni semaforo contenuto in semd_table alla lista dei semafori inattivi
   * e inizializzo i campi dei semafori
  */
  for(int c=0;c<MAXPROC;c++)
  {
    list_add(&semd_table[c].s_link, &semdFree_h);
    semd_table[c].s_key=NULL;
    INIT_LIST_HEAD(&semd_table[c].s_procq);
  }
}

int insertBlocked(int* semAdd, pcb_t* p)
{
  /*
   * se esiste un semaforo attivo con s_key== semAdd aggiungo alla lista dei processi bloccati
   * il processo p ed aggiorno la variabile semAdd del processo p in modo che punti al semaforo in cui
   * il processo è contenuto
  */
  struct list_head *iter;
  list_for_each(iter, &semd_h)
  {
    semd_t *item=container_of(iter, semd_t, s_link);
    if(item->s_key==semAdd)
    {
      list_add_tail(&p->p_list, &item->s_procq);
      p->p_semAdd=semAdd;
      return FALSE;
    }
  }

  /*
   * se non esistono semafori attivi con valore s_key==semAdd controllo se ne esiste
   * almeno uno inattivo che io possa inizializzare per contenere il processo p,
   * in questo caso il semaforo viene spostato dalla lista semdFree_h alla semd_h
   * e viene aggiunto alla sua coda s_procq il processo p
  */
  if(list_empty(&semdFree_h))
    //ERRORE! non esiste un semaforo inattivo a cui posso aggiungere il processo p
    return TRUE;
  
  struct list_head *entry=semdFree_h.next;
  list_del(entry);

  semd_t *descriptor = container_of(entry, semd_t, s_link);
  INIT_LIST_HEAD(&descriptor->s_procq);
  descriptor->s_key=semAdd;
  list_add(&descriptor->s_link, &semd_h);
  list_add_tail(&p->p_list, &descriptor->s_procq);
  p->p_semAdd=semAdd;
  
  return FALSE;
}

pcb_t* removeBlocked(int* semAdd)
{
  /* 
   * come definito dal progetto, un semaforo è attivo se contiene almeno un
   * processo nella s_procq
   * se semd_h risulta vuota allora non eiste un semaforo contenente s_key==semAdd
   * tale che esso contenga un processo da rimuovere da s_procq
  */
  if(list_empty(&semd_h))
    return NULL;

  /*
   * se invece la lista semd_h non è vuota cerco un semaforo tale che s_key==semAdd
   * se lo trovo rimuovo il processo in testa alla coda dei processi bloccati e ritorno un
   * puntatore ad esso
  */
  struct list_head *iter;

  list_for_each(iter, &semd_h)
  {
    semd_t *item=container_of(iter, semd_t, s_link);
    if(item->s_key==semAdd)
    {
      pcb_t *p=removeProcQ(&item->s_procq);

      /*
      * se la lista dei processi bloccati diventa vuoto dopo la rimozione del processo
      * allora il semaforo diventa inattivo e viene spostato nella semdFree_h
      */
      if(list_empty(&item->s_procq))
      {
        list_del(iter);
        list_add(iter, &semdFree_h);
      }

      return p;
    }
  }
  return NULL;
}

pcb_t* outBlocked(pcb_t* p)
{
  /* 
   * come definito dal progetto, un semaforo è attivo se contiene almeno un
   * processo nella s_procq
   * se semd_h risulta vuota allora non eiste un semaforo contenente p in s_procq
   * perchè non esiste neanche un semaforo attivo
  */
  if(list_empty(&semd_h))
    return NULL;

  struct list_head *iter;

  /*
   * cerco nei semafori attivi quello con s_key==a p_semAdd
   * se lo trovo lo rimuovo con la funzione già implementata in pcb.c
  */
  list_for_each(iter, &semd_h)
  {
    semd_t *item=container_of(iter, semd_t, s_link);
    if(item->s_key==p->p_semAdd)
    {
      return outProcQ(&item->s_procq, p);
    }
  }

  return NULL;
}

pcb_t* headBlocked(int* semAdd)
{
  /*
   * cerco un semaforo con il campo s_key==semadd
   * se lo trovo ritorno il puntatore al processo in testa alla lista dei processi
   * bloccati, in caso di errore ritorno NULL;
  */
  if(list_empty(&semd_h))
    return NULL;

  struct list_head *iter;

  list_for_each(iter, &semd_h)
  {
    semd_t *item=container_of(iter, semd_t, s_link);

    if(item->s_key==semAdd)
    {
      return headProcQ(&item->s_procq);
    }
  }

  return NULL;
}
