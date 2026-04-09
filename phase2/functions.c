#include "./headers/functions.h"
#include "headers/initial.h"
#include "headers/klog.h"
#include <uriscv/const.h>

// evita problemi di compilazione dove  gcc prova ad usare memcpy per copiare
// uno struct d tipo state_t da una variabile all'altra
void copyState(state_t *dep, state_t *arr) {
  dep->entry_hi = arr->entry_hi;
  dep->cause = arr->cause;
  dep->status = arr->status;
  dep->pc_epc = arr->pc_epc;
  dep->mie = arr->mie;

  for (int c = 0; c < STATE_GPR_LEN; c++) {
    dep->gpr[c] = arr->gpr[c];
  }
}

/*
 * mappatura dei device all'interno del deviceSempahore
 *
 *          +------------------+-----------+-------------+
 *          | disk, flash,     | terminali | pseudoclock |
 *          | network, printer |  (8x2)    |             |
 * +--------+------------------+-----------+-------------+
 * | indici | 0 ... 31         | 32 ... 47 | 48          |
 * +--------+------------------+-----------+-------------+
 *
 */
int findDeviceIndex(memaddr deviceAddr) {
  // 0x10 equivale alla dimensione in memoria che occupa ogni device
  unsigned int bottom = (unsigned int)deviceAddr - START_DEVREG;

  //-1 in caso di errore
  int deviceIndex = -1;

  // controllo se il device è un terminale, in caso ogni terminale è suddiviso
  // in due subdevice ognuno che occupa 0x8
  if (bottom >= 32 * 0x10)
    deviceIndex = 32 + (bottom - (32 * 0x10)) / 0x8;
  else if (bottom >= 0 && bottom < 32 * 0x10) // non è un terminale
    deviceIndex = bottom / 0x10;

  if (deviceIndex < 0 || deviceIndex >= 49)
    return -1; // errore dispositivo non valido

  return deviceIndex;
}

pcb_t *findProcess(int pid) {
  if (currentProcess != NULL && currentProcess->p_pid == pid) {
    return currentProcess;
  }

  struct list_head *iter;
  list_for_each(iter, &readyQueue) {
    pcb_t *item = container_of(iter, pcb_t, p_list);
    if (item->p_pid == pid) {
      return item;
    }
  }
  return findBlockedPcb(pid);
}

void killProcess(pcb_t *pcb) {
  if (pcb != NULL) {
    // 1. Termina ricorsivamente tutti i figli usando un while
    while (!emptyChild(pcb)) {
      killProcess(removeChild(pcb));
    }

    // 2. Rimuove il processo dal suo genitore
    outChild(pcb);

    // 3. Rimuove il processo dalla readyQueue (se è lì)
    outProcQ(&readyQueue, pcb);

    // 4. Rimuove il processo dai semafori (se era bloccato)
    if (outBlocked(pcb) != NULL) {
      softBlockCount--;
    }

    // 5. Decrementa il numero di processi attivi (FONDAMENTALE!)
    processCount--;

    // 6. Libera il PCB
    freePcb(pcb);
  }
}

cpu_t updateTime(int pid) {
  cpu_t currentTime;
  STCK(currentTime);
  cpu_t time = currentTime - startTime[pid];
  return time;
}
