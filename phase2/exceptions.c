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
    {
      int *commandAddr = (int *)state->reg_a1;
      int value = state->reg_a2;
      *commandAddr = value;
      int* semPtr = &deviceSemaphore[findDeviceIndex(*commandAddr)];
      (*semPtr)--;
      break;
    }
    case -6:
    {
      int pid=getPRID();
      cpu_t currentTime;
      STCK(currentTime);
      cpu_t time=currentTime-startTime[pid];
      state->reg_a0 = findProcess(pid)->p_time+time;
      LDST(state);
      break;
    }
    case -7:
    {
      int *pseudoClock = &deviceSemaphore[47];
      insertBlocked(pseudoClock, currentProcess);
      scheduler();
      break;
    }
    case -8:
    {
      //pcb_t *currentProcess = findProcess(getPRID());
      state->reg_a0=(memaddr)currentProcess->p_supportStruct;
      break;
    }
    case -9:
    {
      //pcb_t *currentProcess = findProcess(getPRID());
      if(state->reg_a1==0)
      {
        state->reg_a0=currentProcess->p_pid;
      }
      else
      {
        if(currentProcess->p_parent!=NULL)
        {
          state->reg_a0=currentProcess->p_parent->p_pid;
        }
        else
        {
          state->reg_a0=0;
        }
      }
      break;
    }
    case -10:
    {
      copyState(&currentProcess->p_s, state);
      cpu_t currentTime;
      STCK(currentTime);
      cpu_t time=currentTime-startTime[getPRID()];
      currentProcess->p_time=time;
      currentProcess=NULL;
      scheduler();
      break;
    }
    default:
      break;
  }
}
