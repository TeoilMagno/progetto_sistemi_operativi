# COMPILAZIONE
compilare correttamente i vari file .c in questa cartella eseguir
```bash
make
```

questo crea file oggetto .o che vengono linkati per creaer i file considerati flash device da uriscv, questi file hanno estensione .uriscv

# CONFIG MACCHINA URISCV
Nella cartella root del progetto è stato aggiunto un nuovo file di config da usaer per i test di fase 3
questo file si chiama config_machine.json per controllare che questa macchina veda correttamente i flash device creati:
si apre "edit config" con la macchina ancora spenta
si apre il menù Device -> Flash Device, qui trovate un elenco di 8 device inclusi tutti con cartella di riferimento ./tester/
nel caso la cartella venga spostata è possibile modificare phase3_config_machine.json, modificando la directory del Flash Device

# IMPORTANTE
Al momento le cartelle del progetto sono già ordinate in modo che uriscv trovi tutti i file, quindi non c'è bisogno di modificare nulla
un controllo veloce si può fare provando ad avviare la macchina, se questa non tova i flash device allora qualcosa non è al posto giusto, altrimenti questa viene avviata correttamente
