#include <uriscv/liburiscv.h>
#include "h/tconst.h"
#include "h/print.h"

/* Definizioni delle SYSCALL (Capitolo 7 delle specifiche) */
#define TERMINATE 2
#define WRITETERMINAL 4
#define READTERMINAL 5
#define EXECUTE 6

/* Funzione di supporto per confrontare due stringhe carattere per carattere */
int str_uguali(char *str1, char *str2) {
    int i = 0;
    /* Ignoriamo l'invio (\n) che READTERMINAL lascia alla fine dell'input */
    while (str1[i] != '\0' && str2[i] != '\0' && str1[i] != '\n' && str2[i] != '\n') {
        if (str1[i] != str2[i]) return 0;
        i++;
    }
    if ((str1[i] == '\0' || str1[i] == '\n') && (str2[i] == '\0' || str2[i] == '\n')) return 1;
    return 0;
}

void main() {
    char prompt[] = "PandOS> ";
    char err[] = "Comando non trovato\n";
    char inputBuffer[128]; 

    while (1) {
        /* Svuota il buffer prima di ogni lettura */
        for(int i = 0; i < 128; i++) inputBuffer[i] = '\0';

        /* Stampa il prompt e aspetta l'input */
        SYSCALL(WRITETERMINAL, (int)prompt, 8, 0);
        SYSCALL(READTERMINAL, (int)inputBuffer, 0, 0);

        /* Gestione dell'uscita come da specifiche */
        if (str_uguali(inputBuffer, "exit")) {
            SYSCALL(TERMINATE, 0, 0, 0);
        }
        else if (str_uguali(inputBuffer, "fibEight")) {
            SYSCALL(EXECUTE, 2, 0, 0); 
        }
        else if (str_uguali(inputBuffer, "echo")) {
            SYSCALL(EXECUTE, 3, 0, 0);
        }
        else if (str_uguali(inputBuffer, "fibEleven")) {
            SYSCALL(EXECUTE, 4, 0, 0);
        }
        else if (str_uguali(inputBuffer, "uname")) {
            SYSCALL(EXECUTE, 5, 0, 0);
        }
        else if (str_uguali(inputBuffer, "date")) {
            SYSCALL(EXECUTE, 6, 0, 0);
        }
        else if (str_uguali(inputBuffer, "sl")) {
            SYSCALL(EXECUTE, 7, 0, 0);
        }
        else if (str_uguali(inputBuffer, "calc")) {
            SYSCALL(EXECUTE, 8, 0, 0);
        }
        else if (!str_uguali(inputBuffer, "")) {
            SYSCALL(WRITETERMINAL, (int)err, 20, 0);
        }
    }
}