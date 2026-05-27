#include "./headers/vmSupport.h"
#include <uriscv/const.h>
#include <uriscv/liburiscv.h>
#include <uriscv/types.h>

//semaforo per avere mutua esclusione sull'accesso alla swap pool
int swapPoolSemaphore = 1;
static int frameIndex=0;

void TLB_exceptionHandler() //Pager
{
  support_t *sup = (support_t *) SYSCALL(GETSUPPORTPTR, 0, 0, 0);

  if(sup != NULL)
  {
    state_t* state = &sup->sup_exceptState[0];

    SYSCALL(PASSEREN, (int)&swapPoolSemaphore, 0, 0);

    int p = state->entry_hi;
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
      pteEntry_t *ptu = frame->sw_pte; //Page To Update
      //aggiornando la pagina già occupante il frame
      ptu->pte_entryLO
    }
  }
}

//returns the index of a framed in the swap pool to be swap-in
int pageReplacement()
{
  frameIndex++;
  return frameIndex % POOLSIZE;
}
