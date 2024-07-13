# pmp-secure-extension

# PMP security extension
- Periferica 'mock' per la modifica del livello di privilegio mappata a 0x40000.
- i file lib/pmp.h e lib/pmp.c sono i file di libreria necessari per accedere alla PMP
- il file lib/security_mock.h contiene i mock delle istruzioni che si dovrebbero implementare.

# HDL
i file verilog modificati sono:
- CSoc_top.sv
- sim_only/secure_control.sv (nuova periferica)
- *_pmp.sv, *_mpu.sv, *_core.sv, *_cs_registers.sv, *_load_store_unit.sv, *if_stage.sv

# MOCK : Importante
tutte le modifiche che dovranno essere rimosse sono contrassegante con il commento: "MOCK: to be removed"

#  altre
le altre cartelle contengono il progetto col mio test per le modifiche hardware. 