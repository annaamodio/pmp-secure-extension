#include "system.h"
#include "system_csr.h"
#include "pmp.h"


void user(){
	printf("*********************\n\n");
	printf("\nNow in user function\n\n");

	int * address=(int*)0x130004;
	/*
	 * Region 3. User can Read but not write.
	 */
	printf("U mode: trying to read in read only location for U (should succed)\n\n");
	int value = *address;
	printf("U mode: trying to write in read only location for U (should fail)\n\n");
	*address=2024;

	printf("***End of tests*****");
	sim_halt();
}

void switchToUser(){
	printf("Switching to user mode\n\n");
	void (*ptr);
	uint32_t status;
	ptr=user;
	/**
	 * Writing address of user entry point in mepc register
	 * Clearing bits MPP to set user mode
	 * Returning to user mode with mret
	 */
	__asm__ volatile("csrw mepc, %0"::"r"(ptr));
	uint32_t mask = 0x1800;
	__asm__ volatile("csrc mstatus, %0"::"r"(mask));
	__asm__ volatile("mret");

}

int main(int argc, char* argv[]){
	
	system_init(); //modified to register custom exception handler
	
	pmp_config_t a;
	/**
	 * This has to have at least RW permissions to be able to use printf and puts
	 */
	a.region_number=0;
	a.A=TOR;
	a.region_start_address=0;
	a.region_end_address=0x100000;
	a.L=0;
	a.R=1; 
	a.W=1;
	a.X=1;
	if(pmp_configure_region(&a)!=0){
		printf("region not configured");
	}
	
	/**
	 * Code and default data
	 */
	a.region_number=1;
	a.A=TOR;
	a.region_start_address=0x100000;
	a.region_end_address=0x110000;
	a.L=0;
	a.R=1; 
	a.W=1;
	a.X=1;
	if(pmp_configure_region(&a)!=0){
		printf("region not configured");
	}

	/**
	 * Read only, for both M and U
	 */
	a.region_number=2;
	a.A=TOR;
	a.region_start_address=0x110000;
	a.region_end_address=0x130000;
	a.L=1;
	a.R=1;
	a.W=0; 
	a.X=0;
	if(pmp_configure_region(&a)!=0){
		printf("region not configured");
	}
	
	/**
	 * Read only for U; Rule ignored for M
	 */
	pmp_config_t cfg2;
	cfg2.region_number=3;
	cfg2.A=NAPOT;
	cfg2.L=0;
	cfg2.R=1;
	cfg2.W=0;
	cfg2.X=0;
	cfg2.region_start_address=0x130000;
	cfg2.region_dimension=32;
	if(pmp_configure_region(&cfg2)!=0){
		printf("region not configured");
	}

	/**
	 * Machine Mode Whitelist Policy: other regions can't be accessed
	 */
	setMMWP();

	/**
	 * Trying to write in Read-Only region for both M and U (2)
	 */
	int* address = (int*)0x120004;
	printf("\nM mode: trying to write in read only location (should fail)\n\n");
	*address = 26;

	/**
	 * Trying to write in region 3; No rules for M
	 */
	address=(int*)0x130004;
	printf("M mode: trying to write in location that is read only for U (should succeed)\n\n");
	*address=23;
	printf("M mode: trying to read from location that is read only for U (should succeed)\n\n");
	int value = *address;

	/**
	 * Trying to access undefined region
	 * Should not work because of Whitelist Policy
	 */

	address=(int *)0x140000;
	printf("M mode: trying to access illegal region (should fail - whitelist)\n\n");
	value =*address;
	
	/*
	* Switching to user mode
	*/
	switchToUser();
	

    return 0;
}
