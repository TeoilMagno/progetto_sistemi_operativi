#ifndef KERNEL
#define KERNEL

#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/pcb.h"
#include "./scheduler.h"
#include <uriscv/liburiscv.h>

static unsigned int processCount=0;
static unsigned int softBlockCount=0;
static struct list_head readyQueue;
static pcb_t *currentProcess=NULL;
static semd_t deviceSemaphores[16];

int main();

#endif
