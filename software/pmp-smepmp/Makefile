###############################################################################
# Propject Info

PROJECT  = csoc
MCU 	 = CSOC

###############################################################################

###############################################################################
# Directories

SRCS      = $(wildcard src/*.c) $(wildcard lib/*.c)
OBJ       = obj/
OBJS      = $(addprefix $(OBJ), $(notdir $(SRCS:.c=.o)))
LD_SCRIPT = ld/$(MCU)_FLASH.ld
INC       = -Ilib -Isrc
STARTUP   = startup

###############################################################################
# Toolchain

# If you have 64-bit toolchain
# RV_PREFIX = riscv64-unknown-elf-
# 32BITCCFLAGS = -march=rv32im -mabi=ilp32
# 32BITODFLAGS = -mriscv:rv32
# CC = $(RV_PREFIX)gcc $(32BITCCFLAGS)
# OBJDUMP = $(RV_PREFIX)objdump $(32BITODFLAGS)
#

RV_PREFIX = /opt/riscv/bin/riscv32-unknown-linux-gnu-
CC        = $(RV_PREFIX)gcc 
LD        = $(RV_PREFIX)ld
OBJDUMP   = $(RV_PREFIX)objdump
OBJCOPY   = $(RV_PREFIX)objcopy

CFLAGS    = -march=rv32imc_zicsr_zifencei -O0 -c -DSIM_CTRL_OUTPUT
LDFLAGS   = -nostdlib -T$(LD_SCRIPT) -Map=output.map
SRECFLAGS = -offset 0x0000 -byte-swap 4

###############################################################################

###############################################################################
# Misc

MKDIR     = @mkdir -p $(@D) 		# Creates folders if not present

###############################################################################

.PHONY: all copy_vmem clean

all: copy_vmem

copy_vmem: bin/ram.vmem
	$(MKDIR) ../../vmem
	cp bin/ram.vmem ../../vmem/ram.vmem

bin/ram.vmem: bin/$(PROJECT).bin
	@echo "[VMEM]	Creating vmem file"
	srec_cat bin/$(PROJECT).bin -binary $(SRECFLAGS) -o bin/ram.vmem -vmem

$(OBJ)%.o: src/%.c 
	@echo "[OBJ]	Creating OBJs from src"
	$(MKDIR)
	$(CC) -o $@ $^ $(INC) $(CFLAGS)

$(OBJ)%.o: lib/%.c
	@echo "[OBJ]	Creating OBJs from libs"
	$(MKDIR)
	$(CC) -o $@ $^ $(INC) $(CFLAGS)

obj/$(STARTUP).o: src/$(STARTUP).s
	@echo "[OBJ]	Creating OBJs from $(STARTUP).s"
	$(MKDIR)
	$(CC) -o obj/$(STARTUP).o src/$(STARTUP).s $(INC) $(CFLAGS)

bin/$(PROJECT).elf: $(OBJS) obj/$(STARTUP).o
	@echo "[ELF]	Creating elf file"
	$(MKDIR)
	$(LD) -o $@ $^ $(LDFLAGS)

bin/$(PROJECT).bin: bin/$(PROJECT).elf
	@echo "[ELF]	Creating bin file"
	$(OBJCOPY) -O binary bin/$(PROJECT).elf bin/$(PROJECT).bin

clean:
	@rm -rf obj
	@rm -rf bin
