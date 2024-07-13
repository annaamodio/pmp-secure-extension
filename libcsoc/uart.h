#ifndef UART_H_
#define UART_H_

#include "assert.h"
#include "stdint.h"
#include "stdbool.h"

// A SW emulation of a standard uart: 1 start bit, 8 bits, no parity, 1 stop bit
// There is no clock frequency nor baud frequency because the user code is responsible to
// feature a [baud_limit] (number of clock cycles per symbol) in the initialization.
// Given this number, the user code can call protocol functions. Each call corresponds
// to a positive edge of a emulated input clock to the uart system.

typedef unsigned char uart_bit_t;

typedef enum uart_rx_state{
    RX_IDLE,
    RX_ONGOING,
    RX_END
}uart_rx_state_t;

typedef enum uart_tx_state{
    TX_IDLE,
    TX_ONGOING,
    TX_END
}uart_tx_state_t;

typedef struct uart{
    unsigned int baud_limit;

    uart_rx_state_t rx_state;
    unsigned int rx_baud_cnt;
    uint8_t rx_shreg;
    uint8_t rx_bits_rcvd;

    uart_tx_state_t tx_state;
    unsigned int tx_baud_cnt;
    uint8_t tx_shreg;
    uint8_t tx_bits_sent;
}uart_t;

void uart_init(uart_t * u, unsigned int bl);
int uart_rx_protocol(uart_t *, const uart_bit_t , uint8_t *);
int uart_tx_protocol(uart_t *, uart_bit_t *, uint8_t, bool);

#endif