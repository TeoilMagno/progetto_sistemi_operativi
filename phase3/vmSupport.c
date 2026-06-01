#include "./headers/vmSupport.h"
#include <uriscv/liburiscv.h>
#include <uriscv/types.h>

extern swap_t swap_pool[POOLSIZE];

//semaforo per avere mutua esclusione sull'accesso alla swap pool
extern int swapPoolSemaphore;
static int frameIndex=0;

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

    int p = findPageIndex( state->entry_hi);
    pteEntry_t *page = &sup->sup_privatePgTbl[p];

    int pfn = pageReplacement();
    swap_t *frame = &swap_pool[pfn];
    
    if(frame->sw_asid == -1) //il frame è libero
    {
      unsigned int asid = (unsigned int)(page->pte_entryHI & 0x00000fff);
      dtpreg_t *devReg =(dtpreg_t *) ((memaddr) DEV_REG_ADDR(IL_FLASH, asid-1));
      devReg->data0 = (memaddr) frame;
      int status = SYSCALL(DOIO, (int)devReg->command, (int)(FLASHREAD << 8), 0);
      
      if((status & 0xff) == 5)
      {
        //program trap exception handler
      }
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
      unsigned int asid = (unsigned int)(ptu->pte_entryHI & 0x00000fff);
      dtpreg_t *devReg =(dtpreg_t *) ((memaddr) DEV_REG_ADDR(IL_FLASH, asid-1));
      devReg->data0 = (memaddr) frame;
      int status = SYSCALL(DOIO, (int)devReg->command, (int)(FLASHWRITE << 8), 0);

      if((status & 0xff) == 4)
      {
        //program trap exception handler
      }
    }

    //carico finalmente la nuova pagina in swap pool
    frame->sw_asid = sup->sup_asid;
    frame->sw_pageNo = p;
    frame->sw_pte = page;

    //la nuova pagina è valida
    page->pte_entryLO = page->pte_entryLO | VALIDON;
    //update del pfn senza variare i flags
    page->pte_entryLO = (page->pte_entryLO & 0xf) | (pfn << 4);

    setENTRYHI(page->pte_entryHI);
    TLBP();
    unsigned int index = getINDEX();

    //se è in cache va aggiornata
    if(!(index & PRESENTFLAG))
    {
      setENTRYLO(page->pte_entryLO);
      TLBWI();
    }

    //rilascio mutua esclusione
    SYSCALL(VERHOGEN, (int)&swapPoolSemaphore, 0, 0);

    LDST(state);
  }
}

//returns the index of a framed in the swap pool to be swap-in
int pageReplacement()
{
  frameIndex++;
  return frameIndex % POOLSIZE;
}
