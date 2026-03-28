#include "./headers/initial.h"

unsigned int processCount=0;
unsigned int softBlockCount=0;
struct list_head readyQueue;
pcb_t *currentProcess=NULL;
int deviceSemaphore[SEMDEVLEN];
cpu_t startTime[NCPU];

extern void test();
extern void uTLB_RefillHandler();

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
  for(int c=0;c<NCPU;c++)
  {
    startTime[c]=0;
  }
  for(int c=0;c<NRSEMAPHORES;c++)
  {
    deviceSemaphore[c]=0;
  }

  //macro to initialize system-wide Interval Timer
  LDIT(PSECOND);
  
  pcb_t *p=allocPcb();
  if(p != NULL){
    processCount++;

    //enables Interrupts
    p->p_s.mie=MIE_ALL;
    //enable Interrupt and Kernel Mode
    p->p_s.status=MSTATUS_MPIE_MASK | MSTATUS_MPP_M;
    RAMTOP(p->p_s.reg_sp);
    p->p_s.pc_epc=(memaddr) test;
    p->p_parent=NULL;
    INIT_LIST_HEAD(&p->p_child);
    INIT_LIST_HEAD(&p->p_sib);
    p->p_time=0;
    p->p_semAdd=NULL;
    p->p_supportStruct=NULL;
    //Inserimento del processo in coda
    insertProcQ(&readyQueue, p);
  }else{ //se è presente un errore il sistema si ferma
    PANIC();
  }

  scheduler();

  return 0;
}
