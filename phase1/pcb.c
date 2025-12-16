#include "./headers/pcb.h"

static struct list_head pcbFree_h;
static pcb_t pcbFree_table[MAXPROC];
static int next_pid = 1;

void initPcbs()
{
  INIT_LIST_HEAD(&pcbFree_h);

  for(int c=0; c<MAXPROC; c++)
  {
    list_add(&(pcbFree_table[c].p_list), &pcbFree_h);
  }
}

void freePcb(pcb_t* p)
{
  list_add(&p->p_list, &pcbFree_h);
}

pcb_t* allocPcb()
{
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

    //todo: inizializzare i valori di p_s, non so quali siano i valori corretti
    // memset(&p->p_s, 0, sizeof(state_t));

    return p;
  }
}

void mkEmptyProcQ(struct list_head* head)
{
  INIT_LIST_HEAD(head);
}

int emptyProcQ(struct list_head* head)
{
  return list_empty(head);
}

void insertProcQ(struct list_head* head, pcb_t* p)
{
}

pcb_t* headProcQ(struct list_head* head)
{
}

pcb_t* removeProcQ(struct list_head* head)
{
}

pcb_t* outProcQ(struct list_head* head, pcb_t* p)
{
}

int emptyChild(pcb_t* p)
{
}

void insertChild(pcb_t* prnt, pcb_t* p)
{
}

pcb_t* removeChild(pcb_t* p)
{
}

pcb_t* outChild(pcb_t* p)
{
}
