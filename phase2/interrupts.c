#include "./headers/interrupts.h"
#include <uriscv/liburiscv.h>

void interruptHandler(state_t *stato)
{
  // calculates Interrupt Exception Code
  // see Table 1: Interrupt Line and Device Class Mapping
  // in specs
  int IntlineNo = calcIntLineNo(getCAUSE());

  if (IntlineNo == 1)
    handlePLT(stato);
  else if (IntlineNo == 2)
    handleIntervalClock(stato);
  else if (IntlineNo >= 3 && IntlineNo <= 7)
    handleDevice(IntlineNo, stato);
}

void handleDevice(int IntlineNo, state_t *stato)
{
  // inizializzazione di varie variabili
  unsigned int savedStatus = 0;
  pcb_t *unblocked = NULL;
  int *sem = NULL;

  // calcolo l'indirizzo a cui è istanziato il deviec che ha creato l'interrupt
  // per poi puntare allo struct interessato
  memaddr devAddr = calcDevAddr(IntlineNo);

  int semIndex = 0;

  if (IntlineNo == 7) // il device è un terminale
  {
    termreg_t *termReg = (termreg_t *)devAddr;

    // controllo che l'operazione sia di output
    if ((termReg->transm_status & 0xff) == OKCHARTRANS)
    {
      savedStatus = termReg->transm_status;
      termReg->transm_command = ACK;
      semIndex = findDeviceIndex(devAddr + 0x8);
      if (semIndex != -1) // findeDeviceIndex restituisce -1 in caso di errore
      {
        sem = &deviceSemaphore[semIndex]; // 0xc == transm_command
        unblocked = removeBlocked(sem);
      }
    }
    // controllo che l'operazione sia di input
    else if ((termReg->recv_status & 0xff) == CHARRECV)
    {
      savedStatus = termReg->recv_status;
      termReg->recv_command = ACK;
      semIndex = findDeviceIndex(devAddr);
      if (semIndex != -1) // findeDeviceIndex restituisce -1 in caso di errore
      {
        sem = &deviceSemaphore[semIndex]; // 0xc == transm_command
        unblocked = removeBlocked(sem);
      }
    }

  } else {
    // il device non è un terminale
    dtpreg_t *devReg = (dtpreg_t *)devAddr;
    savedStatus = devReg->status;
    devReg->command = ACK;

    semIndex = findDeviceIndex(devAddr);
    if (semIndex != -1) // findeDeviceIndex restituisce -1 in caso di errore
    {
      sem = &deviceSemaphore[semIndex]; // 0xc == transm_command
      unblocked = removeBlocked(sem);
    }
  }

  if (unblocked != NULL)
  {
    unblocked->p_s.reg_a0 = savedStatus;
    insertProcQ(&readyQueue, unblocked);
    softBlockCount--;
  }
  else // dovrebbe essere impossibile, ma se succede...
  {
    if (semIndex != -1)
      (*sem)++;
  }

  if (currentProcess != NULL)
  {
    LDST(stato);
  }
  else
    scheduler();
}

void handlePLT(state_t *stato)
{
  // ACK dell'interrupt
  setTIMER(TIMESLICE);

  // salvo lo stato della cpu nel p_s del processo da sbloccare
  copyState(&currentProcess->p_s, stato);

  // inserisco il processo in readyQueue
  insertProcQ(&readyQueue, currentProcess);
  scheduler();
}

void handleIntervalClock(state_t *stato)
{
  // ACK dell'interrupt
  LDIT(PSECOND);

  // sblocco tutti i processi che aspettano uno PseudoClock Tick
  // e li sposto nella readyQueue
  pcb_t *p = NULL;
  while ((p = removeBlocked(&deviceSemaphore[PSEUDOINDEX])) != NULL)
  {
    insertProcQ(&readyQueue, p);
    softBlockCount--;
  }

  if (currentProcess != NULL)
    LDST(stato);
  else
    scheduler();
}

int calcIntLineNo(unsigned int cause)
{
  switch (cause & CAUSE_EXCCODE_MASK)
  {
    case IL_CPUTIMER:
      return 1;
      break;

    case IL_TIMER:
      return 2;
      break;

    case IL_DISK:
      return 3;
      break;

    case IL_FLASH:
      return 4;
      break;

    case IL_ETHERNET:
      return 5;
      break;

    case IL_PRINTER:
      return 6;
      break;

    case IL_TERMINAL:
      return 7;
      break;

    default:
      // imòossibile
      PANIC();
  }
}

int calcDevNo(int IntlineNo)
{
  memaddr *bitmap = (unsigned int *)0x10000040;
  unsigned int word = IntlineNo - 3;

  // controllo quale istanza del device ha create l'interrupt
  if (bitmap[word] & DEV0ON)
    return 0;
  else if (bitmap[word] & DEV1ON)
    return 1;
  else if (bitmap[word] & DEV2ON)
    return 2;
  else if (bitmap[word] & DEV3ON)
    return 3;
  else if (bitmap[word] & DEV4ON)
    return 4;
  else if (bitmap[word] & DEV5ON)
    return 5;
  else if (bitmap[word] & DEV6ON)
    return 6;
  else if (bitmap[word] & DEV7ON)
    return 7;
  else
    // non dovrebbe mai succedere questo case, ma se succede
    PANIC();
}

memaddr calcDevAddr(int IntlineNo)
{
  int DevNo = calcDevNo(IntlineNo);
  return START_DEVREG + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10);
}
