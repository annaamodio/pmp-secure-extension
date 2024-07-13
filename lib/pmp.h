//PMP APIs
#ifndef PMP_H_
#define PMP_H_

#include <stdint.h>

/**
 * Granularity of regions (minimum dimension is 2^(G+2) bytes)
 */
#define PMP_GRANULARITY 0

/**
 * number of regions for this implementation.
 */
#define PMP_REGIONS 16


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
 * error codes
 */
#define PMP_OK       0
#define PMP_LOCKED 1  /* when you try to modify a locked rule (or SLocked)*/
#define PMP_MISSING 2 /* missing parameters*/
#define PMP_NOT_VALID 3 /*invalid parameters*/

/** 
 * @struct pmp_config_t
 * @brief this struct contains the configuration for a PMP region
 * @param region_number the number
 * @param SL security lock 
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
    unsigned int SL : 1;
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
#define PMP_CFG_SL 0x40

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
 * @brief Rule Locking Bypass. to set during boot process or debug
 * (will not be set if any rules with L=1 are present)
 * returns PMP_OK if successful, PMP_LOCKED otherwise
 * @param enable    enable or disable Rule Locking Bypass
 **/
int setRLB(unsigned int enable);

/**
 * @brief setting MML bit enables Machine Mode Lockdown
 **/
void setMML();

/**
 * @brief setting MMWP bit enables Machine Mode Whitelist Policy
 **/
void setMMWP();

/**
 *  @brief Used to remove a region. returns 0 if success, -1 otherwise
 * @details sets mode to OFF
 */
int remove_region(uint8_t region);

/**
 * @brief configures the region, may be called during inizialization. returns 0 if success, -1 otherwise
 * @param config    the region configuration
 */
int pmp_configure_region(pmp_config_t* config);

/**
 * @brief reads region configuration
 * @param region    number of region
 * @param config    config struct to save region configuration
 */
void pmp_read_region (uint8_t region, pmp_config_t* config);

#endif