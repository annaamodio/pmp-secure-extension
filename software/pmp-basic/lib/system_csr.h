#ifndef CSR_H_
#define CSR_H_

#include <stdint.h>

#define WRITE_TO_CSR(addr,value) __asm__ volatile ("csrw %0, %1" :: "r" (addr), "i" (value))
#define READ_FROM_CSR(addr,value) __asm__ volatile ("csrr %0, %1" : "=m" (value) : "r" (addr))
//take value

#endif