#ifndef INTERRUPTS
#define INTERRUPTS

#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/pcb.h"
#include "./functions.h"
#include "./initial.h"
#include "./klog.h"
#include "./scheduler.h"
#include <uriscv/cpu.h>
#include <uriscv/liburiscv.h>

void interruptHandler(state_t *stato);
void handleDevice(int IntlineNo, state_t *stato);
void handlePLT(state_t *stato);
void handleIntervalClock(state_t *stato);

#endif
