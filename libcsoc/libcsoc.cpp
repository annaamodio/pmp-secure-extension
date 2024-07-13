#include "libcsoc.h"
#include "assert.h"
#include "stdbool.h"

// These defines are for the specific configuration of the verilated SoC
// and should not be modified unless also the rtl is.

#define RST_SYS_ON 0
#define RST_SYS_OFF 1

#define CLOCK_SYS_HIGH 1
#define CLOCK_SYS_LOW 0

#define CSOC_CLOCKFREQ 50000000
#define UART_BAUDRATE  500000

// Internal defines
#define UART_BAUDLIMIT (CSOC_CLOCKFREQ/UART_BAUDRATE)

#define TRACE_FOLDER_NAME "waves"
#define TRACE_FILE_PATH TRACE_FOLDER_NAME "/" TRACE_FILE_NAME

static void clock_period(csoc_t *soc, bool rst_state){

    assert(soc);

    // If trace is enabled we dump the waves
    #ifdef TRACING
    soc->ctx->trace->dump(soc->ctx->verilated_ctx->time());
    #endif        

    soc->soc_top->clk_sys_i = CLOCK_SYS_HIGH;
    soc->soc_top->rst_sys_ni = rst_state;
    soc->soc_top->eval();
    soc->ctx->verilated_ctx->timeInc(1);

    // If trace is enabled we dump the waves
    #ifdef TRACING
    soc->ctx->trace->dump(soc->ctx->verilated_ctx->time());
    #endif

    soc->soc_top->clk_sys_i = CLOCK_SYS_LOW;
    soc->soc_top->eval();
    soc->ctx->verilated_ctx->timeInc(1);
    
    // Every clock period we increment time by 2 timeprecision
    // This should be alligned with the clock frequency.
    // Ideally 2 timeprecision == 1/CSOC_CLOCKFREQ should hold true.
}

int csoc_ctx_init(csoc_ctx_t *ctx, int ctx_arg_c, char **ctx_args){

    assert(ctx);

    ctx->ctx_arg_c = ctx_arg_c;
    ctx->ctx_args = ctx_args;
    ctx->verilated_ctx = new VerilatedContext;
    if (!ctx->verilated_ctx)
        return -1;
    
    ctx->verilated_ctx->commandArgs(ctx->ctx_arg_c, ctx->ctx_args);
    
    #ifdef TRACING
    printf("LIBCSOC: Enabling tracing. Waves saved in: %s\n",TRACE_FILE_PATH);
    Verilated::mkdir(TRACE_FOLDER_NAME);
    ctx->verilated_ctx->traceEverOn(true);
    ctx->trace = new VerilatedVcdC;
    if (!ctx->trace)
        return -1;
    #else
        ctx->trace = NULL;
    #endif

    return 0;
}

void csoc_ctx_destroy(csoc_ctx_t *ctx){

    assert(ctx && ctx->verilated_ctx);

    #ifdef TRACING
    ctx->trace->close();
    delete ctx->trace;
    #endif

    delete ctx->verilated_ctx;
}

/*
*  Initialize the SoC with the given context.
*/
int csoc_soc_init(csoc_t *soc, csoc_ctx_t *ctx){

    assert(soc && ctx && ctx->verilated_ctx);

    soc->ctx = ctx;
    soc->soc_top = new VCSoC_top(soc->ctx->verilated_ctx);
    if (!soc->soc_top)
        return -1;
    
    // Tracing init
    #ifdef TRACING
    soc->soc_top->trace(ctx->trace,TRACE_DEPTH);
    ctx->trace->open(TRACE_FILE_PATH);
    #endif
    

    // First evaluations are made to generate a negedge in the rst_sys_ni signal
    soc->soc_top->clk_sys_i = CLOCK_SYS_LOW;
    soc->soc_top->rst_sys_ni = RST_SYS_OFF;
    soc->soc_top->eval();
    soc->soc_top->rst_sys_ni = RST_SYS_ON;
    soc->soc_top->eval();

    // Init the receive queue
    cqueue_init(&soc->recv_queue);
    // Init the uart
    uart_init(&soc->uart,UART_BAUDLIMIT);

    // Initialize the uart_rx_i to 1 by calling the uart_tx_protocol with start == false
    uart_tx_protocol(&soc->uart,&soc->soc_top->uart_rx_i,'d', false);
    return 0;
}

/*
*  End the simulation executing final blocks and freeing
*   memory.
*/
void csoc_soc_destroy(csoc_t *soc){

    assert(soc);

    soc->soc_top->final();
    delete soc->soc_top;
}

/*
*  The [soc] simulation will proceed for [cycles] clock cycles
*   or until it gets a $finish or $stop/error. Returns a non-zero
*   value if the simulation got a $finish or $stop/error while
*   executing.
*/
int csoc_soc_proceedClockCycles(csoc_t *soc,uint64_t cycles){

    assert(soc);

    uint8_t rx_byte;

    while (cycles != 0 && !soc->ctx->verilated_ctx->gotFinish()){

        clock_period(soc, RST_SYS_OFF);
        cycles--;
        
        uart_tx_protocol(&soc->uart,&soc->soc_top->uart_rx_i,'d', false);

        if (uart_rx_protocol(&soc->uart,soc->soc_top->uart_tx_o, &rx_byte) == RX_END)
            cqueue_enq(&soc->recv_queue, rx_byte);
        
    }

    if (cycles == 0)
        return 0;
    else
        return -1;
}

/*
*  The SoC simulation will proceed for the clock cycles needed
*   for sending the specified number of bytes to the SoC or 
*   until it gets a $finish or $stop/error. No assumptions are 
*   made on the state of the SoC so - since a UART is used as
*   the underlying hardware mechanism - there is no guarantee
*   that the bytes are received or lost. The user code in both
*   this program and the SoC should deal with the synchronization.
*   Returns a non-zero value if the simulation got a $finish or 
*   $stop/error while executing.
*/
int csoc_soc_proceedSend(csoc_t *soc, const void *buf, size_t len){

    assert(soc);
    assert(buf);

    uint8_t rx_byte;
    size_t i = 0;
    uint8_t *buffer = (uint8_t *)buf;
    int res_tx, res_rx;

    while (i < len && !soc->ctx->verilated_ctx->gotFinish()){   
        
        clock_period(soc, RST_SYS_OFF);
        res_tx = uart_tx_protocol(&soc->uart,&soc->soc_top->uart_rx_i,buffer[i], true);
        res_rx = uart_rx_protocol(&soc->uart,soc->soc_top->uart_tx_o, &rx_byte);

        if (res_rx == RX_END)
            cqueue_enq(&soc->recv_queue, rx_byte);
        if (res_tx == TX_END)
            i++;
    }

    if (i == len)
        return 0;
    else
        return -1;
}

/*
*  The SoC simulation will proceed for the clock cycles needed
*   for receiving the specified number of bytes from the SoC or 
*   until it gets a $finish or $stop/error. It is possible that 
*   some or all the bytes are alreay received while simulation 
*   was proceeding due to the other function calls. In this case
*   the simulation will proceed only for the clock cycles needed
*   to receive the remaining bytes.
*   Returns a non-zero value if the simulation got a $finish or 
*   $stop/error while executing.
*/
int csoc_soc_proceedRecv(csoc_t *soc, void *buf, size_t len){

    assert(soc);
    assert(buf);

    uint8_t *buffer = (uint8_t *)buf;
    size_t i = 0;
    uint8_t rx_byte;

    // First we check for already received bytes
    while (!cqueue_empty(&soc->recv_queue) && i < len)
        cqueue_deq(&soc->recv_queue, &buffer[i++]);
    
    // Then we receive the others and put them directly in the user buffer
    while (i < len && !soc->ctx->verilated_ctx->gotFinish()){
        clock_period(soc, RST_SYS_OFF);
        uart_tx_protocol(&soc->uart,&soc->soc_top->uart_rx_i,'d', false);
        if (uart_rx_protocol(&soc->uart,soc->soc_top->uart_tx_o, &rx_byte) == RX_END)
            buffer[i++] = rx_byte;
    }
    
    if (i == len)
        return 0;
    else
        return -1;
}