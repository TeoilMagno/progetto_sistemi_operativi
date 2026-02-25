#include "./headers/initial.h"

static unsigned int processCount=0;
static unsigned int softBlockCount=0;
static struct list_head readyQueue;
static pcb_t *currentProcess=NULL;
static semd_t deviceSemaphores[16];

extern void p2test();

// void uTLB_RefillHandler()
// {
//   int prid = getPRID();
//   setENTRYHI(0x80000000);
//   setENTRYLO(0x00000000);
//   TLBWR();
//   LDST((state_t*) BIOSDATAPAGE);
// }

void exceptionHandler()
{
}

int main()
{
  passupvector_t *puv=(passupvector_t *)PASSUPVECTOR;
  puv->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
  puv->tlb_refill_stackPtr = KERNELSTACK;
  puv->exception_handler = (memaddr)exceptionHandler;
  puv->exception_stackPtr = KERNELSTACK;

  initPcbs();
  initASL();

  mkEmptyProcQ(&readyQueue);

  for(int c=0;c<16;c++)
  {
    deviceSemaphores[c].s_key=0;
  }

  //macro to initialize system-wide Interval Timer
  LDIT(PSECOND);
  
  pcb_t *p=allocPcb();
  processCount++;

  //enables Interrupts
  p->p_s.mie=MIE_ALL;
  //enable Interrupt and Kernel Mode
  p->p_s.status=MSTATUS_MPIE_MASK | MSTATUS_MPP_M;
  RAMTOP(p->p_s.reg_sp);
  p->p_s.pc_epc=(memaddr)p2test;
  p->p_parent=NULL;
  INIT_LIST_HEAD(&p->p_child);
  INIT_LIST_HEAD(&p->p_sib);
  p->p_time=0;
  p->p_semAdd=NULL;
  p->p_supportStruct=NULL;

  return 0;
}
