#ifndef LIBCSOC_H_
#define LIBCSOC_H_

#include "verilated_vcd_c.h"
#include "VCSoC_top.h"
#include "cqueue.h"
#include "uart.h"
#include "stdlib.h"
#include "stdbool.h"

#define TRACE_FILE_NAME "trace.vcd"
#define TRACE_DEPTH 99

typedef struct csoc_ctx{

    int ctx_arg_c;
    char **ctx_args;
    VerilatedContext *verilated_ctx;
    VerilatedVcdC *trace;
} csoc_ctx_t;

typedef struct csoc {

    csoc_ctx_t *ctx;
    VCSoC_top *soc_top;
    cqueue_t recv_queue;
    uart_t uart;
} csoc_t;

int csoc_ctx_init(csoc_ctx_t *, int, char **);
void csoc_ctx_destroy(csoc_ctx_t *);

int csoc_soc_init(csoc_t *,csoc_ctx_t *);
void csoc_soc_destroy(csoc_t *);

int csoc_soc_proceedClockCycles(csoc_t *, uint64_t);
int csoc_soc_proceedSend(csoc_t *, const void *, size_t);
int csoc_soc_proceedRecv(csoc_t *, void *, size_t);

#endif