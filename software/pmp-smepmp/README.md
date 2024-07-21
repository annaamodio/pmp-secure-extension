# PMP enhanced (smepmp) test

Questi test utilzzano la libreria per assicurare il funzionamento della pmp in conformità alla specifica enhanced (smepmp)

NB: Prima di eseguire i test, assicurarsi che l'estensione secure sia disabilitata (funzionamento nominale). Modificare il file `rtl/cv32e40s/include/cv32e40s_pkg.sv` per assicurarsi che sia presente la seguente espressione:
```
parameter bit SECURE_MONITOR = 0; 
```

I test si trovano nel file `main.c`. Viene testata la modalità Whitelist e la modalità Machine Mode Lockdown. A tale scopo è stato modificato il linkerscript in modo da separare codice/dati utente e supervisore.

Per eseguire questi test:
```
make clean
make build
make pmp@pmp-smepmp
./csoc_simulation
```

I risultati sono riportati in csoc_system.log
