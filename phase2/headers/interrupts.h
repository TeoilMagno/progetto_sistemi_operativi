#ifndef INTERRUPTS
#define INTERRUPTS

#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/pcb.h"
#include "./initial.h"
#include "./scheduler.h"
#include <uriscv/liburiscv.h>
#include <uriscv/cpu.h>
#include "./functions.h"

void interruptHandler(state_t *stato);
void handleDevice(int IntlineNo, state_t *stato);
void handlePLT(state_t *stato);
void handleClock(state_t *stato);

#endif
