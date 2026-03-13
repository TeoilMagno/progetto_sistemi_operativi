#ifndef EXCEPTION_HANDLER
#define EXCEPTION_HANDLER

#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/pcb.h"
#include "./initial.h"
#include "./interrupts.h"
#include <uriscv/liburiscv.h>
#include <uriscv/cpu.h>

void exceptionHandler();

#endif
