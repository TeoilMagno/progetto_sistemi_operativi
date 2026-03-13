#include"./headers/interrupts.h"

void interrupt()
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


  if(IntlineNo==7) //interrupt causato da un terminale
  {}

  if(IntlineNo>=3 && IntlineNo<=6)
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

    memaddr devAddrBase = START_DEVREG+((IntlineNo-3)*0x80)+(DevNo*0x10);

    dtpreg_t *devReg = (dtpreg_t *)devAddrBase;
    memaddr stateAddr = devReg->status;

    //probabilemnte sbagliato
    state_t *state = (state_t *)stateAddr;
    semd_t *sem = (semd_t *)state->reg_a1;
    pcb_t *unblocked = headBlocked(sem->s_key);

    devReg->command=ACK;
    SYSCALL(VERHOGEN, *sem->s_key, 0, 0);


  }
  
}
