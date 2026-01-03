#include "./headers/asl.h"

static semd_t semd_table[MAXPROC];
static struct list_head semdFree_h;
static struct list_head semd_h;

void initASL()
{
  INIT_LIST_HEAD(&semdFree_h);
  INIT_LIST_HEAD(&semd_h);

  for(int c=0;c<MAXPROC;c++)
  {
    list_add(&(semd_table[c].s_link), &semdFree_h);
    semd_table[c].s_key=NULL;
    INIT_LIST_HEAD(&semd_table[c].s_procq);
  }
}

int insertBlocked(int* semAdd, pcb_t* p)
{
  struct list_head *iter;
  int found=0;

  list_for_each(iter, &semd_h)
  {
    semd_t *item=container_of(iter, semd_t, s_link);
    if(*(item->s_key)==*semAdd)
    {
      list_add_tail(&p->p_list, &item->s_procq);
      found=1;
      return 0;
    }
  }

  if(found==0)
  {
    if(list_empty(&semdFree_h))
      return 1;
    else
    {
      struct list_head *entry=semdFree_h.next;
      list_del(entry);

      semd_t descriptor;
      descriptor.s_link=*entry;
      INIT_LIST_HEAD(&descriptor.s_procq);
      entry=NULL;

      descriptor.s_key=semAdd;
      list_add(&(descriptor.s_link), &semd_h);
      list_add_tail(&p->p_list, &descriptor.s_procq);
      return 0;
    }
  }
}

pcb_t* removeBlocked(int* semAdd)
{
  struct list_head *iter;

  list_for_each(iter, &semd_h)
  {
    semd_t *item=container_of(iter, semd_t, s_link);
    if(*(item->s_key)==*semAdd)
    {
      struct list_head *free=(&item->s_procq);
      list_del(free);

      if(list_empty(&item->s_procq))
      {
        list_del(iter);
        list_add(iter, &semdFree_h);
      }

      pcb_t *p=container_of(free,pcb_t,p_list);

      return p;
    }

    return NULL;
  }
}

pcb_t* outBlocked(pcb_t* p)
{
}

pcb_t* headBlocked(int* semAdd)
{
}
