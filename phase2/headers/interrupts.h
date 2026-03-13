#ifndef INTERRUPTS
#define INTERRUPTS

#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/pcb.h"
#include "./initial.h"
#include <uriscv/liburiscv.h>
#include <uriscv/cpu.h>

void interruptHandler(state_t *stato);

#endif
