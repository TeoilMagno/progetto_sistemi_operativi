#ifndef FUNCTIONS
#define FUNCTIONS

#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../phase1/headers/asl.h"
#include "../../phase1/headers/pcb.h"
#include "./initial.h"
#include <uriscv/cpu.h>
#include <uriscv/liburiscv.h>

void copyState(state_t *dep, state_t *arr);

int findDeviceIndex(memaddr deviceAddr);

pcb_t *findProcess(int pid);

void killProcess(pcb_t *pcb);

cpu_t updateTime(int pid);

#endif
