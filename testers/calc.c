#include <uriscv/liburiscv.h>
#include "h/tconst.h"
#include "h/print.h"

#define TERMINATE 2
#define WRITETERMINAL 4
#define READTERMINAL 5

/* Funzione per convertire il risultato numerico in stringa da stampare (range da -8 a 81) */
void int_to_str(int num, char *str, int *len) {
    int i = 0;
    if (num < 0) {
        str[i++] = '-';
        num = -num;
    }
    if (num > 9) {
        str[i++] = (num / 10) + '0';
        str[i++] = (num % 10) + '0';
    } else {
        str[i++] = num + '0';
    }
    str[i++] = '\n'; /* Andiamo a capo alla fine */
    str[i] = '\0';
    *len = i;
}

void main() {
    char prompt[] = "Calc> ";
    char err[] = "Errore\n";
    char div0[] = "Div 0\n";
    char input[128];
    char output[10];
    int out_len;
    
    /* Svuota il buffer prima della lettura */
    for(int i = 0; i < 128; i++) input[i] = '\0';

    /* 1. Stampa il prompt e leggi l'input (SYS4 e SYS5) */
    SYSCALL(WRITETERMINAL, (int)prompt, 6, 0);
    SYSCALL(READTERMINAL, (int)input, 0, 0);

    /* 2. Verifica che i caratteri inseriti siano effettivamente numeri */
    if (input[0] < '0' || input[0] > '9' || input[2] < '0' || input[2] > '9') {
        SYSCALL(WRITETERMINAL, (int)err, 7, 0);
        SYSCALL(TERMINATE, 0, 0, 0);
    }

    /* 3. Converte da ASCII a intero ('3' diventerà il numero 3) */
    int n1 = input[0] - '0';
    char op = input[1];
    int n2 = input[2] - '0';
    int res = 0;

    /* 4. Logica della calcolatrice */
    if (op == '+') res = n1 + n2;
    else if (op == '-') res = n1 - n2;
    else if (op == '*') res = n1 * n2;
    else if (op == '/') {
        if (n2 == 0) {
            SYSCALL(WRITETERMINAL, (int)div0, 6, 0);
            SYSCALL(TERMINATE, 0, 0, 0);
        }
        res = n1 / n2;
    } else {
        SYSCALL(WRITETERMINAL, (int)err, 7, 0);
        SYSCALL(TERMINATE, 0, 0, 0);
    }

    /* 5. Converti il risultato in stringa e stampalo */
    int_to_str(res, output, &out_len);
    SYSCALL(WRITETERMINAL, (int)output, out_len, 0);

    /* 6. Termina per restituire il controllo alla shell (SYS2) */
    SYSCALL(TERMINATE, 0, 0, 0);
}