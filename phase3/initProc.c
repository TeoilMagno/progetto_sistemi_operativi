#include "./headers/initProc.h"
#include "./headers/vmSupport.h"

swap_t swap_pool[POOLSIZE];
int swapPoolSemaphore = 1;
int masterSemaphore = 0;
int shellSemaphore = 0;
int terminalWriteSem = 1;
int terminalReadSem = 1;
support_t supportPool[UPROCMAX];
static struct list_head supportFree_h;


void deallocateSupport(support_t* sup){
    list_add(&sup->s_list, &supportFree_h); //Aggiungo la Support Structures alla lista delle strutture inattive
}

//Se non ci sono strutture support inattive, ritorno NULL
//Se c'è almeno una struttura inattiva, la rimuovo dalla lista e la ritorno
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

void test()
{
    //Inizializzo le strutture della swap pool
    initSwapStructs();    
    
    //Inizializzo supportFree_h: la lista delle Support Structures inattive
    INIT_LIST_HEAD(&supportFree_h);
    
    //Aggiungo ogni Support Structures in supportPool alla lista delle strutture inattive 
    for(int c=0; c<UPROCMAX; c++){
        deallocateSupport(&supportPool[c]);
    }

    support_t *shellSup=allocateSupport(); //alloco la Support Structures per lo shell

    if(shellSup == NULL){
        PANIC();
    }
}
