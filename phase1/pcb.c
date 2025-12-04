#include "./headers/pcb.h"

static struct list_head pcbFree_h;
static pcb_t pcbFree_table[MAXPROC];
static int next_pid = 1;

void initPcbs()
{
  pcbFree_h = LIST_HEAD_INIT(pcbFree_h);

    struct list_head *pos;
    list_for_each(pos, &pcbFree_h)
    {
      kitem_t *item=container_of(pos, kitem_t, list);
      list_add(pos,pcbFree_h);
    }

}

void freePcb(pcb_t* p)
{
  list_add(p->p_list, pcbFree_h);
}

pcb_t* allocPcb()
{
  if(list_empty(pcbFree_h))
    return NULL;
  else
  {
    /*
     * da finire, non capisco cosa si intenda con
     * "provide initial values for all of the PCBs fields"
    */
    
    //penso si debbano usare le funzioni list_for_each() o list_for_each_entry()
    //a seguire list_del()
    
    struct list_head *pos;
    list_for_each(pos, &pcbFree_h)
    {
      kitem_t *item=container_of(pos,kitem_t,pcbFree_h);
      printf("Elemento i-esimo %d\n",item->elem);
    }

    list_del(pos);
    return pos;
  }
}

void mkEmptyProcQ(struct list_head* head)
{

}

int emptyProcQ(struct list_head* head)
{
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
