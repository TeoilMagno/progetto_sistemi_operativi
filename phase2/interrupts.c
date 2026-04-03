#include "./headers/interrupts.h"

void interruptHandler(state_t *stato) {
  // calculates Interrupt Exception Code
  // see Table 1: Interrupt Line and Device Class Mapping
  // in specs
  int IntlineNo = 0;

  switch (getCAUSE() & CAUSE_EXCCODE_MASK) {
  case IL_CPUTIMER:
    IntlineNo = 1;
    break;

  case IL_TIMER:
    IntlineNo = 2;
    break;

  case IL_DISK:
    IntlineNo = 3;
    break;

  case IL_FLASH:
    IntlineNo = 4;
    break;

  case IL_ETHERNET:
    IntlineNo = 5;
    break;

  case IL_PRINTER:
    IntlineNo = 6;
    break;

  case IL_TERMINAL:
    IntlineNo = 7;
    break;

  default:
    // imòossibile
  }

  if (IntlineNo == 1)
    handlePLT(stato);
  else if (IntlineNo >= 3 && IntlineNo <= 7)
    handleDevice(IntlineNo, stato);
}

void handleDevice(int IntlineNo, state_t *stato) {
  unsigned int savedStatus = 0;
  pcb_t *unblocked = NULL;
  int DevNo = 0;
  memaddr bitmap = 0x10000040 + 0x04 * (IntlineNo - 3);
  int *sem = NULL;

  if (bitmap & DEV0ON)
    DevNo = 0;
  else if (bitmap & DEV1ON)
    DevNo = 1;
  else if (bitmap & DEV2ON)
    DevNo = 2;
  else if (bitmap & DEV3ON)
    DevNo = 3;
  else if (bitmap & DEV4ON)
    DevNo = 4;
  else if (bitmap & DEV5ON)
    DevNo = 5;
  else if (bitmap & DEV6ON)
    DevNo = 6;
  else if (bitmap & DEV7ON)
    DevNo = 7;
  else
    DevNo = -1;

  memaddr devAddr = START_DEVREG + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10);

  if (IntlineNo == 7) // è un terminale
  {
    termreg_t *termReg = (termreg_t *)devAddr;
    unsigned int transStatus = termReg->transm_status;
    unsigned int recvStatus = termReg->recv_status;

    if (transStatus == OKCHARTRANS) {
      savedStatus = transStatus;
      termReg->transm_command = ACK;
      sem = &deviceSemaphore[findDeviceIndex(devAddr)]; // 0xc == transm_command
    }
    if (recvStatus == CHARRECV) {
      savedStatus = recvStatus;
      termReg->recv_command = ACK;
      sem = &deviceSemaphore[findDeviceIndex(devAddr)]; // 0x4 == recv_command
    }

    if (sem != NULL)
      unblocked = removeBlocked(sem);
    else // dovrebbe essere impossibile, ma se succede...
    {
      (*sem)++;
    }
  } else {
    dtpreg_t *devReg = (dtpreg_t *)devAddr;
    savedStatus = devReg->status;
    devReg->command = ACK;

    sem = &deviceSemaphore[findDeviceIndex(devAddr)];
    unblocked = removeBlocked(sem);
  }

  if (unblocked != NULL) {
    unblocked->p_s.reg_a1 = savedStatus;
    list_add(&unblocked->p_list, &readyQueue);
  } else // dovrebbe essere impossibile, ma se succede...
  {
    (*sem)++;
  }

  if (currentProcess != NULL)
    LDST(stato);
  else
    scheduler();
}

void handlePLT(state_t *stato) {
  setTIMER(TIMESLICE);

  copyState(&currentProcess->p_s, stato);

  insertProcQ(&readyQueue, currentProcess);
  scheduler();
}

void handleIntervalClock(state_t *stato) {
  LDIT(PSECOND);

  pcb_t *p = NULL;
  for (int c = 0; c < deviceSemaphore[PSEUDOINDEX]; c++) {
    p = removeBlocked(&deviceSemaphore[PSEUDOINDEX]);
    insertProcQ(&readyQueue, p);
  }

  if (currentProcess != NULL)
    LDST(stato);
  else
    scheduler();
}
