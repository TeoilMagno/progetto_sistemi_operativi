#include "./headers/vmSupport.h"
#include "headers/const.h"
#include <uriscv/const.h>
#include <uriscv/liburiscv.h>
#include <uriscv/types.h>

//semaforo per avere mutua esclusione sull'accesso alla swap pool
int swapPoolSemaphore = 1;
static unsigned int frameIndex=0;

void pager()
{
  support_t *sup = (support_t *) SYSCALL(GETSUPPORTPTR, 0, 0, 0);

  if(sup != NULL)
  {
    state_t* state = &sup->sup_exceptState[0];
    unsigned int cause = state->cause;
    unsigned int excCode = (cause & GETEXECCODE) >> CAUSESHIFT;

    if(excCode == EXC_TLBMOD)
    {
      //programTrapHandler(sup);
      return;
    }

    SYSCALL(PASSEREN, (int)&swapPoolSemaphore, 0, 0);

    int p = state->entry_hi >> ASIDSHIFT;
    pteEntry_t page = sup->sup_privatePgTbl[p];

    swap_t *frame = (swap_t *)((memaddr *)(FRAMEPOOLSTART + POOLSIZE * pageReplacement()));
    
    if(frame->sw_asid == -1) //il frame è libero
    {
      frame->sw_asid = sup->sup_asid;
      frame->sw_pageNo = p;
      frame->sw_pte = &page;
    }
    else //il frame è occupato
    {
      //inizio azione atomica
      //disabilito gli interrupts
      setSTATUS(getSTATUS() & ~MSTATUS_MIE_MASK);

      pteEntry_t *ptu = frame->sw_pte; //Page To Update
      //aggiornando la pagina già occupante il frame
      ptu->pte_entryLO = (ptu->pte_entryLO) & VALIDOFF;

      //controllo se la pagine è nella cache della TLB
      setENTRYHI(ptu->pte_entryHI);
      TLBP();
      unsigned int index = getINDEX();

      //se è in cache va aggiornata
      if(!(index & PRESENTFLAG))
      {
        setENTRYLO(ptu->pte_entryLO);
        TLBWI();
      }

      //riabilito gli interrupts
      setSTATUS(getSTATUS() | MSTATUS_MIE_MASK);
      //fine azione atomica
      
      //aggiorno il backing store del vecchio processo con il nuovo frame
      //TODO AL MOMENTO QUESTO CODICE È ERRATO, NON SCRIVO SUL VECCHIO FLASH DEVICE
      //MA SU QUELLO DEL CURRENT PROCESS
      dtpreg_t *devReg = (dtpreg_t *)calcDevAddr(calcIntLineNo(cause));
      devReg->data0 = (memaddr) frame;
      SYSCALL(DOIO, (int)devReg->command, (int)FLASHWRITE, 0);


    }
  }
}

//returns the index of a framed in the swap pool to be swap-in
int pageReplacement()
{
  frameIndex++;
  return frameIndex % POOLSIZE;
}
