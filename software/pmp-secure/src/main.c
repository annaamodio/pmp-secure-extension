#include "system.h"
#include "system_csr.h"
#include "pmp.h"
#include "security_mock.h"

//COMMIT ANCHE DI QUESTO

void user(){
	
	printf("USER - non secure mode\n\n");
	int * address = 0x120008;
	int * address2 = 0x125008;
	
	printf("Reading from region 1 (U + non secure) should succeed\n\n");
	int v = *address;
	/* printf("Writing in region 1 (U + non secure) should fail\n\n");
	*address= 625;  */


	/* printf("Reading from region 2 (U + non secure) should fail\n\n");
	v = *address2; */
	/* printf("Writing in region 2 (U + non secure) should fail\n\n");
	*address2= 625;  */

	SET_SECURE_MODE;
	/*
	to check if security mode was actually changed
	int sec = *(int*)0x40000;
	printf("SEC: %x\n",sec);*/
	printf("USER - secure mode\n\n");
	 printf("Reading from region 1 (U + secure) should succeed\n\n");
	v = *address; /*
	printf("Writing in region 1 (U + secure) should fail\n\n");
	*address= 625;  */


	/* printf("Reading from region 2 (U + secure) should fail \n\n");
	v = *address2; */
	/* printf("Writing in region 2 (U + secure) should fail\n\n");
	*address2= 625;  */
	
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

void nonSecure(){
	printf("In non secure mode (M)\n\n");
	int * address = 0x120008;
	int * address2 = 0x125008;
	
	printf("Reading from region 1 (M + non secure): should succeed\n\n");
	int v = *address;
	/* printf("Writing in region 1 (M + non secure): should fail\n\n");
	*address= 625;  */


	printf("Reading from region 2 (M + non secure): should succeed\n\n");
	v = *address2;
	printf("Writing in region 2 (M + non secure): should succeed\n\n");
	*address2= 625; 

	printf("Attempting to modify S-Locked rule (M + non secure): should fail (error)\n\n");
	pmp_config_t cfg2 = {
		.region_number = 2,
		.region_start_address = 0x125000,
		.region_dimension = 32,
		.A = NAPOT,
		.SL = 1,
		.L = 0,
		.X = 0,
		.W = 0,
		.R = 0,
	};
	
	if(pmp_configure_region(&cfg2)){
		printf("error\n\n");
	}

	switchToUser();
}


int main(){

	system_init();

	int * address = 0x120008;
	int * address2 = 0x125008;

	*(address) = 325;
	//youre in secure mode pmp checks should be ignored

	pmp_config_t cfg0 ={
		.region_number = 0,
		.region_start_address = 0,
		.region_end_address = 0x110000,
		.A = TOR,
		.SL = 1,
		.L = 0,
		.X = 1,
		.W = 1,
		.R = 1,
	};
	if(pmp_configure_region(&cfg0)){
		printf("error\n\n");
	}


	/** region 1:
	 * can't be modified (L=1)
	 * M mode + Secure can do everything
	 * U mode + Secure can't write
	 * M/U mode + Non secure can't write
	 */
	pmp_config_t cfg = {
		.region_number = 1,
		.region_start_address = 0x120000,
		.region_dimension = 32,
		.A = NAPOT,
		.SL = 1,
		.L = 1,
		.X = 1,
		.W = 0,
		.R = 1,
	};

	if(pmp_configure_region(&cfg)){
		printf("error\n\n");
	}
	
	
	/** region 2:
	 * can be modified only in secure mode (L=0, SL=1)
	 * M mode + Secure can do everything
	 * M + Non secure can do everything (L=0)
	 * U mode can't write or read
	 */
	pmp_config_t cfg2 = {
		.region_number = 2,
		.region_start_address = 0x125000,
		.region_dimension = 32,
		.A = NAPOT,
		.SL = 1,
		.L = 0,
		.X = 1,
		.W = 0,
		.R = 0,
	};

	if(pmp_configure_region(&cfg2)){
		printf("error\n\n");
	}

	printf("Reading from region 1 (M + secure) : should succeed\n\n");
	int v = *address;
	printf("Writing in region 1 (M + secure): should succeed\n\n");
	*address= 625; 


	printf("Reading from region 2 (M + secure): should succeed\n\n");
	v = *address2;
	printf("Writing in region 2 (M + secure): should succeed\n\n");
	*address2= 625; 

	SMRET_mock();

	sim_halt();

	return 0;
}