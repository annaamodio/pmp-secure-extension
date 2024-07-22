# PMP security extension

Test ed estensione della PMP per il core RISC-V cv32e40s.
Progetto di Embedded Systems AA:2023/2024 

## Repository
- **lib** contiene la libreria per la pmp (pmp.h, pmp.c)
- **rtl** contiene la descrizione del SoC in system verilog
- **simulation**, **software** e **libcsoc** sono necessarie per i test (da https://github.com/rdgarce/certify_le_soc)

## Tests
Per eseguire i test:
```
make build // per l'esecuzione classica
//OPPURE 
make trace // per eseguire con supporto per il tracing
//OPPURE
make debug // per eseguire con supporto al debug
```
Si può poi buildare nel seguente modo uno tra i tre diversi test:
```
make pmp@pmp-basic //testing delle funzionalità classiche con la libreria
make pmp@pmp-smepmp //testing delle funzionalità enhanced
make pmp@pmp-secure //testing dell'estensione secure
```
E successivamente avviare l'esecuzione con 
```
./csoc_simulation
```
I risultati sono osservabili nel log di sistema (csoc_system.log). I file pmp-basic.log, pmp-smepmp.log, pmp-secure.log contengono i risultati di run di test precedenti.

I test sono meglio documentati in software/pmp-basic, software/pmp-smepmp, software/pmp-secure.



# Estensione
L'estensione secure è implementata modificando i file (rtl/):
- CSoc_top.sv
- (cv32e40s/) *_pmp.sv, *_mpu.sv, *_core.sv, *_cs_registers.sv, *_load_store_unit.sv, *if_stage.sv

Al momento, è stato aggiunto un meccanismo fittizio tramite una periferica di simulazione (mappata a 0x40000) per consentire la modifica del livello di privilegio. A tale scopo, sono stati aggiunti i seguenti file:
- sim_only/secure_control.sv  (implementazione della nuova periferica)
- lib/security_mock.h (implementazione fittizia delle istruzioni per la modifica)
Tutte le modifiche da rimuovere per implementare completamente il supporto sono marcate con il commento `MOCK: To be removed`
