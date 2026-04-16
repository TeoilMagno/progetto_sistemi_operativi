#include "./headers/scheduler.h"
#include "headers/initial.h"
#include "headers/klog.h"

extern cpu_t startTime[NCPU];

void scheduler() {
  if (list_empty(&readyQueue)) {
    // If the Process Count is 0, invoke the HALT BIOS service/instruction
    // [Section 13.2]. Consider this a job well done!
    if (processCount == 0)
      HALT();
    // If the Process Count > 0 and Soft-block Count > 0 enter a Wait State.
    else if (processCount > 0 && softBlockCount > 0) {
      // sets the mie register to enable interrupts and either disable the PLT
      setMIE(MIE_ALL & ~MIE_MTIE_MASK);
      unsigned int status = getSTATUS();
      status |= MSTATUS_MIE_MASK;
      setSTATUS(status);
      WAIT();
    }
    // caso di deadlock
    else if (processCount > 0 && softBlockCount == 0)
      PANIC();
  } else // se la readyQueue non è vuota
  {
    klog_print(" processCount: ");
    klog_print_dec(processCount);
    klog_print(" softBlockCount: ");
    klog_print_dec(softBlockCount);
    // Estrae il primo pcb da readyQueue che diventa il processo in esecuzione
    // sulla CPU
    currentProcess = removeProcQ(&readyQueue);
    // Timer di 5 ms allo scadere del quale genera un interrupt
    setTIMER(TIMESLICE);
    LDST(&(currentProcess->p_s));
  }
}
