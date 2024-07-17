#include "pmp.h"

// Other utility functions

/**
 * returns 1 if secure, 0 if non secure
 */
static int get_security_state(){
    //MOCK. To be modified
    // should read register or in some way know the security level
    // (se vuoi eliminarlo, fallo: serve per controllare anche ad alto livello se la regola è SLOCKED e ritornare errore)
    // viene chiamato in configure_region
    int v = *(int*)0x40000;
    return v;
}

/**
 * Reads and returns pmpcfg(number) register 
 */
static uint32_t read_pmpcfg(uint8_t number){
    uint32_t val_read;
    switch (number)
    {
    case 0:
        __asm__ volatile ("csrr %0, pmpcfg0" : "=r" (val_read));
        break;
    case 1:
        __asm__ volatile ("csrr %0, pmpcfg1" : "=r" (val_read));
        break;
    case 2:
        __asm__ volatile ("csrr %0, pmpcfg2" : "=r" (val_read));
        break;
    case 3:
        __asm__ volatile ("csrr %0, pmpcfg3" : "=r" (val_read));
        break;
    #if PMP_REGIONS == 64
    case 4:
        __asm__ volatile ("csrr %0, pmpcfg4" : "=r" (val_read));
        break;
    case 5:
        __asm__ volatile ("csrr %0, pmpcfg5" : "=r" (val_read));
        break;
    case 6:
        __asm__ volatile ("csrr %0, pmpcfg6" : "=r" (val_read));
        break;
    case 7:
        __asm__ volatile ("csrr %0, pmpcfg7" : "=r" (val_read));
        break;
    case 8:
        __asm__ volatile ("csrr %0, pmpcfg8" : "=r" (val_read));
        break;
    case 9:
        __asm__ volatile ("csrr %0, pmpcfg9" : "=r" (val_read));
        break;
    case 10:
        __asm__ volatile ("csrr %0, pmpcfg10" : "=r" (val_read));
        break;
    case 11:
        __asm__ volatile ("csrr %0, pmpcfg11" : "=r" (val_read));
        break;
    case 12:
        __asm__ volatile ("csrr %0, pmpcfg12" : "=r" (val_read));
        break;
    case 13:
        __asm__ volatile ("csrr %0, pmpcfg13" : "=r" (val_read));
        break;
    case 14:
        __asm__ volatile ("csrr %0, pmpcfg14" : "=r" (val_read));
        break;
    case 15:
        __asm__ volatile ("csrr %0, pmpcfg15" : "=r" (val_read));
        break;
    #endif
    default:
        break;
    }
    return val_read;
}


/**
 * Writes value to pmpcfg(region) register
 */
static void write_pmp_csr_value(uint8_t region, uint32_t value){
   uint32_t reg = (uint32_t) (region /4);
  
    switch (reg){
        case 0:
            __asm__ volatile("csrw pmpcfg0, %0"::"r"(value));
            break;
        case 1:
            __asm__ volatile("csrw pmpcfg1, %0"::"r"(value));
            break;
        case 2:
            __asm__ volatile("csrw pmpcfg2, %0"::"r"(value));
            break;
        case 3:
            __asm__ volatile("csrw pmpcfg3, %0"::"r"(value));
            break;
        #if PMP_REGIONS == 64
        case 4:
            __asm__ volatile("csrw pmpcfg4, %0"::"r"(value));
            break;
        case 5:
            __asm__ volatile("csrw pmpcfg5, %0"::"r"(value));
            break;
        case 6:
            __asm__ volatile("csrw pmpcfg6, %0"::"r"(value));
            break;
        case 7:
            __asm__ volatile("csrw pmpcfg7, %0"::"r"(value));
            break;
        case 8:
            __asm__ volatile("csrw pmpcfg8, %0"::"r"(value));
            break;
        case 9:
            __asm__ volatile("csrw pmpcfg9, %0"::"r"(value));
            break;
        case 10:
            __asm__ volatile("csrw pmpcfg10, %0"::"r"(value));
            break;
        case 11:
            __asm__ volatile("csrw pmpcfg11, %0"::"r"(value));
            break;
        case 12:
            __asm__ volatile("csrw pmpcfg12, %0"::"r"(value));
            break;
        case 13:
            __asm__ volatile("csrw pmpcfg13, %0"::"r"(value));
            break;
        case 14:
            __asm__ volatile("csrw pmpcfg14, %0"::"r"(value));
            break;
        case 15:
            __asm__ volatile("csrw pmpcfg15, %0"::"r"(value));
            break;
        #endif
        default:
            break;

    } 
}

/**
 * Writes 8 bits config value to register (cfg is the control word)
 */
static void pmp_write_cfg(uint8_t region, uint8_t cfg){

    //Maybe atomically?
    //For now we assume that pmp configuration is done in a single intialization thread
    uint32_t current_value = read_pmpcfg((int)(region/4));

    //mask for the correct 8 bits
    uint32_t mask = 0xFF << ((region%4) * 8);
    uint32_t toWrite = (uint32_t)cfg << ((region%4) * 8);

    //write correct value
    uint32_t next_value = (current_value & ~mask) | toWrite;
    write_pmp_csr_value (region,next_value);
}

/**
 * derives control word from configuration struct
 */
static void pmp_cfg_to_csr(pmp_config_t * config, uint32_t* word){
    
    uint32_t val = 0x00;
    if(config->L==1) val|=PMP_CFG_L;
    if(config->R==1) val|=PMP_CFG_R;
    if(config->W==1) val|=PMP_CFG_W;
    if(config->X==1) val|=PMP_CFG_X;
    if(config->SL==1) val|=PMP_CFG_SL;

    switch (config->A){
        case OFF:
            val|=PMP_CFG_OFF;
            break;
        case TOR:
            val|=PMP_CFG_TOR;
            break;
        case NA4:
            val|=PMP_CFG_NA4;
            break;
        case NAPOT:
            val|=PMP_CFG_NAPOT;
            break;
    }
    *word = val;
}

/**
 * Returns pmp(region)cfg 8 bits corresponding to a region
 */
static uint8_t read_pmp_csr_value(uint8_t region){
    uint32_t reg = (uint32_t) (region /4);
    uint32_t val_read = read_pmpcfg(reg);

    uint32_t mask = (0xFF) << ((region%4) * 8);
    val_read = val_read & mask;
    val_read = val_read >> ((region%4) * 8); 
    return (uint8_t) val_read;
}

/**
 * Writes to pmp address register (pmpaddr(region) = value)
 */

static void write_pmpaddr_csr_value(uint8_t region, uint32_t address){
    switch (region) {
    case 0:
        __asm__("csrw pmpaddr0, %[addr]" ::[addr] "r"(address) :);
        break;
    case 1:
        __asm__("csrw pmpaddr1, %[addr]" ::[addr] "r"(address) :);
        break;
    case 2:
        __asm__("csrw pmpaddr2, %[addr]" ::[addr] "r"(address) :);
        break;
    case 3:
        __asm__("csrw pmpaddr3, %[addr]" ::[addr] "r"(address) :);
        break;
    case 4:
        __asm__("csrw pmpaddr4, %[addr]" ::[addr] "r"(address) :);
        break;
    case 5:
        __asm__("csrw pmpaddr5, %[addr]" ::[addr] "r"(address) :);
        break;
    case 6:
        __asm__("csrw pmpaddr6, %[addr]" ::[addr] "r"(address) :);
        break;
    case 7:
        __asm__("csrw pmpaddr7, %[addr]" ::[addr] "r"(address) :);
        break;
    case 8:
        __asm__("csrw pmpaddr8, %[addr]" ::[addr] "r"(address) :);
        break;
    case 9:
        __asm__("csrw pmpaddr9, %[addr]" ::[addr] "r"(address) :);
        break;
    case 10:
        __asm__("csrw pmpaddr10, %[addr]" ::[addr] "r"(address) :);
        break;
    case 11:
        __asm__("csrw pmpaddr11, %[addr]" ::[addr] "r"(address) :);
        break;
    case 12:
        __asm__("csrw pmpaddr12, %[addr]" ::[addr] "r"(address) :);
        break;
    case 13:
        __asm__("csrw pmpaddr13, %[addr]" ::[addr] "r"(address) :);
        break;
    case 14:
        __asm__("csrw pmpaddr14, %[addr]" ::[addr] "r"(address) :);
        break;
    case 15:
        __asm__("csrw pmpaddr15, %[addr]" ::[addr] "r"(address) :);
        break;
    case 16:
        __asm__("csrw pmpaddr16, %[addr]" ::[addr] "r"(address) :);
        break;
    #if PMP_REGIONS == 64
    case 17:
        __asm__("csrw pmpaddr17, %[addr]" ::[addr] "r"(address) :);
        break;
    case 18:
        __asm__("csrw pmpaddr18, %[addr]" ::[addr] "r"(address) :);
        break;
    case 19:
        __asm__("csrw pmpaddr19, %[addr]" ::[addr] "r"(address) :);
        break;
    case 20:
        __asm__("csrw pmpaddr20, %[addr]" ::[addr] "r"(address) :);
        break;
    case 21:
        __asm__("csrw pmpaddr21, %[addr]" ::[addr] "r"(address) :);
        break;
    case 22:
        __asm__("csrw pmpaddr22, %[addr]" ::[addr] "r"(address) :);
        break;
    case 23:
        __asm__("csrw pmpaddr23, %[addr]" ::[addr] "r"(address) :);
        break;
    case 24:
        __asm__("csrw pmpaddr24, %[addr]" ::[addr] "r"(address) :);
        break;
    case 25:
        __asm__("csrw pmpaddr25, %[addr]" ::[addr] "r"(address) :);
        break;
    case 26:
        __asm__("csrw pmpaddr26, %[addr]" ::[addr] "r"(address) :);
        break;
    case 27:
        __asm__("csrw pmpaddr27, %[addr]" ::[addr] "r"(address) :);
        break;
    case 28:
        __asm__("csrw pmpaddr28, %[addr]" ::[addr] "r"(address) :);
        break;
    case 29:
        __asm__("csrw pmpaddr29, %[addr]" ::[addr] "r"(address) :);
        break;
    case 30:
        __asm__("csrw pmpaddr30, %[addr]" ::[addr] "r"(address) :);
        break;
    case 31:
        __asm__("csrw pmpaddr31, %[addr]" ::[addr] "r"(address) :);
        break;
    case 32:
        __asm__("csrw pmpaddr32, %[addr]" ::[addr] "r"(address) :);
        break;
    case 33:
        __asm__("csrw pmpaddr33, %[addr]" ::[addr] "r"(address) :);
        break;
    case 34:
        __asm__("csrw pmpaddr34, %[addr]" ::[addr] "r"(address) :);
        break;
    case 35:
        __asm__("csrw pmpaddr35, %[addr]" ::[addr] "r"(address) :);
        break;
    case 36:
        __asm__("csrw pmpaddr36, %[addr]" ::[addr] "r"(address) :);
        break;
    case 37:
        __asm__("csrw pmpaddr37, %[addr]" ::[addr] "r"(address) :);
        break;
    case 38:
        __asm__("csrw pmpaddr38, %[addr]" ::[addr] "r"(address) :);
        break;
    case 39:
        __asm__("csrw pmpaddr39, %[addr]" ::[addr] "r"(address) :);
        break;
    case 40:
        __asm__("csrw pmpaddr40, %[addr]" ::[addr] "r"(address) :);
        break;
    case 41:
        __asm__("csrw pmpaddr41, %[addr]" ::[addr] "r"(address) :);
        break;
    case 42:
        __asm__("csrw pmpaddr42, %[addr]" ::[addr] "r"(address) :);
        break;
    case 43:
        __asm__("csrw pmpaddr43, %[addr]" ::[addr] "r"(address) :);
        break;
    case 44:
        __asm__("csrw pmpaddr44, %[addr]" ::[addr] "r"(address) :);
        break;
    case 45:
        __asm__("csrw pmpaddr45, %[addr]" ::[addr] "r"(address) :);
        break;
    case 46:
        __asm__("csrw pmpaddr46, %[addr]" ::[addr] "r"(address) :);
        break;
    case 47:
        __asm__("csrw pmpaddr47, %[addr]" ::[addr] "r"(address) :);
        break;
    case 48:
        __asm__("csrw pmpaddr48, %[addr]" ::[addr] "r"(address) :);
        break;
    case 49:
        __asm__("csrw pmpaddr49, %[addr]" ::[addr] "r"(address) :);
        break;
    case 50:
        __asm__("csrw pmpaddr50, %[addr]" ::[addr] "r"(address) :);
        break;
    case 51:
        __asm__("csrw pmpaddr51, %[addr]" ::[addr] "r"(address) :);
        break;
    case 52:
        __asm__("csrw pmpaddr52, %[addr]" ::[addr] "r"(address) :);
        break;
    case 53:
        __asm__("csrw pmpaddr53, %[addr]" ::[addr] "r"(address) :);
        break;
    case 54:
        __asm__("csrw pmpaddr54, %[addr]" ::[addr] "r"(address) :);
        break;
    case 55:
        __asm__("csrw pmpaddr55, %[addr]" ::[addr] "r"(address) :);
        break;
    case 56:
        __asm__("csrw pmpaddr56, %[addr]" ::[addr] "r"(address) :);
        break;
    case 57:
        __asm__("csrw pmpaddr57, %[addr]" ::[addr] "r"(address) :);
        break;
    case 58:
        __asm__("csrw pmpaddr58, %[addr]" ::[addr] "r"(address) :);
        break;
    case 59:
        __asm__("csrw pmpaddr59, %[addr]" ::[addr] "r"(address) :);
        break;
    case 60:
        __asm__("csrw pmpaddr60, %[addr]" ::[addr] "r"(address) :);
        break;
    case 61:
        __asm__("csrw pmpaddr61, %[addr]" ::[addr] "r"(address) :);
        break;
    case 62:
        __asm__("csrw pmpaddr62, %[addr]" ::[addr] "r"(address) :);
        break;
    case 63:
        __asm__("csrw pmpaddr63, %[addr]" ::[addr] "r"(address) :);
        break;
    #endif
    default:
        break;
    }

}

/**
 * reads and returns pmpaddr(region) csr value
 */

uint32_t read_pmpaddr_csr_value(uint8_t region){
    uint32_t address;

    switch (region) {
    case 0:
        __asm__("csrr %[addr], pmpaddr0" : [addr] "=r"(address)::);
        break;
    case 1:
        __asm__("csrr %[addr], pmpaddr1" : [addr] "=r"(address)::);
        break;
    case 2:
        __asm__("csrr %[addr], pmpaddr2" : [addr] "=r"(address)::);
        break;
    case 3:
        __asm__("csrr %[addr], pmpaddr3" : [addr] "=r"(address)::);
        break;
    case 4:
        __asm__("csrr %[addr], pmpaddr4" : [addr] "=r"(address)::);
        break;
    case 5:
        __asm__("csrr %[addr], pmpaddr5" : [addr] "=r"(address)::);
        break;
    case 6:
        __asm__("csrr %[addr], pmpaddr6" : [addr] "=r"(address)::);
        break;
    case 7:
        __asm__("csrr %[addr], pmpaddr7" : [addr] "=r"(address)::);
        break;
    case 8:
        __asm__("csrr %[addr], pmpaddr8" : [addr] "=r"(address)::);
        break;
    case 9:
        __asm__("csrr %[addr], pmpaddr9" : [addr] "=r"(address)::);
        break;
    case 10:
        __asm__("csrr %[addr], pmpaddr10" : [addr] "=r"(address)::);
        break;
    case 11:
        __asm__("csrr %[addr], pmpaddr11" : [addr] "=r"(address)::);
        break;
    case 12:
        __asm__("csrr %[addr], pmpaddr12" : [addr] "=r"(address)::);
        break;
    case 13:
        __asm__("csrr %[addr], pmpaddr13" : [addr] "=r"(address)::);
        break;
    case 14:
        __asm__("csrr %[addr], pmpaddr14" : [addr] "=r"(address)::);
        break;
    case 15:
        __asm__("csrr %[addr], pmpaddr15" : [addr] "=r"(address)::);
        break;
    #if PMP_REGIONS == 64
    case 16:
        __asm__("csrr %[addr], pmpaddr16" : [addr] "=r"(address)::);
        break;
    case 17:
        __asm__("csrr %[addr], pmpaddr17" : [addr] "=r"(address)::);
        break;
    case 18:
        __asm__("csrr %[addr], pmpaddr18" : [addr] "=r"(address)::);
        break;
    case 19:
        __asm__("csrr %[addr], pmpaddr19" : [addr] "=r"(address)::);
        break;
    case 20:
        __asm__("csrr %[addr], pmpaddr20" : [addr] "=r"(address)::);
        break;
    case 21:
        __asm__("csrr %[addr], pmpaddr21" : [addr] "=r"(address)::);
        break;
    case 22:
        __asm__("csrr %[addr], pmpaddr22" : [addr] "=r"(address)::);
        break;
    case 23:
        __asm__("csrr %[addr], pmpaddr23" : [addr] "=r"(address)::);
        break;
    case 24:
        __asm__("csrr %[addr], pmpaddr24" : [addr] "=r"(address)::);
        break;
    case 25:
        __asm__("csrr %[addr], pmpaddr25" : [addr] "=r"(address)::);
        break;
    case 26:
        __asm__("csrr %[addr], pmpaddr26" : [addr] "=r"(address)::);
        break;
    case 27:
        __asm__("csrr %[addr], pmpaddr27" : [addr] "=r"(address)::);
        break;
    case 28:
        __asm__("csrr %[addr], pmpaddr28" : [addr] "=r"(address)::);
        break;
    case 29:
        __asm__("csrr %[addr], pmpaddr29" : [addr] "=r"(address)::);
        break;
    case 30:
        __asm__("csrr %[addr], pmpaddr30" : [addr] "=r"(address)::);
        break;
    case 31:
        __asm__("csrr %[addr], pmpaddr31" : [addr] "=r"(address)::);
        break;
    case 32:
        __asm__("csrr %[addr], pmpaddr32" : [addr] "=r"(address)::);
        break;
    case 33:
        __asm__("csrr %[addr], pmpaddr33" : [addr] "=r"(address)::);
        break;
    case 34:
        __asm__("csrr %[addr], pmpaddr34" : [addr] "=r"(address)::);
        break;
    case 35:
        __asm__("csrr %[addr], pmpaddr35" : [addr] "=r"(address)::);
        break;
    case 36:
        __asm__("csrr %[addr], pmpaddr36" : [addr] "=r"(address)::);
        break;
    case 37:
        __asm__("csrr %[addr], pmpaddr37" : [addr] "=r"(address)::);
        break;
    case 38:
        __asm__("csrr %[addr], pmpaddr38" : [addr] "=r"(address)::);
        break;
    case 39:
        __asm__("csrr %[addr], pmpaddr39" : [addr] "=r"(address)::);
        break;
    case 40:
        __asm__("csrr %[addr], pmpaddr40" : [addr] "=r"(address)::);
        break;
    case 41:
        __asm__("csrr %[addr], pmpaddr41" : [addr] "=r"(address)::);
        break;
    case 42:
        __asm__("csrr %[addr], pmpaddr42" : [addr] "=r"(address)::);
        break;
    case 43:
        __asm__("csrr %[addr], pmpaddr43" : [addr] "=r"(address)::);
        break;
    case 44:
        __asm__("csrr %[addr], pmpaddr44" : [addr] "=r"(address)::);
        break;
    case 45:
        __asm__("csrr %[addr], pmpaddr45" : [addr] "=r"(address)::);
        break;
    case 46:
        __asm__("csrr %[addr], pmpaddr46" : [addr] "=r"(address)::);
        break;
    case 47:
        __asm__("csrr %[addr], pmpaddr47" : [addr] "=r"(address)::);
        break;
    case 48:
        __asm__("csrr %[addr], pmpaddr48" : [addr] "=r"(address)::);
        break;
    case 49:
        __asm__("csrr %[addr], pmpaddr49" : [addr] "=r"(address)::);
        break;
    case 50:
        __asm__("csrr %[addr], pmpaddr50" : [addr] "=r"(address)::);
        break;
    case 51:
        __asm__("csrr %[addr], pmpaddr51" : [addr] "=r"(address)::);
        break;
    case 52:
        __asm__("csrr %[addr], pmpaddr52" : [addr] "=r"(address)::);
        break;
    case 53:
        __asm__("csrr %[addr], pmpaddr53" : [addr] "=r"(address)::);
        break;
    case 54:
        __asm__("csrr %[addr], pmpaddr54" : [addr] "=r"(address)::);
        break;
    case 55:
        __asm__("csrr %[addr], pmpaddr55" : [addr] "=r"(address)::);
        break;
    case 56:
        __asm__("csrr %[addr], pmpaddr56" : [addr] "=r"(address)::);
        break;
    case 57:
        __asm__("csrr %[addr], pmpaddr57" : [addr] "=r"(address)::);
        break;
    case 58:
        __asm__("csrr %[addr], pmpaddr58" : [addr] "=r"(address)::);
        break;
    case 59:
        __asm__("csrr %[addr], pmpaddr59" : [addr] "=r"(address)::);
        break;
    case 60:
        __asm__("csrr %[addr], pmpaddr60" : [addr] "=r"(address)::);
        break;
    case 61:
        __asm__("csrr %[addr], pmpaddr61" : [addr] "=r"(address)::);
        break;
    case 62:
        __asm__("csrr %[addr], pmpaddr62" : [addr] "=r"(address)::);
        break;
    case 63:
        __asm__("csrr %[addr], pmpaddr62" : [addr] "=r"(address)::);
        break;
    #endif
    default:
        break;
    }

    return address;
}


//API implementation

int get_granularity(){
    return (1 << (PMP_GRANULARITY + 2));
}

int setRLB(unsigned int enable){
    uint32_t v;
    if(enable){
        __asm__ volatile("csrsi mseccfg, %0"::"i"(RLB));
        __asm__ volatile("csrr %0, mseccfg":"=r"(v));
        if(v&RLB!=RLB){
            //Couldn't set RLB. there are locked rules
            return 1;
        }
    }else{
        __asm__ volatile("csrci mseccfg, %0"::"i"(RLB));
    }
    return 0;
}

void setMML(){
    __asm__ volatile ("csrsi mseccfg, %0"::"i"(MML));
}
   
void setMMWP(){
     __asm__ volatile ("csrsi mseccfg, %0"::"i"(MMWP));
}

int remove_region(uint8_t region){

    //TODO: INCLUDE SL CHECK
    pmp_config_t cfg;
    pmp_read_region(region,&cfg);
    uint32_t L = cfg.L;
    uint32_t v;
    __asm__ volatile("csrr %0, mseccfg":"=r"(v));
    v = v & RLB;
    if(L && v!=RLB){
        //Can't remove locked region (without RLB) %d
        return PMP_LOCKED;
    }
    
    // if PMP_NUM_REGIONS > 0, all pmpcfg register form a collective WARL field with 0x00
    // so in order to remove the region we need to write a different number
    pmp_write_cfg(region, PMP_CFG_OFF | PMP_CFG_R);

    return PMP_OK;
}

int pmp_configure_region(pmp_config_t* config){

    unsigned int reg_num = config->region_number;
    unsigned int region_dimension = (config->A==TOR)? (config->region_end_address)-(config->region_start_address):config->region_dimension;

    //perform all the checks.
    if(config->region_dimension==0 && config->A==NAPOT){
       // region dimension must be specified in NAPOT mode
        return PMP_MISSING;
    }
    if(config->region_end_address==0 && config->A==TOR){
        // start and end address must be specified in TOR mode
        return PMP_MISSING;
    }
    if(config->A==NAPOT && (config->region_start_address%config->region_dimension)){
        // NAPOT region must be aligned 
        return PMP_NOT_VALID;
    }
    if (reg_num<0 || reg_num>PMP_REGIONS){
       //region number is not valid\n
        return PMP_NOT_VALID;
    }
    if(reg_num==0 && config->A==TOR && config->region_start_address!=0){
        //first region always starts from 0 in TOR mode.
        return PMP_NOT_VALID;
    }
    if((config->A==NA4) && (config->region_dimension!=0 || config->region_dimension!=4)){
        //in mode NA4 range is 4
        return PMP_NOT_VALID;
    }else if (region_dimension < get_granularity()){
        //region dimension is less then granularity
        return PMP_NOT_VALID;
    }

    //check if rule is locked. if so, can't be modified unless RLB is 1
    //locked if L=1 or if SL=1 and mode is Non Secure
    pmp_config_t old_cfg;
    pmp_read_region(reg_num, &old_cfg);
    //read RLB
    uint32_t v;
    __asm__ volatile("csrr %0, mseccfg":"=r"(v));
    v = v & RLB;
    if( (old_cfg.L==1 ||(old_cfg.SL==1 && get_security_state() == 0)) && v!=RLB){
        //region is locked
        return PMP_LOCKED;
    }
   
    //if MML is not set, RW=01 form a warl field, so the write will fail
    __asm__ volatile("csrr %0, mseccfg":"=r"(v));
    v=v & MML;
    if(config->R==0 && config->W==1 && v!=MML){
        //RW=01 is not vaild if MML is not set
        return PMP_NOT_VALID;
    }

    //if TOR mode, configuration is not possible if previous region is locked and start address is different
    

    uint32_t start_address, end_address;
    start_address = (config->region_start_address) >> 2;
    //set the address
    switch(config->A){
        case TOR:
            //range from previous region to current
            end_address = (config->region_end_address) >> 2;
            if(reg_num!=0) write_pmpaddr_csr_value(reg_num-1,start_address);
            if(read_pmpaddr_csr_value(reg_num-1)!=start_address){
                // if couldn't write it means previous region is locked
                return PMP_LOCKED;
            }
            write_pmpaddr_csr_value(reg_num,end_address);
            break;
        case NA4:
            write_pmpaddr_csr_value(reg_num,start_address);
            break;
        case NAPOT:
            //the address has to be modified.
            //set the 0 in the right position
            start_address &= ~(config->region_dimension >> 3);
            
            //to set less significant bits to 1
            start_address |= ((config->region_dimension >> 3) - 1);
            
            //write correct address
            write_pmpaddr_csr_value(reg_num,start_address);
            break;
        case OFF:
            //off doesnt do anything, just write in the control word.
            break;
    }

    //control word has to be written after writing the address for consistency
    uint32_t word;
    pmp_cfg_to_csr(config,&word);
    pmp_write_cfg(reg_num,word);
    return PMP_OK;
}

void pmp_read_region (uint8_t region, pmp_config_t* config){
    pmp_config_t cfg;

    uint8_t word = read_pmp_csr_value(region);
    cfg.X=((word & PMP_CFG_X )== PMP_CFG_X)? 1:0;
    cfg.L=((word & PMP_CFG_L) == PMP_CFG_L)? 1:0;
    cfg.R=((word & PMP_CFG_R) == PMP_CFG_R)? 1:0;
    cfg.W=((word & PMP_CFG_W) == PMP_CFG_W)? 1:0;
    cfg.SL=((word & PMP_CFG_SL) == PMP_CFG_SL)? 1:0;
    
    uint32_t address;
    switch((word & 0x18)){
        case PMP_CFG_OFF:
            cfg.A = OFF;
            cfg.region_start_address=0;
            cfg.region_end_address=0;
            break;
        case PMP_CFG_NA4:
            cfg.A = NA4;
            address=read_pmpaddr_csr_value(region);
            address = address<<2;
            cfg.region_start_address = address;
            cfg.region_end_address=address+4;
            break;
        case PMP_CFG_NAPOT:
            cfg.A = NAPOT;
            cfg.region_start_address=read_pmpaddr_csr_value(region);
            break;
        case PMP_CFG_TOR:
            cfg.A = TOR;
            uint32_t start_address;
            start_address = read_pmpaddr_csr_value(region-1);
            start_address = start_address<<2;
            address = read_pmpaddr_csr_value(region);
            address = address<<2;
            cfg.region_start_address = start_address;
            cfg.region_end_address=address;
            break;
        default:
            break;
    }
    
    *config = cfg;

}

