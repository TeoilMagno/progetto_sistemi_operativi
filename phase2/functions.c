#include "./headers/functions.h"

void copyState(state_t *dep, state_t *arr)
{
  dep->entry_hi=arr->entry_hi;
  dep->cause=arr->cause;
  dep->status=arr->status;
  dep->pc_epc=arr->pc_epc;
  dep->mie=arr->mie;
}

int findDeviceIndex(memaddr deviceAddr)
{
  //0x10 equivale alla dimensione in memoria che occupa ogni device
  unsigned int bottom = (unsigned int)deviceAddr - START_DEVREG;

  //-1 in caso di errore
  int deviceIndex = -1;

  //controllo se il device è un terminale, in caso ogni terminale è suddiviso in due subdevice
  //ognuno che occupa 0x8
  if(bottom >= 32*0x10)
    deviceIndex = 32 + (bottom-(32*0x10)/0x8);
  else //non è un terminale
    deviceIndex = bottom/0x10;

  if(deviceIndex<0 || deviceIndex>49)
    return -1; //errore dispositivo non valido
  
  return deviceIndex;
}

pcb_t* findProcess(int pid)
{
  if(currentProcess!=NULL && currentProcess->p_pid==pid)
  {
    return currentProcess;
  }
  
  struct list_head* iter;
  list_for_each(iter, &readyQueue)
  {
    pcb_t* item = container_of(iter, pcb_t, p_list);
    if (item->p_pid==pid)
    {
      return item;
    }
  }
  return findBlockedPcb(pid);  
}

void killProcess(pcb_t* pcb)
{
  struct list_head *iter;
  list_for_each(iter, &pcb->p_child)
  {
    pcb_t* item = container_of(iter, pcb_t, p_list);
    killProcess(item);
  }
  outChild(pcb);
  outProcQ(&readyQueue, pcb);
  outBlocked(pcb);
  freePcb(pcb);
}

int updateTime(int pid)
{
  cpu_t currentTime;
  STCK(currentTime);
  cpu_t time=currentTime-startTime[pid];
  return pid;
}
