#include "./headers/exceptions.h"
#include "headers/functions.h"
#include "headers/initial.h"
#include "headers/klog.h"
#include <uriscv/types.h>

void syscallHandler(state_t *state);

void exceptionHandler() {
  unsigned int cause = getCAUSE();
  unsigned int causeCode = (cause & CAUSE_EXCCODE_MASK);
  state_t *state = GET_EXCEPTION_STATE_PTR(getPRID());

  if (CAUSE_IS_INT(cause)) {
    interruptHandler(state);
  } else {
    if (causeCode >= 24 && causeCode <= 28) // Gestione TBL
    {
      passUpOrDie(PGFAULTEXCEPT, state);
    } else if (causeCode == 8 || causeCode == 11) // Gestione SYSCALL
    {
      syscallHandler(state);
      // Gestione Program Trap
    } else if ((causeCode >= 0 && causeCode <= 7) || causeCode == 9 ||
               causeCode == 10 || (causeCode >= 12 && causeCode <= 23)) {
      passUpOrDie(GENERALEXCEPT, state);
    }
  }
}

void syscallHandler(state_t *state) {
  if ((state->status & MSTATUS_MPP_MASK) == 0) {
    unsigned int oldCode =
        (state->cause & GETEXECCODE); // Isolo la parte del registro che contine
                                      // il codice errore attuale
    state->cause = state->cause - oldCode; // Tolgo oldCode da state->cause
    state->cause =
        state->cause |
        (PRIVINSTR << CAUSESHIFT); // Inserisco PRIVINSTR per segnalare l'errore
    // Program Trap
    passUpOrDie(GENERALEXCEPT, state);
    return;
  }
  switch (state->reg_a0) {
  case CREATEPROCESS: {
    pcb_t *newPcb = allocPcb();
    if (newPcb == NULL) {
      state->reg_a0 = -1;
    } else {
      state->reg_a0 = newPcb->p_pid;
      state_t *sreg_a1 = (state_t *)state->reg_a1;
      copyState(&newPcb->p_s, sreg_a1);
      newPcb->p_prio = state->reg_a2;
      newPcb->p_supportStruct = (support_t *)state->reg_a3;
      insertChild(currentProcess, newPcb);
      insertProcQ(&readyQueue, newPcb);
      processCount++;
    }
    state->pc_epc += 4;
    LDST(state);
    break;
  }
  case TERMPROCESS: {
    if (state->reg_a1 == 0) {
      killProcess(currentProcess);
      scheduler();
    } else {
      killProcess(findProcess(state->reg_a1));
      scheduler();
    }
    break;
  }
  case PASSEREN: {
    int *semAdd = (int *)state->reg_a1;
    if (*semAdd <= 0) {
      insertBlocked(semAdd, currentProcess);
      state->pc_epc += 4;
      copyState(&currentProcess->p_s, state);
      currentProcess->p_time += updateTime(getPRID());
      currentProcess = NULL;
      scheduler();
    } else {
      (*semAdd)--;
      state->pc_epc += 4;
      LDST(state);
    }
    break;
  }
  case VERHOGEN: {
    int *semAdd = (int *)state->reg_a1;
    pcb_t *unlockedProcess = removeBlocked(semAdd);
    if (unlockedProcess != NULL) {
      insertProcQ(&readyQueue, unlockedProcess);
    } else {
      (*semAdd)++;
    }
    state->pc_epc += 4;
    LDST(state);
    break;
  }
  case DOIO: {
    memaddr *commandAddr = (memaddr *)state->reg_a1;
    unsigned int value = state->reg_a2;
    *commandAddr = value;
    int *semPtr = &deviceSemaphore[findDeviceIndex((memaddr)(commandAddr))];
    if (*semPtr <= 0)
      insertBlocked(semPtr, currentProcess);
    else
      (*semPtr)--;

    state->pc_epc += 4;
    copyState(&currentProcess->p_s, state);
    currentProcess->p_time += updateTime(getPRID());
    currentProcess = NULL;
    softBlockCount++;
    scheduler();
    break;
  }
  case GETTIME: {
    int pid = getPRID();
    state->reg_a0 = findProcess(pid)->p_time + updateTime(pid);
    state->pc_epc += 4;
    LDST(state);
    break;
  }
  case CLOCKWAIT: {
    int *pseudoClock = &deviceSemaphore[PSEUDOINDEX];
    if (*pseudoClock <= 0)
      insertBlocked(pseudoClock, currentProcess);
    else
      (*pseudoClock)--;
    state->pc_epc += 4;
    copyState(&currentProcess->p_s, state);
    currentProcess->p_time += updateTime(getPRID());
    currentProcess = NULL;
    softBlockCount++;
    scheduler();
    break;
  }
  case GETSUPPORTPTR: {
    state->reg_a0 = (memaddr)currentProcess->p_supportStruct;
    state->pc_epc += 4;
    LDST(state);
    break;
  }
  case GETPROCESSID: {
    if (state->reg_a1 == 0) {
      state->reg_a0 = currentProcess->p_pid;
    } else {
      if (currentProcess->p_parent != NULL) {
        state->reg_a0 = currentProcess->p_parent->p_pid;
      } else {
        state->reg_a0 = 0;
      }
    }
    state->pc_epc += 4;
    LDST(state);
    break;
  }
  case YIELD: {
    insertProcQ(&readyQueue, currentProcess);
    state->pc_epc += 4;
    copyState(&currentProcess->p_s, state);
    currentProcess->p_time += updateTime(getPRID());
    currentProcess = NULL;
    scheduler();
    break;
  }
  default:
    // Se la syscall non è nei casi da -1 a -10 passiamo il controllo a
    // passUpOrDie
    state->pc_epc += 4;
    passUpOrDie(GENERALEXCEPT, state);
    break;
  }
}

void passUpOrDie(int index, state_t *exceptionState) {
  if (currentProcess->p_supportStruct == NULL) { // caso Die
    // Terminazione del processo
    killProcess(currentProcess);
    currentProcess = NULL;
    scheduler();
  } else { // caso Pass Up
    // copia dello stato attuale in sup_exceptState
    copyState(&(currentProcess->p_supportStruct->sup_exceptState[index]),
              exceptionState);
    // Caricamento del contesto in LDCXT
    context_t *ctx =
        &(currentProcess->p_supportStruct->sup_exceptContext[index]);
    LDCXT(ctx->stackPtr, ctx->status, ctx->pc);
  }
}
