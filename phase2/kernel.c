#include "./headers/kernel.c"

static unsigned int processCount=0;
static unsigned int softBlockCount=0;
static struct list_head readyQueue;
static pcb_t *currentProcess=NULL;
/*Device Semaphores: The Nucleus maintains one integer semaphore for each external (sub)device,
plus one additional semaphore to support the Pseudo-clock. Since terminal devices are actu-
ally two independent sub-devices, the Nucleus maintains two semaphores for each terminal
device.*/
static semd_t deviceSemaphores[2];

void uTLB_RefillHandler()
{
  int prid = getPRID();
  setENTRYHI(0x80000000);
  setENTRYLO(0x00000000);
  TLBWR();
  LDST((state_t*) BIOSDATAPAGE);
}

void exceptionHandler()
{
}

int main()
{
  passupvector_t *puv=(passupvector_t *)0x0FFFF900;
  puv->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
  puv->tlb_refill_stackPtr = KERNELSTACK;
  puv->exception_handler = (memaddr)exceptionHandler;
  puv->exception_stackPtr = KERNELSTACK;

  initPcbs();
  initASL();

  mkEmptyProcQ(&readyQueue);

  for(int c=0;c<2;c++)
  {
    deviceSemaphores[c].s_key=0;
  }
}
