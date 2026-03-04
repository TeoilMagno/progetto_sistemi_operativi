#ifndef KERNEL
#define KERNEL

#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/pcb.h"
#include "./scheduler.h"
#include "./exceptions.h"
#include <uriscv/liburiscv.h>

static unsigned int processCount=0;
static unsigned int softBlockCount=0;
static struct list_head readyQueue;
static pcb_t *currentProcess=NULL;
static semd_t deviceSemaphores[16];

extern void test();
extern void uTLB_RefillHandler();
void exceptionHandler();
int main();

#endif
