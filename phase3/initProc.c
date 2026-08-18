#include "./headers/initProc.h"

swap_t swap_pool[POOLSIZE];
int swapPoolSemaphore = 1;
int masterSemaphore = 0;
int shellSemaphore = 0;
int terminalWriteSem = 1;
int terminalReadSem = 1;
support_t supportPool[UPROCMAX];

void test()
{

}
