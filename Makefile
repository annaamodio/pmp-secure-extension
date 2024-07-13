######################################################################
# Check for sanity to avoid later confusion

ifneq ($(words $(CURDIR)),1)
 $(error Unsupported: GNU Make cannot build in directories containing spaces, build elsewhere: '$(CURDIR)')
endif

######################################################################
# Set up Verilator variables

# If $VERILATOR_ROOT isn't in the environment, we assume it is part of a
# package install, and verilator is in your path. Otherwise find the
# binary relative to $VERILATOR_ROOT (such as when inside the git sources).
ifeq ($(VERILATOR_ROOT),)
VERILATOR = verilator
VERILATOR_COVERAGE = verilator_coverage
else
export VERILATOR_ROOT
VERILATOR = $(VERILATOR_ROOT)/bin/verilator
VERILATOR_COVERAGE = $(VERILATOR_ROOT)/bin/verilator_coverage
endif

# Generate C++
VERILATOR_FLAGS += --cc
# Add VERILATOR define. Already predefined by verilator
VERILATOR_FLAGS += -DVERILATOR=1
# Generate Binary
VERILATOR_FLAGS += --main
# Generate makefile dependencies (not shown as complicates the Makefile)
#VERILATOR_FLAGS += -MMD
# Optimize
#VERILATOR_FLAGS += -x-assign fast
# Warn abount lint issues; may not want this on less solid designs
VERILATOR_FLAGS += -Wall
# Make waveforms
#VERILATOR_FLAGS += --trace
# Check SystemVerilog assertions
#VERILATOR_FLAGS += --assert
# Generate coverage analysis
#VERILATOR_FLAGS += --coverage
# Run Verilator in debug mode
#VERILATOR_FLAGS += --debug
# Add this trace to get a backtrace in gdb
#VERILATOR_FLAGS += --gdbbt
# Warings to ignore while Verilating
VERILATOR_FLAGS += -Wno-PINCONNECTEMPTY -Wno-GENUNNAMED -Wno-DECLFILENAME \
			-Wno-BLKANDNBLK -Wno-UNUSEDSIGNAL -Wno-UNUSEDPARAM            \
			-Wno-UNOPTFLAT -Wno-IMPLICIT -Wno-VARHIDDEN -Wno-WIDTH        \
			-Wno-ASCRANGE -Wno-CMPCONST

######################################################################
# Set up Project variables

RTL_DIR = rtl
TOP_MODULE = CSoC_top

VERILATOR_INCLUDE += -y $(RTL_DIR) -y $(RTL_DIR)/cv32e40s           \
					-y $(RTL_DIR)/cv32e40s/include -y $(RTL_DIR)/dm \
					-y $(RTL_DIR)/prim -y $(RTL_DIR)/sim_only

#Targets

.PHONY: info build trace debug clean

sim_name = $(firstword $(subst @, ,$@))
soft_name = $(word 2,$(subst @, ,$@))

info:
	@echo "To use the environment, follow the steps in order:"
	@echo "(1) Build the library"
	@echo "   Run \"make build\" to verilate the SoC and build libcsoc."
	@echo "   Run \"make trace\" to verilate the SoC and build libcsoc with tracing support."
	@echo "   Run \"make debug\" to verilate the SoC and build libcsoc with debug support."
	@echo "(2) Build the simulation"
	@echo "   Run \"make [sim name]@[soft name]\" to build the [sim name] simulation with [soft name] software for the SoC."
	@echo "(3) Run the simulation"
	@echo "   Run \"./csoc_simulation\" to run the simulation you just built"

debug: VERILATOR_FLAGS += --debug
debug: LIBSETTING = debug
debug: build

trace: VERILATOR_FLAGS += --trace
trace: LIBSETTING = trace
trace: build

build:
	$(VERILATOR) $(VERILATOR_FLAGS) --Mdir libcsoc/verilated_soc --top-module $(TOP_MODULE) $(RTL_DIR)/$(TOP_MODULE).sv $(VERILATOR_INCLUDE)
	make -C libcsoc/verilated_soc -f V$(TOP_MODULE).mk
	make -C libcsoc $(LIBSETTING)

%:
	@if [ "@" = "$(findstring @,$@)" ] && [ -d "simulation/$(sim_name)" ] && [ -d "software/$(soft_name)" ]; then \
		make -C simulation/$(sim_name);                                                                           \
		make -C software/$(soft_name);                                                                            \
	else echo "Simulation or Software not found"; fi

clean:
	@rm -rf libcsoc/verilated_soc
	@rm -f *_simulation
	@rm -f *.log
	@rm -f vmem/*
	@rm -f waves/*
	@make -C libcsoc clean
	@make -C simulation -f clean_all.mk
	@make -C software -f clean_all.mk
	@echo "Project cleaned"
