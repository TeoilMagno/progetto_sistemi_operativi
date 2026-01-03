#include "./headers/asl.h"
#include "./headers/pcb.h"

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
  list_for_each(iter, &semd_h)
  {
    semd_t *item=container_of(iter, semd_t, s_link);
    if(item->s_key==semAdd)
    {
      list_add_tail(&p->p_list, &item->s_procq);
      p->p_semAdd=semAdd;
      return 0;
    }
  }
  if(list_empty(&semdFree_h)){return 1;}
  
  struct list_head *entry=semdFree_h.next;
  list_del(entry);

  semd_t *descriptor = container_of(entry, semd_t, s_link);
  INIT_LIST_HEAD(&descriptor->s_procq);
  descriptor->s_key=semAdd;
  list_add(&descriptor->s_link, &semd_h);
  list_add_tail(&p->p_list, &descriptor->s_procq);
  p->p_semAdd=semAdd;
  
  return 0;
}

pcb_t* removeBlocked(int* semAdd)
{
  /* 
   * come definito dal progetto, un semaforo è attivo se contiene almeno un
   * processo nella s_procq
   * se semd_h risulta vuota allora non eiste un semaforo contenente con s_key==semAdd
   * tale che esso contenga un processo da rimuovere da s_procq
  */
  if(list_empty(&semd_h)){return NULL;}

  struct list_head *iter;

  list_for_each(iter, &semd_h)
  {
    semd_t *item=container_of(iter, semd_t, s_link);
    if(item->s_key==semAdd)
    {
      pcb_t *p=removeProcQ(&item->s_procq);

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
  */
  if(list_empty(&semd_h)){return NULL;}

  struct list_head *iter;

  //cerco nei semafori attivi quello con s_key == a p_semAdd
  //se lo trovo lo rimuovo con la funzione già implementata in prc.c
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
