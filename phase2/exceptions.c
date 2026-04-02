#include "./headers/exceptions.h"

void syscallHandler(state_t *state);

void exceptionHandler()
{
  unsigned int cause = getCAUSE();
  unsigned int causeCode = cause & CAUSE_EXCCODE_MASK;
  state_t *state = GET_EXCEPTION_STATE_PTR(getPRID());

  if(CAUSE_IS_INT(cause))
  {
    interruptHandler(state);
  }
  else
  {
    if(causeCode>=24 && causeCode<=28)
    {
      //TLB
    }
    else if(causeCode==8 || causeCode==11)
    {
      syscallHandler(state);
    }
    else if((causeCode>=0&&causeCode<=7)||causeCode==9||causeCode==10||(causeCode>=12&&causeCode<=23))
    {
      //Program Trap
    } 
  }
}

void syscallHandler(state_t *state)
{
  if((state->status & MSTATUS_MPP_MASK)==0)
  {
    state->cause = PRIVINSTR;
    //Program Trap
  }
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
      }
      state->pc_epc+=4;
      LDST(state);
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
        state->pc_epc+=4;
        copyState(&currentProcess->p_s, state);
        currentProcess->p_time+=updateTime(getPRID());
        currentProcess=NULL;
        softBlockCount++;
        scheduler();
      }
      else
      {
        (*semAdd)--;
        state->pc_epc+=4;
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
          (*semAdd)++;
        }
      }
      state->pc_epc+=4;
      LDST(state);
      break;
    }
    case -5:
    {
      memaddr *commandAddr = (memaddr *)state->reg_a1;
      int value = state->reg_a2;
      *commandAddr = value;
      int* semPtr = &deviceSemaphore[findDeviceIndex(*commandAddr)];
      (*semPtr)--;
      insertBlocked(semPtr, currentProcess);
      state->pc_epc+=4;
      copyState(&currentProcess->p_s, state);
      currentProcess->p_time+=updateTime(getPRID());
      currentProcess=NULL;
      softBlockCount++;
      scheduler();
      break;
    }
    case -6:
    {
      int pid=getPRID();
      state->reg_a0 = findProcess(pid)->p_time+updateTime(pid);
      state->pc_epc+=4;
      LDST(state);
      break;
    }
    case -7:
    {
      int *pseudoClock = &deviceSemaphore[47]; 
      insertBlocked(pseudoClock, currentProcess);
      state->pc_epc+=4;
      copyState(&currentProcess->p_s, state);
      currentProcess->p_time+=updateTime(getPRID());
      currentProcess=NULL;
      softBlockCount++;
      scheduler();
      break;
    }
    case -8:
    {
      state->reg_a0=(memaddr)currentProcess->p_supportStruct;
      state->pc_epc+=4;
      LDST(state);
      break;
    }
    case -9:
    {
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
      state->pc_epc+=4;
      LDST(state);
      break;
    }
    case -10:
    {
      insertProcQ(&readyQueue, currentProcess);
      state->pc_epc+=4;
      copyState(&currentProcess->p_s, state);
      currentProcess->p_time+=updateTime(getPRID());
      currentProcess=NULL;
      scheduler();
      break;
    }
    default:
      break;
  }
}

void passUpOrDie(int index, state_t *exceptionState){
    if(currentProcess->p_supportStruct == NULL){ //caso Die
        //Terminazione del processo
        killProcess(currentProcess);
        scheduler();
    }else{ //caso Pass Up
        //copia dello stato attuale in sup_exceptState        
        copyState(&(currentProcess->p_supportStruct->sup_exceptState[index]), exceptionState);
        //Caricamento del contesto in LDCXT
        state_t *context = &(currentProcess->p_supportStruct->sup_exceptContext[index]);       
        LDCXT(context);
    }
}
