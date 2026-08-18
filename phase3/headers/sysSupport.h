#ifndef SYS_SUPPORT
#define SYS_SUPPORT

#include "../../headers/types.h"
#include "../../headers/klog.h"
#include "./const.h"
#include <uriscv/cpu.h>
#include <uriscv/liburiscv.h>
#include <uriscv/arch.h>

extern int masterSemaphore;
extern int shellSemaphore;
extern int terminalWriteSem;
extern int terminalReadSem;
extern support_t supportPool[UPROCMAX];

void generalExceptionHandler();
void supportSyscallHandler(state_t *state, int asid);
void programTrapHandler(int asid);
void TerminateSys(int asid);
void WriteTerminalSys(state_t *state, int asid);
void ReadTerminalSys(state_t *state, int asid);
void ExecuteSys(state_t *state, int asid);

#endif
