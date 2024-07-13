# Compile defines
ifeq ($(VERILATOR_ROOT),)
VERILATOR_INCLUDE = /usr/local/share/verilator/include
else
VERILATOR_INCLUDE = $(VERILATOR_ROOT)/include
endif

INCLUDE = -Iverilated_soc -I$(VERILATOR_INCLUDE) -I$(VERILATOR_INCLUDE)/vltstd
CFLAGS = -c

# Targets

.PHONY: build trace debug clean

build: libcsoc.a

trace: CFLAGS += -DTRACING
trace: build

debug: CFLAGS += -O0 -g
debug: build

libcsoc.a: libcsoc.o uart.o cqueue.o verilated_vcd_c.o verilated_soc/*.o
	ar rcs libcsoc.a libcsoc.o uart.o cqueue.o verilated_vcd_c.o verilated_soc/*.o
	
libcsoc.o: libcsoc.cpp
	g++ $(INCLUDE) $(CFLAGS) libcsoc.cpp -o libcsoc.o

uart.o: uart.c
	g++ $(INCLUDE) $(CFLAGS) uart.c -o uart.o

cqueue.o: cqueue.c
	g++ $(INCLUDE) $(CFLAGS) cqueue.c -o cqueue.o

verilated_vcd_c.o: $(VERILATOR_INCLUDE)/verilated_vcd_c.cpp
	g++ $(INCLUDE) $(CFLAGS) $(VERILATOR_INCLUDE)/verilated_vcd_c.cpp -o verilated_vcd_c.o

clean:
	@rm -f libcsoc.a
	@rm -f *.o
