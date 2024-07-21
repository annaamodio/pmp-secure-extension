# PMP basic tests

Questi test utilzzano la libreria per assicurare il funzionamento della pmp in conformità alla specifica base. 

NB: Prima di eseguire i test, assicurarsi che l'estensione secure sia disabilitata (funzionamento nominale). Modificare il file `rtl/cv32e40s/include/cv32e40s_pkg.sv` per assicurarsi che sia presente la seguente espressione:
```
parameter bit SECURE_MONITOR = 0; 
```

I test si trovano nel file `main.c`. Vengono configurate delle regioni e si testa l'accesso in base ai permessi da modalità utente e supervisore.

Per eseguire questi test:
```
make clean
make build
make pmp@pmp-basic
./csoc_simulation
```

I risultati sono riportati in csoc_system.log
