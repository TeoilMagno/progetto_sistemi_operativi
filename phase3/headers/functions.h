#ifndef FUNCTIONS_PHASE3
#define FUNCTIONS_PHASE3

#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../headers/klog.h"
#include "./const.h"
#include <uriscv/cpu.h>
#include <uriscv/liburiscv.h>
#include <uriscv/arch.h>

int findPageIndex(unsigned int pte_entryHI);

#endif
