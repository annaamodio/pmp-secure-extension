// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdint.h>

#include "system_regs.h"


#define USER_FUNC  __attribute__((__section__(".user_code")))
#define M_FUNC  __attribute__((__section__(".m_code")))

#define UART_IRQ_NUM 16
#define UART_IRQ (1 << UART_IRQ_NUM)
#define DEFAULT_UART UART_FROM_BASE_ADDR(UART0_BASE)

#define GPIO_OUT GPIO_FROM_BASE_ADDR(GPIO_BASE + GPIO_OUT_REG)
#define GPIO_IN GPIO_FROM_BASE_ADDR(GPIO_BASE + GPIO_IN_REG)
#define GPIO_IN_DBNC GPIO_FROM_BASE_ADDR(GPIO_BASE + GPIO_IN_DBNC_REG)
#define GPIO_OUT_SHIFT GPIO_FROM_BASE_ADDR(GPIO_BASE + GPIO_OUT_SHIFT_REG)

#define TIMER_IRQ_NUM 7
#define TIMER_IRQ (1 << TIMER_IRQ_NUM)

#define DEFAULT_SPI SPI_FROM_BASE_ADDR(SPI0_BASE)

#define MODE_DIRECT 0x0
#define MODE_VECTOR 0x1


/**
 * Declaration of the irq handlers. These MUST be implemented
 */
void uart_irq_handler(void) __attribute__((interrupt));
void timer_irq_handler(void) __attribute__((interrupt));

/**
 * we must implement pmp error handlers
 * 
 */ 
/**
 * Initialize the sytem vector table and enable interrupts
*/
void system_init(void);

/**
 * Writes character to default UART. Signature matches c stdlib function
 * of the same name.
 *
 * @param c Character to output
 * @returns Character output (never fails so no EOF ever returned)
 */
int putchar(int c);

int printf(const char *fmt, ...);

/**
 * Reads character from default UART. Signature matches c stdlib function
 * of the same name.
 *
 * @returns Character from the uart rx fifo
 */
int getchar(void);

/**
 * Immediately halts the simulation
 */
void sim_halt();

/**
 * Writes string to default UART. Signature matches c stdlib function of
 * the same name.
 *
 * @param str String to output
 * @returns 0 always (never fails so no error)
 */
int puts(const char *str);

/**
 * Writes ASCII hex representation of number to default UART.
 *
 * @param h Number to output in hex
 */
void puthex(uint32_t h);
void putbyte(uint32_t h);
/**
 * Writes ASCII dec representation of number to default UART.
 *
 * @param h Number to output in decimal
 */
void putdec(uint32_t n);

/**
 * Install an exception handler by writing a `j` instruction to the handler in
 * at the appropriate address given the `vector_num`.
 *
 * @param vector_num Which IRQ the handler is for, must be less than 32. All
 * non-interrupt exceptions are handled at vector 0.
 *
 * @param handle_fn Function pointer to the handler function. The function is
 * responsible for interrupt prolog and epilog, such as saving and restoring
 * register to the stack and executing `mret` at the end.
 *
 * @return 0 on success, 1 if `vector_num` out of range, 2 if the address of
 * `handler_fn` is too far from the exception handler base to use with a `j`
 * instruction.
 */
int install_exception_handler(uint32_t vector_num, void(*handler_fn)(void));
void register_default_exception_handler(void(*handler_fn)(void),unsigned int mode);

/**
 * Set per-interrupt enables (`mie` CSR)
 *
 * @param enable_mask Any set bit is set in `mie`, enabling the interrupt. Bits
 * not set in `enable_mask` aren't changed.
 */
void enable_interrupts(uint32_t enable_mask);

/**
 * Clear per-interrupt enables (`mie` CSR)
 *
 * @param enable_mask Any set bit is cleared in `mie`, disabling the interrupt.
 * Bits not set in `enable_mask` aren't changed.
 */
void disable_interrupts(uint32_t disable_mask);

/**
 * Set the global interrupt enable (the `mie` field of `mstatus`). This enables
 * or disable all interrupts at once.
 *
 * @param enable Enable interrupts if set, otherwise disabled
 */
void set_global_interrupt_enable(uint32_t enable);

unsigned int get_mepc();
unsigned int get_mcause();
unsigned int get_mtval();
uint32_t get_mcycle(void);
void reset_mcycle(void);


#endif
