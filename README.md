Questo è il REPO per il progetto di sistemi operativi dell'anno scolastico 2025/2026

!! Questa cartella non contiene tutti i file che servono al corretto funzionamento del progetto:
1)  Il codice implementato va testato con uriscv di seguito la REPO per installarlo
    https://github.com/virtualsquare/uriscv/
2)  questa cartella non contiene il file CmakeList.txt in quanto al variare della distro
    utlizzata variano anche le directory di installazione di uriscv con annesse librerie

!! per il corretto funzionamento bisogna recarsi a https://www.cs.unibo.it/~renzo/so/PandOSsh/ !!

scaricare starterkit.tar.gz
```bash
tar -xvf starterkit.tar.gz
```

modificare il file CmakeList.txt a seconda delle directory in cui è installato uriscv
e copiare il file nella directory di lavoro, infine eseguire:
```bash
cmake -B build
```

dopo aver implementato tutte le varie funzioni si può provare a compilare tutto il progetto con:
```bash
cmake --build build
```

Nota: per la fase 1 dobbiamo implementari alcuni metodi e alcune strutture, ma
nel file ./headers/listx.h abbiamo già a dispozione alcuni metodi da utilizzare,
forniti dai docenti

infine si consiglia di creare una file .gitignore indicando:
build/
CmakeList.txt

in modo da non creare conflitti con gli altri
