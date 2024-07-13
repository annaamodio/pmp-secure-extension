#include "uart.h"

#define ONE_ZERO(bit) ( (bit) == 0 ? 0 : 1 )

/*
*  Init the uart [u] in the idle state with the [bl]
*   baud limit.
*/
void uart_init(uart_t * u, unsigned int bl){

    assert(u);

    u->baud_limit = bl;

    u->rx_state = RX_IDLE;
    u->rx_baud_cnt = 0;
    u->rx_shreg = 0;
    u->rx_bits_rcvd = 0;
    
    u->tx_state = TX_IDLE;
    u->tx_baud_cnt = 0;

}

/*
*  Execute one clock cycle protocol of the receive unit of the uart
*   given that [rx_line] is the value of the receive line and return
*   the current state of the receive unit.
*   If the return value is RX_END then [rcvd_byte] is populated with
*   the received byte
*/
int uart_rx_protocol(uart_t * u, const uart_bit_t rx_line, uint8_t *rcvd_byte){

    assert(u);
    assert(rcvd_byte);

    switch (u->rx_state)
    {

    case RX_IDLE:
        
        if (rx_line)
            break;
        else
            u->rx_state = RX_ONGOING;
    
    case RX_ONGOING:

        u->rx_baud_cnt++;

        if (u->rx_baud_cnt >= u->baud_limit){
            
            u->rx_baud_cnt = 0;
            u->rx_bits_rcvd++;

            // 1 stop bit completely received
            if (u->rx_bits_rcvd == 10){
                *rcvd_byte = u->rx_shreg;
                u->rx_baud_cnt = 0;
                u->rx_bits_rcvd = 0;    
                u->rx_shreg = 0;
                u->rx_state = RX_IDLE;
                return RX_END;
            }
        }
        // Sample the data bits in the middle of the transitions of the 8 data bits
        else if (u->rx_bits_rcvd > 0 && u->rx_bits_rcvd < 9 && u->rx_baud_cnt == u->baud_limit/2)
            u->rx_shreg |= ( ONE_ZERO(rx_line) << (u->rx_bits_rcvd-1) );
        
        break;

    default:
        break;
    }

    return u->rx_state;

}

/*
*  Execute one clock cycle protocol of the transmitter unit of the uart
*   [u] given that [tx_line] is the value of the transmission line and 
*   return the current state of the transmitter unit. If and only if 
*   the uart transmitter modure is in TX_IDLE the user can start the
*   sending of [out_byte] setting [start_tx] to true.
*   Further calls with [start_tx] asserted are idempotent while sending the
*   previous byte.
*/
int uart_tx_protocol(uart_t * u, uart_bit_t *tx_line, uint8_t out_byte, bool start_tx){

    assert(u);
    assert(tx_line);

    switch (u->tx_state)
    {

    case TX_IDLE:
        
        if (!start_tx) {
            *tx_line = 1;
            break;
        }
        else {
            u->tx_state = TX_ONGOING;
            u->tx_shreg = out_byte;
        }
    
    case TX_ONGOING:
        
        u->tx_baud_cnt++;

        if (u->tx_baud_cnt >= u->baud_limit){
            u->tx_baud_cnt = 0;
            u->tx_bits_sent++;
            if (u->tx_bits_sent == 10) {
                u->tx_state = TX_IDLE;
                u->tx_baud_cnt = 0;
                u->tx_bits_sent = 0;
                u->tx_shreg = 0xFF;
                *tx_line = 1;
                return TX_END;
            }
        }
        else{
            if (u->tx_bits_sent == 0)
                *tx_line = 0;
            else if (u->tx_bits_sent > 0 && u->tx_bits_sent < 9)
                *tx_line = ONE_ZERO( u->tx_shreg & (1 << (u->tx_bits_sent-1)) );
            else
                *tx_line = 1;
        }
        
        break;

    default:
        break;
    }

    return u->tx_state;
}