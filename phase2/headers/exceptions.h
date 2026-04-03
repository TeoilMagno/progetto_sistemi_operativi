#ifndef EXCEPTION_HANDLER
#define EXCEPTION_HANDLER

#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/pcb.h"
#include "./functions.h"
#include "./initial.h"
#include "./interrupts.h"
#include <uriscv/cpu.h>
#include <uriscv/liburiscv.h>

extern cpu_t startTime[NCPU];

void exceptionHandler();
void passUpOrDie(int index, state_t *exceptionState);

#endif
