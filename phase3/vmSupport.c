#include "./headers/vmSupport.h"

extern swap_t swap_pool[POOLSIZE];

//semaforo per avere mutua esclusione sull'accesso alla swap pool
extern int swapPoolSemaphore;
static int frameIndex=0;

void initSwapStructs() 
{
    int i;
    for (i = 0; i < POOLSIZE; i++)
    {
        swap_pool[i].sw_asid = -1;
        swap_pool[i].sw_pageNo = -1;
        swap_pool[i].sw_pte = NULL;
    }
}

int findPageIndex(unsigned int pte_entryHI)
{
  int vpn = ENTRYHI_GET_VPN(pte_entryHI);

  if(vpn == STACK_PAGE)
    return USERPGTBLSIZE - 1;
  else
    return vpn - 0x80000;
}


void readFromDevice(pteEntry_t *page, swap_t *frame, int p)
{
  //ottengo l'ASID del processo che ha causato il page fault
  unsigned int asid = (unsigned int)((page->pte_entryHI & 0x00000fff) >> ASIDSHIFT);
  //grazie all'ASID ottengo il puntatore al device col processo che ha causato il page fault
  dtpreg_t *devReg =(dtpreg_t *) ((memaddr) DEV_REG_ADDR(IL_FLASH, asid-1));
  //prepare ll'operazione di read
  //in uriscv read e write vengono effettuate per DMA
  //quindi indico in data0 l'indirizzo fisico in cui scrivere la pagina p del device
  devReg->data0 = (memaddr) frame;
  //indico quale paina deve essere letta e preparo il comando per la read
  int readCommand = (p << 8) | FLASHREAD;
  int status = SYSCALL(DOIO, (int)devReg->command, (int)readCommand, 0);

  if((status & 0xff) == 5)
  {
    programTrapHandler(asid);
  }
}

void writeToDevice(swap_t *frame)
{
  //inizio azione atomica
  //disabilito gli interrupts
  setSTATUS(getSTATUS() & ~MSTATUS_MIE_MASK);

  pteEntry_t *ptu = frame->sw_pte; //Page To Update
  //aggiornando la pagina già occupante il frame
  ptu->pte_entryLO = (ptu->pte_entryLO) & VALIDOFF;

  //controllo se la pagine è nella cache della TLB
  setENTRYHI(ptu->pte_entryHI);
  TLBP();
  unsigned int index = getINDEX();

  //se è in cache va aggiornata
  if(!(index & PRESENTFLAG))
  {
    setENTRYLO(ptu->pte_entryLO);
    TLBWI();
  }

  //riabilito gli interrupts, fine azione atomica
  setSTATUS(getSTATUS() | MSTATUS_MIE_MASK);
  //fine azione atomica

  //aggiorno il backing store del vecchio processo con il nuovo frame
  //ottengo l'ASID del processo di cui scarico il frame
  unsigned int asid = frame->sw_asid;
  //grazie all'ASID ottengo il puntatore al device col processo che ha causato il page fault
  dtpreg_t *devReg =(dtpreg_t *) ((memaddr) DEV_REG_ADDR(IL_FLASH, asid-1));
  //quindi indico in data0 l'indirizzo fisico da copiare nella pagina p del device
  devReg->data0 = (memaddr) frame;
  //indico quale paina deve essere letta e preparo il comando per la read
  int writeCommand = (frame->sw_pageNo << 8) | FLASHWRITE;
  int status = SYSCALL(DOIO, (int)devReg->command, (int) writeCommand, 0);

  if((status & 0xff) == 4)
  {
    programTrapHandler(asid);
  }
}

void pager()
{
  support_t *sup = (support_t *) SYSCALL(GETSUPPORTPTR, 0, 0, 0);

  if(sup != NULL)
  {
    state_t* state = &sup->sup_exceptState[0];
    unsigned int cause = state->cause;
    unsigned int excCode = (cause & GETEXECCODE) >> CAUSESHIFT;

    if(excCode == TLBINVLDMOD)
    {
      programTrapHandler(sup->sup_asid);
      return;
    }

    SYSCALL(PASSEREN, (int)&swapPoolSemaphore, 0, 0);

    int p = findPageIndex( state->entry_hi);
    pteEntry_t *page = &sup->sup_privatePgTbl[p];

    int fi = pageReplacement(); // frameIndex
    swap_t *frame = &swap_pool[fi];
    
    if(frame->sw_asid != -1) //il frame è occupato
    {
      //il frame è occupato, quindi lo scarico sulla memoria del device corrispondente
      writeToDevice(frame); 
    }

    //poi vado a scrivere nello stesso frame la pagina richiesta
    readFromDevice(page, frame, p);

    //inizio azione atomica
    //disabilito gli interrupts
    setSTATUS(getSTATUS() & ~MSTATUS_MIE_MASK);
    //carico finalmente la nuova pagina in swap pool
    frame->sw_asid = sup->sup_asid;
    frame->sw_pageNo = p;
    frame->sw_pte = page;

    //la nuova pagina è valida
    page->pte_entryLO = page->pte_entryLO | VALIDON;
    //update del fi senza variare i flags
    page->pte_entryLO = (page->pte_entryLO & 0xf) | (fi << 4);

    setENTRYHI(page->pte_entryHI);
    setENTRYLO(page->pte_entryLO);
    TLBWI();

    //riabilito gli interrupts, fine azione atomica
    setSTATUS(getSTATUS() | MSTATUS_MIE_MASK);
    //fine azione atomica

    //rilascio mutua esclusione
    SYSCALL(VERHOGEN, (int)&swapPoolSemaphore, 0, 0);

    LDST(state);
  }
}

//returns the index of a framed in the swap pool to be swap-in
int pageReplacement()
{
  frameIndex++;
  return frameIndex % POOLSIZE;
}
