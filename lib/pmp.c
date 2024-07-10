#include "pmp.h"
#include "system.h"
#include <stdio.h>
#include <math.h>

int get_granularity(){
    return (1 << (PMP_GRANULARITY + 2));
}

void setRLB(unsigned int enable){
    uint32_t v;
    if(enable){
        __asm__ volatile("csrsi mseccfg, %0"::"i"(RLB));
        __asm__ volatile("csrr %0, mseccfg":"=r"(v));
        if(v&RLB!=RLB){
            printf("Couldn't set RLB. There are locked rules\n\n");
        }
    }else{
        __asm__ volatile("csrci mseccfg, %0"::"i"(RLB));
    }
}

void setMML(){
    __asm__ volatile ("csrsi mseccfg, %0"::"i"(MML));
}
   
void setMMWP(){
     __asm__ volatile ("csrsi mseccfg, %0"::"i"(MMWP));
}

int remove_region(uint8_t region){

    uint32_t L = pmp_read_region(region).L;
    uint32_t v;
    __asm__ volatile("csrr %0, mseccfg":"=r"(v));
    v = v & RLB;
    if(L && v!=RLB){
        printf("Can't remove locked region (without RLB) %d",region);
    }
    
    // if PMP_NUM_REGIONS > 0, all pmpcfg register form a collective WARL field with 0x00
    // so in order to remove the region we need to write a different number
    pmp_write_cfg(region, PMP_CFG_OFF | PMP_CFG_R);

    printf("Correctly removed region %x\n",region);
}

void pmp_cfg_to_csr(pmp_config_t * config, uint32_t* word){
    
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

uint32_t read_pmpcfg(uint8_t number){
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
    }
    //extend in case of more regions
    return val_read;
}

uint8_t read_pmp_csr_value(uint8_t region){
    uint32_t reg = (uint32_t) (region /4);
    uint32_t val_read = read_pmpcfg(reg);

    uint32_t mask = (0xFF) << ((region%4) * 8);
    val_read = val_read & mask;
    val_read = val_read >> ((region%4) * 8); 
    return (uint8_t) val_read;
}

void write_pmp_csr_value(uint8_t region, uint32_t value){
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
        //extend in case of more regions.
    } 
}

void pmp_write_cfg(uint8_t region, uint8_t cfg){

     //TODO:This should be done atomically
    //For now we assume that pmp configuration is done in a single intialization thread
    uint32_t current_value = read_pmpcfg((int)(region/4));

    //mask for the correct 8 bits
    uint32_t mask = 0xFF << ((region%4) * 8);
    uint32_t toWrite = (uint32_t)cfg << ((region%4) * 8);

    //write correct value
    uint32_t next_value = (current_value & ~mask) | toWrite;
    write_pmp_csr_value (region,next_value);
}


void write_pmpaddr_csr_value(uint8_t region, uint32_t address){
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
        }
}

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
    default:
        break;
    }
    return address;
}

pmp_config_t pmp_read_region (uint8_t region){
    pmp_config_t cfg;

    uint8_t word = read_pmp_csr_value(region);
    cfg.X=(word & PMP_CFG_X == PMP_CFG_X)? 1:0;
    cfg.L=(word & PMP_CFG_L == PMP_CFG_L)? 1:0;
    cfg.R=(word & PMP_CFG_R == PMP_CFG_R)? 1:0;
    cfg.W=(word & PMP_CFG_W == PMP_CFG_W)? 1:0;
    cfg.SL=(word & PMP_CFG_SL == PMP_CFG_SL)? 1:0;
    
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
    
    return cfg;

}

int pmp_configure_region(pmp_config_t* config){

    unsigned int reg_num = config->region_number;
    unsigned int region_dimension = (config->A==TOR)? (config->region_end_address)-(config->region_start_address):config->region_dimension;

    //perform all the checks.
    if(config->region_dimension==0 && config->A==NAPOT){
        printf("[PMP configuration: region %d] region dimension must be specified in NAPOT mode\n",config->region_number);
        return -1;
    }
    if(config->region_end_address==0 && config->A==TOR){
        printf("[PMP configuration: region %d] start and end address must be specified in TOR mode\n",config->region_number);
        return -1;
    }
    if(config->A==NAPOT && (config->region_start_address%config->region_dimension)){
        printf("[PMP configuration: region %d] NAPOT region must be aligned\n");
        return -1;
    }
    if (reg_num<0 || reg_num>PMP_REGIONS){
        printf("[PMP configuration: region %d] region number is not valid\n",reg_num);
        return -1;
    }
    if(reg_num==0 && config->A==TOR && config->region_start_address!=0){
        printf("[PMP configuration: region %d] first region always starts from 0 in TOR mode. Start address will be ignored\n", reg_num);
    }
    if((config->A==NA4) && (config->region_dimension!=0 || config->region_dimension!=4)){
        printf("[PMP configuration: region %d] range is 4 otherwise will be ignored\n",reg_num);
    }else if (region_dimension < get_granularity()){
        printf("[PMP configuration: region %d] region dimension (%d bytes) is not valid (less then granularity (%d))\n",reg_num, config->region_dimension, get_granularity());
        return -1;
    }

    //check if rule is locked. if so, can't be modified unless RLB is 1
    pmp_config_t old_cfg = pmp_read_region(reg_num);
    //read RLB
    uint32_t v;
    __asm__ volatile("csrr %0, mseccfg":"=r"(v));
    v = v & RLB;
    if(old_cfg.L==1 && v!=RLB){
        printf("[PMP configuration: region %d] region is locked, can't be modified\n",reg_num);  
        return -1;
    }

    //if MML is not set, RW=01 form a warl field, so the write will fail
    __asm__ volatile("csrr %0, mseccfg":"=r"(v));
    v=v & MML;
    if(config->R==0 && config->W==1 && v!=MML){
        printf("[PMP configuration: region %d] RW=01 is not vaild if MML is not set.\n",reg_num);  
        return -1;
    }

    uint32_t start_address, end_address;
    start_address = (config->region_start_address) >> 2;
    //set the address
    switch(config->A){
        case TOR:
            //range from previous region to current
            end_address = (config->region_end_address) >> 2;
            if(reg_num!=0) write_pmpaddr_csr_value(reg_num-1,start_address);
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
    printf("Region %d correctly configured\n",reg_num);
    return 0;
}