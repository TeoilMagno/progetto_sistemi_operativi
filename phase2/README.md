# IMPORTANTE!
Per controllare le ultime correzione di fase 2, bisogna modificare CMakeLists.txt reinserendo phase2/p2test.c
questa funzione contiene già una funzione "test" altrimenti dichiarata in phase3/initProc.c che creerebbe conflitto, quindi si necessita rimuovere almeno questo file da CMakeLists.txt
ultima cosa importate, per testare fase 2 è raccomandato usare la funzione uTLB_RefillHandler contenuta in phase2/p2test.c quella in phase2/exceptions.c è ancora work in progress per fase 3
l'uTLB_RefillHandler di p2test non fa effetivamente nulla e va bene per controllare fase 2, quello in exceptions.c dovrebbe essere un effettivo algoritmo di paginazione da implementare per fase 3, ed è ancora da testare, quindi si consiglia di commentarlo durante il testing di fase 2
