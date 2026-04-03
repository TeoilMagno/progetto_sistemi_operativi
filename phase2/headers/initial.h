#ifndef KERNEL
#define KERNEL

#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/pcb.h"
#include "./exceptions.h"
#include "./functions.h"
#include "./scheduler.h"
#include <uriscv/liburiscv.h>

#define PSEUDOINDEX 48

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_t *currentProcess;
extern int deviceSemaphore[SEMDEVLEN];

extern void test();
extern void uTLB_RefillHandler();
void exceptionHandler();
int main();

#endif
