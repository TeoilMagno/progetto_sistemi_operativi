#include "./headers/sysSupport.h"

void TerminateSys(int asid)
{
    if(asid==SHELL_ASID)
    {
        SYSCALL(VERHOGEN, (int) &masterSemaphore, 0, 0);
    }
    else
    {
        SYSCALL(VERHOGEN, (int) &shellSemaphore, 0, 0);
    }
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

void WriteTerminalSys(state_t *state, int asid)
{
    char *virtAddr = (char *) state->reg_a1;
    int len = state->reg_a2;
    if(len<0 || len>128 || (memaddr)virtAddr<KUSEG || (memaddr)(virtAddr+len)>USERSTACKTOP)
    {
        TerminateSys(asid);
        return;
    }
    SYSCALL(PASSEREN, (int) &terminalWriteSem, 0, 0);
    termreg_t *termAddr = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, 0);
    for(int i=0; i<len; i++)
    {
        unsigned int cmdValue = (((unsigned int) virtAddr[i])<<8) | TRANSMITCHAR;
        int status = SYSCALL(DOIO, (int) &(termAddr->transm_command), (int) cmdValue, 0);
        if((status & 0xFF) != OKCHARTRANS)
        {
            state->reg_a0=-status;
            break;
        }
        if(i==len-1){state->reg_a0=i;}
    }
    SYSCALL(VERHOGEN, (int) &terminalWriteSem, 0, 0);
    state->pc_epc +=4;
    LDST(state);
}

void ReadTerminalSys(state_t *state, int asid)
{
    char *virtAddr = (char *) state->reg_a1;
    if((memaddr)virtAddr<KUSEG || (memaddr)virtAddr>=USERSTACKTOP)
    {
        TerminateSys(asid);
        return;
    }
    SYSCALL(PASSEREN, (int) &terminalReadSem, 0, 0);
    termreg_t *termAddr = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, 0);
    int len = 0;
    while(1)
    {
        int status = SYSCALL(DOIO, (int) &(termAddr->recv_command), RECEIVECHAR, 0);
        if((status & 0xFF) != OKCHARTRANS)
        {
            state->reg_a0=-status;
            break;
        }
        char character=(char)(status>>8);
        *virtAddr=character;
        virtAddr++;
        len++;
        if(character=='\n')
        {
            state->reg_a0=len;
            break;
        }
    }
    SYSCALL(VERHOGEN, (int) &terminalReadSem, 0, 0);
    state->pc_epc+=4;
    LDST(state);
}

void ExecuteSys(state_t *state, int asid)
{
    int newAsid = state->reg_a1;
    if (asid != SHELL_ASID || newAsid<1 ||newAsid>UPROCMAX) {
        TerminateSys(asid);
        return;
    }
    state_t newState;
    for(int i=0; i<STATE_GPR_LEN; i++){newState.gpr[i]=0;}
    newState.pc_epc = UPROCSTARTADDR;
    newState.status = USERPON | IEPON | TEBITON;
    newState.entry_hi = newAsid << ASIDSHIFT;
    newState.reg_sp = USERSTACKTOP;
    if(SYSCALL(CREATEPROCESS, (int) &newState, PROCESS_PRIO_LOW, (int) &supportPool[newAsid-1])==-1)
    {
        state->reg_a0=-1;
        state->pc_epc+=4;
        LDST(state);
        return;
    }
    SYSCALL(PASSEREN, (int) &shellSemaphore, 0, 0);
    state->pc_epc+=4;
    LDST(state);
}

void generalExceptionHandler()
{
    support_t *sup = (support_t *) SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t* state = &(sup->sup_exceptState[GENERALEXCEPT]);
    int asid = sup->sup_asid;
    unsigned int excCode = (state->cause & GETEXECCODE) >> CAUSESHIFT;
    if (excCode == 8)
    {
        supportSyscallHandler(state, asid);
    }
    else
    {
        programTrapHandler(asid);
    }
}

void supportSyscallHandler(state_t* state, int asid){
    switch(state->reg_a0)
    {
        case TERMINATE:
            TerminateSys(asid);
            break;
        case WRITETERMINAL:
            WriteTerminalSys(state, asid);
            break;
        case READTERMINAL:
            ReadTerminalSys(state, asid);
            break;
        case EXECUTE:
            ExecuteSys(state, asid);
            break;
        default:
            programTrapHandler(asid);
            break;
    }
}

void programTrapHandler(int asid)
{
    TerminateSys(asid);
}
