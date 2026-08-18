#ifndef INIT_PROC
#define INIT_PROC

#include "../../headers/types.h"
#include "../../headers/klog.h"
#include <uriscv/cpu.h>
#include <uriscv/liburiscv.h>

extern swap_t swap_pool[POOLSIZE];
extern int swapPoolSemaphore;
extern int masterSemaphore;
extern int shellSemaphore;
extern int terminalWriteSem;
extern int terminalReadSem;
extern support_t supportPool[UPROCMAX];

void test();

#endif
