#ifndef SECURE_H
#define SECURE_H

#include <stdint.h>

#define SET_SECURE_MODE (*(int*)(0x40000)=1)
#define SET_NONSECURE_MODE (*(int*)(0x40000)=0)

extern void nonSecure();

#define SMRET_ADDRESS nonSecure

void SMC_mock(){
    SET_SECURE_MODE;
    __asm__ volatile("ecall");
}

void SMRET_mock(){
    //set secure address
    __asm__ volatile("csrw mepc, %0"::"r"(SMRET_ADDRESS));
    SET_NONSECURE_MODE;
    __asm__ volatile("mret");

}


#endif