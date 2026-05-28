#ifndef VM_SUPPORT
#define VM_SUPPORT

#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../headers/klog.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase2/headers/interrupts.h"
#include "./const.h"
#include <uriscv/cpu.h>
#include <uriscv/liburiscv.h>

int swapPoolSemaphore;
static unsigned int frameIndex;

void pager();
int pageReplacement();

#endif
