# PMP secure extension tests

Questi test utilzzano la libreria per assicurare il funzionamento della pmp in conformità ai nuovi requisiti di supporto secure.

NB: Prima di eseguire i test, assicurarsi che l'estensione secure sia **abilitata** Modificare il file `rtl/cv32e40s/include/cv32e40s_pkg.sv` per assicurarsi che sia presente la seguente espressione:
```
parameter bit SECURE_MONITOR = 1; 
```

I test si trovano nel file `main.c`. Vengono testate le nuove funzionalità modificando anche il security level

Per eseguire questi test, lanciare nella root del progetto:
```
make clean
make build
make pmp@pmp-secure
./csoc_simulation
```

I risultati sono riportati in `csoc_system.log`
