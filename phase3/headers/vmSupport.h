#ifndef VM_SUPPORT
#define VM_SUPPORT

#include "../../headers/types.h"
#include "../../headers/klog.h"
#include "./const.h"
#include "./sysSupport.h"
#include <uriscv/cpu.h>
#include <uriscv/liburiscv.h>
#include <uriscv/arch.h>
#include <uriscv/types.h>

extern swap_t swap_pool[POOLSIZE];
extern int swapPoolSemaphore;

void initSwapStructs();
int findPageIndex(unsigned int pte_entryHI);
void writeToDevice(swap_t *frame);
void readFromDevice(pteEntry_t *page, swap_t *frame, int p);
void pager();
int pageReplacement();

#endif
