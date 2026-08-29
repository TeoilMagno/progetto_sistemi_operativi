#include "./headers/initProc.h"
#include "./headers/vmSupport.h"

swap_t swap_pool[POOLSIZE]; //Tabella dei frame della Swap Pool
int swapPoolSemaphore = 1; //Mutex per l'accesso alla tabella della Swap Pool
int masterSemaphore = 0;
int shellSemaphore = 0;
int terminalWriteSem = 1;
int terminalReadSem = 1;
support_t supportPool[UPROCMAX];
static struct list_head supportFree_h; //Lista delle Support Structures non ancora assegnate 
int flashSemaphore[UPROCMAX];


void deallocateSupport(support_t* sup){
    list_add(&sup->s_list, &supportFree_h); //Aggiungo la Support Structures alla lista delle strutture inattive
}

/*Se non ci sono strutture support inattive, ritorno NULL
Se c'è almeno una struttura inattiva, la rimuovo dalla lista e la ritorno*/
support_t* allocateSupport(){
    if(list_empty(&supportFree_h)){
        return NULL;
    }
    else{
        struct list_head *entry=supportFree_h.next;

        list_del(entry);
        support_t *sup=container_of(entry, support_t, s_list);

        return sup;
    }
}

//Inizializza la Support Structure assegnata a un U-proc
void initSupportStructure(support_t *sup, int asid){
    sup->sup_asid = asid; //Associo la Support Structure all'U-proc
    
    /*Inizializzo le entry da 0 a 30*/
    for(int i = 0; i < USERPGTBLSIZE-1; i++){
        sup->sup_privatePgTbl[i].pte_entryHI = ((0x80000 + i) << VPNSHIFT) | (asid << ASIDSHIFT); //La VPN delle pagine è compresa tra 0x80000 e 0x8001E
        sup->sup_privatePgTbl[i].pte_entryLO = DIRTYON;
    }
    
    /*Inizializzo la entry 31*/
    sup->sup_privatePgTbl[USERPGTBLSIZE - 1].pte_entryHI = (STACK_PAGE << VPNSHIFT) | (asid << ASIDSHIFT);
    sup->sup_privatePgTbl[USERPGTBLSIZE - 1].pte_entryLO = DIRTYON;
    /*Inizializzazione del Pager per le eccezioni della TLB*/
    sup->sup_exceptContext[PGFAULTEXCEPT].stackPtr = (memaddr)&sup->sup_stackTLB[499];
    sup->sup_exceptContext[PGFAULTEXCEPT].status = IEPON | IMON | TEBITON;
    sup->sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr)pager;
    /*Inizializzazione del gestore per syscall utente e program trap*/
    sup->sup_exceptContext[GENERALEXCEPT].stackPtr = (memaddr)&sup->sup_stackGen[499];
    sup->sup_exceptContext[GENERALEXCEPT].status = IEPON | IMON | TEBITON;
    sup->sup_exceptContext[GENERALEXCEPT].pc = (memaddr)generalExceptionHandler;
}

//Inizializzazione di U-Proc
void initUProcState(state_t *state, int asid){
    /* Azzero i registri generali dello stato iniziale. */
    for(int i = 0; i < STATE_GPR_LEN; i++){
        state->gpr[i] = 0;
    }

    state->pc_epc = UPROCSTARTADDR; //Indirizzo di partenza del programma
    state->reg_sp = USERSTACKTOP; //Cima dello stack utente 
    state->status = USERPON | IEPON | IMON | TEBITON; //Imposto user mode, interrupt e local timer abilitati
    state->mie = MIE_ALL; //Abilito tutte le sorgenti degli interrupt
    state->entry_hi = asid << ASIDSHIFT; //Imposto l'ASID dell'U-Proc
}

void test()
{
    initSwapStructs(); //Inizializzo le strutture della swap pool
    
    INIT_LIST_HEAD(&supportFree_h); //Inizializzo la lista delle Support Structures libere
    
    //Aggiungo ogni Support Structures alla lista delle strutture libere  
    for(int i=0; i<UPROCMAX; i++){
        flashSemaphore[i] = 1;
        deallocateSupport(&supportPool[i]);
    }

    support_t *shellSup=allocateSupport(); //Alloco una Support Structures per lo shell

    if(shellSup == NULL){
        PANIC();
    }

    initSupportStructure(shellSup, SHELL_ASID); //Inizializzo Support Structure 
    
    /*Inizializzo lo shell*/
    state_t shellState;
    initUProcState(&shellState, SHELL_ASID);

    if(SYSCALL(CREATEPROCESS, (int) &shellState, PROCESS_PRIO_LOW, (int) shellSup) == -1){
            PANIC();
    }

    SYSCALL(PASSEREN, (int) &masterSemaphore, 0, 0);
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

