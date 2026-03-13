#include "./headers/exceptions.h"

void exceptionHandler()
{
  unsigned int cause = getCAUSE();
  unsigned int causeCode = cause && CAUSE_EXCCODE_MASK;
  state_t *state = (state_t *) GET_EXCEPTION_STATE_PTR(getPRID());

  if(CAUSE_IS_INT(cause))
  {
    interruptHandler(state);
  }
  else
  {
    switch(causeCode)
    {
      case 8:
        //SYSCALL
        break;
      
      case 11:
        //SYSCALL
        break;
      
      case 24:
        //TLB
        break;
      
      case 25:
        //TLB
        break;
      
      case 26:
        //TLB
        break;
      
      case 27:
        //TLB
        break;
      
      case 28:
        //TLB
        break;
      
      default:
        //Program Trap
        break;
    }
  }
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

void syscallHandler(state_t *state)
{
  switch(state->reg_a0)
  { 
    case -1:
    {
      pcb_t *newPcb = allocPcb();
      if(newPcb==NULL)
      {
        state->reg_a0 = -1;
      }
      else
      {
        state->reg_a0 = newPcb->p_pid;
        state_t *sreg_a1 = (state_t *) state->reg_a1;    
        for(int i=0; i<STATE_GPR_LEN; i++)
        {
          newPcb->p_s.gpr[i]=sreg_a1->gpr[i];
        }
        newPcb->p_s.entry_hi=sreg_a1->entry_hi;
        newPcb->p_s.cause=sreg_a1->cause;
        newPcb->p_s.status=sreg_a1->status;
        newPcb->p_s.pc_epc=sreg_a1->pc_epc;
        newPcb->p_s.mie=sreg_a1->mie;
        newPcb->p_prio=state->reg_a2;
        newPcb->p_supportStruct=(support_t *) state->reg_a3;
        insertChild(currentProcess, newPcb);
        insertProcQ(&readyQueue, newPcb);
        processCount++;
        LDST(state);
      }
      break;
    }
    case -2:
    {
      if(state->reg_a1==0)
      {
        killProcess(currentProcess);
        scheduler();
      }
      else 
      {
        killProcess(findProcess(state->reg_a1));
        scheduler();
      }
      break;
    }
    case -3:
    {
      int *semAdd = (int *)state->reg_a1;
      if(*semAdd<=0)
      {
        insertBlocked(semAdd, currentProcess);
        scheduler();
      }
      else
      {
        *semAdd += -1;
        LDST(state);
      }
      break;
    }
    case -4:
    {
      int *semAdd = (int *)state->reg_a1;
      if(*semAdd<=0)
      {
        pcb_t* unlockedProcess = removeBlocked(semAdd);
        if (unlockedProcess != NULL)
        {
          insertProcQ(&readyQueue, unlockedProcess);
        }
        else
        {
          *semAdd += 1;
        }
      }
      LDST(state);
      break;
    }
    case -5:
      break;
    case -6:
      break;
    case -7:
      break;
    case -8:
      break;
    case -9:
      break;
    case -10:
      break;
    default:
      break;
  }
}
