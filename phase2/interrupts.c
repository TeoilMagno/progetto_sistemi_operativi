#include"./headers/interrupts.h"

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

void interruptHandler(state_t *stato)
{
  //calculates Interrupt Exception Code
  //see Table 1: Interrupt Line and Device Class Mapping
  //in specs
  int IntlineNo = 0;
  int DevNo = 0;

  switch(getCAUSE() & CAUSE_EXCCODE_MASK)
  {
    case IL_CPUTIMER:
      IntlineNo=1;
      break;

    case IL_TIMER:
      IntlineNo=2;
      break;

    case IL_DISK:
      IntlineNo=3;
      break;

    case IL_FLASH:
      IntlineNo=4;
      break;

    case IL_ETHERNET:
      IntlineNo=5;
      break;

    case IL_PRINTER:
      IntlineNo=6;
      break;

    case IL_TERMINAL:
      IntlineNo=7;
      break;

    default:
      //imòossibile
  }

  if(IntlineNo>=3 && IntlineNo<=7)
  {
    memaddr bitmap = 0x10000040+0x04*(IntlineNo-3);

    if(bitmap & DEV0ON) DevNo = 0;
    else if(bitmap & DEV1ON) DevNo = 1;
    else if(bitmap & DEV2ON) DevNo = 2;
    else if(bitmap & DEV3ON) DevNo = 3;
    else if(bitmap & DEV4ON) DevNo = 4;
    else if(bitmap & DEV5ON) DevNo = 5;
    else if(bitmap & DEV6ON) DevNo = 6;
    else if(bitmap & DEV7ON) DevNo = 7;
    else DevNo = -1;

    if(IntlineNo==7) // è un terminale
    {
    }

    memaddr devAddrBase = START_DEVREG+((IntlineNo-3)*0x80)+(DevNo*0x10);

    dtpreg_t *devReg = (dtpreg_t *)devAddrBase;
    memaddr savedStatus = devReg->status;
    devReg->command=ACK;

    int *sem = &deviceSemaphores[findDeviceIndex(devAddrBase)];
    pcb_t *unblocked = removeBlocked(sem);

    if(unblocked != NULL)
    {
      unblocked->p_s.reg_a1 = savedStatus;
      list_add(&unblocked->p_list, &readyQueue);
    }

    if(currentProcess != NULL)
      LDST(stato);
    else
      scheduler();
  }
  
}
