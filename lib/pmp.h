#include <stdint.h>
#include "system_csr.h"

#ifndef PMP_H_
#define PMP_H_
/**
 * @enum PMP_MODE_t
 * @brief the 4 different modes for address matching
 */
typedef enum 
{   OFF = 0,
    TOR = 1,
    NA4 = 2,
    NAPOT = 3
}PMP_MODE_t;

/**
 * Granularity of regions (minimum dimension is 2^(G+2) bytes)
 */
#define PMP_GRANULARITY 0

/**
 * number of regions for this implementation.
 */
#define PMP_REGIONS 16

/** 
 * @struct pmp_config_t
 * @brief this struct contains the configuration for a PMP region
 * @param region_number the number
 * @param R read permission
 * @param W write permission
 * @param X execute permission
 * @param L locked rule
 * @param A address matching mode
 * @param region_start_address start address; can me omitted if A=TOR and region_number=0, because it defaults to 0
 * @param region_end_address end address; to be specified only in TOR mode
 * @param region_dimension dimension; to be specified in NA4 or NAPOT mode, must be larger than granularity
 * */
typedef struct  {
    unsigned int region_number;
    unsigned int R : 1;
    unsigned int W : 1;
    unsigned int X : 1;
    unsigned int L: 1;
    PMP_MODE_t A : 2;
    uint32_t region_start_address;
    uint32_t region_end_address;
    unsigned int region_dimension;
} pmp_config_t;

/**
    bits to set in control word
**/
#define PMP_CFG_L 0x80
#define PMP_CFG_R 0x01
#define PMP_CFG_W 0x02
#define PMP_CFG_X 0x04
#define PMP_CFG_OFF 0x00
#define PMP_CFG_TOR 0x08
#define PMP_CFG_NA4 0x10
#define PMP_CFG_NAPOT 0x18

/**
 * bits in mseccfg
 */
#define MML 0x1
#define MMWP 0x2
#define RLB 0x4

/**
 * @brief derives minimum region dimension
 **/
int get_granularity();

/**
 * @brief setting MML bit enables Machine Mode Lockdown
 **/
void setMML();

/**
 * @brief setting MMWP bit enables Machine Mode Whitelist Policy
 **/
void setMMWP();

/** 
 * @brief Rule Locking Bypass. to set during boot process or debug
 * (will not be set if any rules with L=1 are present)
 * @param enable    enable or disable Rule Locking Bypass*/
void setRLB(unsigned int enable);

/**
 *  @brief Used to remove a region. returns 0 if success, -1 otherwise
 * @details sets mode to OFF
 */
int remove_region(uint8_t region);

/** 
 * @brief rreads pmpcfgx (contains pmpycfg where ceil(y/4)=x)
 * @param region    the number x
*/
uint32_t read_pmpcfg(uint8_t number);

/** 
 * @brief reads pmpxcfg from pmpcfgy register; returns 8 bit of configuration
 * @param region    the number of the region to read.
*/
uint8_t read_pmp_csr_value(uint8_t region);

/** 
 * @brief writes to pmp configuration register
 * @param region    the number of the region to write.
 * @param value     the value to write.
*/
void write_pmp_csr_value(uint8_t region, uint32_t value);

/** 
 * @brief writes to pmp address register
 * @param region    the number of the region to write.
 * @param address     the value to write.
*/
void write_pmpaddr_csr_value(uint8_t region, uint32_t address);

/** 
 * @brief reads pmpaddr csr value. returns the read value
 * @param region    the number of the region to read.
*/
uint32_t read_pmpaddr_csr_value(uint8_t region);

/**
 * @brief derives control word from configuration struct
 * @param config    config struct
 * @param word      pointer to store the control word
 */
void pmp_cfg_to_csr(pmp_config_t * config, uint32_t* word);


/**
 * @brief writes config value to register
 * @details takes 8 bits of configuration, writes the correct 8 bits in 32-bit register
 * @param region    number of region to write
 * @param word      control word
 */
void pmp_write_cfg(uint8_t region, uint8_t toWrite);

/**
 * @brief configures the region, may be called during inizialization. returns 0 if success, -1 otherwise
 * @param config    the region configuration
 */
int pmp_configure_region(pmp_config_t* config);

/**
 * @brief reads region configuration
 * @param region    number of region
 */
pmp_config_t pmp_read_region (uint8_t region);

#endif