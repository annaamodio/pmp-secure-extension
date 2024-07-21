#include "system.h"
#include "system_csr.h"
#include "pmp.h"

extern uint32_t _vectors_start;
extern uint32_t _text_start, _mcode_start;
extern uint32_t _text_end, _mcode_end;
extern uint32_t _data_start, _rodata_start, _usertext_start;
extern uint32_t _data_end, _rodata_end, _usertext_end;
extern uint32_t _stack_start, _bss_end, _bss_start;
uint32_t user_area, m_area;

#define SHARED_DATA_DIMENSION 32
#define USER_DATA_DIMENSION 32
#define M_DATA_DIMENSION 32

void shared_function(){
	printf("This function can be called by M and U\n\n");
}

void USER_FUNC u_function(){
	printf("This function can only be called by U\n\n");
}

void M_FUNC m_function(){
	printf("This function can only be called by M\n\n");
} 

void M_FUNC Mtests(){
	
	static int count = 0;
	int * address;
	int value;
	while(1){
	if(count < 7) printf("[%d] ",count);
	switch (count)
	{
	case 0:
		count++;
		address = m_area +14;
		printf("M mode: write to M region (should succeed)\n\n");
		*address = 33;
		break;
	case 1:
		count++;
		address = m_area +14;
		printf("M mode: read from M region (should succeed)\n\n");
		value = *address;
		break;
	case 2:
		count++;
		address = user_area + 14;
		printf("M mode: write to U region (should fail)\n");
		*address = 34;
		break;
	case 3:
		count++;
		address = user_area +14;
		printf("M mode: read from U region (should fail)\n");
		value = *address;
		break;
	case 4:
		count++;
		printf("M mode: calling shared function (should succeed)\n\n");
		shared_function();
		break;
	case 5:
		count++;
		printf("M mode: calling M function (should succeed)\n\n");
		m_function(); 
		break;
	case 6:
		count++;
		printf("M mode: calling U function (should fail)\n");
		u_function(); 
		break;
	default:
		/* M tests are over. Switch to user mode*/
		switchToUser();
		return;
	}
	}
}

void USER_FUNC Utests(){
	static int count = 0;
	int * address = user_area + 14;
	int value;
	while(1){
	if(count < 7) printf("[%d] ",count);
	switch (count)
	{
		case 0:
			count++;
			printf("U mode: write to U region (should succeed)\n\n");
			*address = 34;
			break;
		case 1:
			count++;
			printf("U mode: read from U region (should succeed)\n\n");
			value = *address;
			break;
		case 2:
			count++;
			address = m_area + 14;
			printf("U mode: write to M region (should fail)\n");
			*address = 33;
			break;
		case 3:
			count++;
			address = m_area + 14;
			printf("U mode: read from M region (should fail)\n");
			value = *address;
			break;
		case 4:
			count++;
			printf("U mode: calling shared function (should succeed)\n\n");
  			shared_function();
			break;
		case 5:
			count++;
			printf("U mode: calling M function (should fail)\n");
			m_function();
			break; 
		case 6:
			count++;
			printf("U mode: calling U function (should succeed)\n\n");
			u_function(); 
			break; 
		default:
			/*stop the simulation*/
			printf("****End of tests****\n");
			sim_halt();
			break;
	}
	}
	
}


void USER_FUNC user()
{
	printf("*********************\n\n");
	printf("\nNow in user function\n\n");

	/*call user tests*/
	Utests();
}

void M_FUNC switchToUser()
{
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


void info(){
	printf("vectors: %p\n", &_vectors_start);

	printf("Text start: %p\n", &_text_start);
	printf("Text end: %p\n", &_text_end);

	printf("M start: %p\n", &_mcode_start);
	printf("M end: %p\n", &_mcode_end);

	printf("user start %p\n", &_usertext_start);
	printf("user end %p\n", &_usertext_end);

	printf("Ro start %p\n", &_rodata_start);
	printf("Ro end %p\n", &_rodata_end);

	printf("Data start: %p\n", &_data_start);
	printf("Data end: %p\n", &_data_end);
	printf("Bss start: %p\n ",&_bss_start);
	printf("Bss end: %p\n ",&_bss_end);
	printf("Stack start: %p\n", &_stack_start);
	
}

int M_FUNC main(int argc, char *argv[])
{

	system_init(); // modified to register custom exception handler

	//info();

	setRLB(1); /* enable Rule Locking Bypass*/

	/**
	 * This is M code, including this main function. RX for M, denied for U
	 * */
	pmp_config_t cfg;
	cfg.region_number = 2;
	cfg.region_start_address = (uint32_t)&_mcode_start;
	cfg.A = TOR;
	cfg.region_end_address = (uint32_t)&_mcode_end;
	cfg.L = 1;
	cfg.R = 1;
	cfg.W = 0;
	cfg.X = 1;

	if (pmp_configure_region(&cfg) != 0)
	{
		printf("Error configuring region 2\n\n");
	}

	
	/**
	 * This is shared code (sections .text and .vectors) including this main function
	 * Now , RX for M and U; after MML, it will become RX for M and denied for U.
	 * We will change it
	 * */
	pmp_config_t cfg1 = {
		.region_number = 1,
		.region_start_address = (uint32_t)&_vectors_start,
		.region_end_address = (uint32_t)&_text_end,
		.A = TOR,
		.L = 1,
		.R = 1,
		.W = 0,
		.X = 1};

	if (pmp_configure_region(&cfg1) != 0)
	{
		printf("Error configuring region 1\n\n");
	}

	/**
	 * Now setting Machine Mode Lockdown
	 */
	setMML();
	printf("MML set\n\n");


	/**
	 * We can change it thanks to Rule Locking Bypass (enabled before)
	 * We want it to be a shared code region, so we have RX for M, X for U
	 * 
	 * (necessary, because without MML this config means not accessible for reading to M)
	 */
	cfg1.L = 1;
	cfg1.R = 0;
	cfg1.W = 1;
	cfg1.X = 1;
	if (pmp_configure_region(&cfg1) != 0)
	{
		printf("Error configuring region 1\n\n");
	}

	/**
	 * This region is used for printf() and puts(); RW for both M and U
	 */
	pmp_config_t cfgA = {
		.region_number = 0,
		.region_start_address = 0,
		.region_end_address = 0x100000,
		.A = TOR,
		.L = 0,
		.R = 0,
		.W = 1,
		.X = 1};

	if (pmp_configure_region(&cfgA) != 0)
	{
		printf("Error configuring region 0\n\n");
	}

	/**
	 * This region is used for user code. R/X for U, denied for M 
	 */
	cfg1.region_number = 3;
	cfg1.region_start_address = (uint32_t)&_usertext_start;
	cfg1.region_end_address = (uint32_t)&_usertext_end;
	cfg1.A = TOR;
	cfg1.L = 0;
	cfg1.R = 1;
	cfg1.W = 0;
	cfg1.X = 1;
	if (pmp_configure_region(&cfg1) != 0)
	{
		printf("Error configuring region 3\n\n");
	}

	/**
	 * This region is for read only data
	 * R only for both M and U
	 */
	pmp_config_t cfg2 = {
		.region_number = 4,
		.region_start_address = (uint32_t)&_rodata_start,
		.region_end_address = (uint32_t)&_rodata_end,
		.A = TOR,
		.L = 1,
		.R = 1,
		.W = 1,
		.X = 1};

	if (pmp_configure_region(&cfg2) != 0)
	{
		printf("Error configuring region 4\n\n");
	}

	user_area = (uint32_t)(&_data_start) + SHARED_DATA_DIMENSION;
	m_area = user_area + USER_DATA_DIMENSION;

	/**
	 * This region is for user only data
	 * RW for U, denied for M
	 */
	pmp_config_t cfg3;
	cfg3.region_number = 5;
	cfg3.region_start_address = user_area;
	cfg3.A = NAPOT;
	cfg3.region_dimension = USER_DATA_DIMENSION;
	cfg3.L = 0;
	cfg3.R = 1;
	cfg3.W = 1;
	cfg3.X = 0;

	if (pmp_configure_region(&cfg3) != 0)
	{
		printf("Error configuring region 6\n\n");
	}
	
	/**
	 * This region is for M only data
	 * RW for M, denied for U
	 */
	cfg3.region_number = 6;
	cfg3.region_start_address = m_area;
	cfg3.A = NAPOT;
	cfg3.region_dimension = M_DATA_DIMENSION;
	cfg3.L = 1;
	cfg3.R = 1;
	cfg3.W = 1;
	cfg3.X = 0;

	if (pmp_configure_region(&cfg3) != 0)
	{
		printf("Error configuring region 6\n\n");
	}
	
	
	/**
	 * This region is for shared data (lowest priority, so whatever doesn't match the other two)
	 * also includes .bss section
	 * RW for both M and U
	 */
	pmp_config_t cfg4 = {
		.region_number = 8,
		.region_start_address = (uint32_t)&_data_start,
		.region_end_address=(uint32_t)&_bss_end,
		.A = TOR,
		.L = 0,
		.R = 0,
		.W = 1,
		.X = 1};

	if (pmp_configure_region(&cfg4) != 0)
	{
		printf("Error configuring region 8\n\n");
	}

	/**
	 * This region includes the stack
	 * Has to be R/W for both and not X
	 */
	cfg3.region_number = 9;
	cfg3.region_start_address = (uint32_t)&_bss_end;
	cfg3.A = TOR;
	cfg3.region_end_address = (uint32_t)&_stack_start;
	cfg3.L = 0;
	cfg3.R = 0;
	cfg3.W = 1;
	cfg3.X = 1;
	if (pmp_configure_region(&cfg3) != 0)
	{
		printf("Error configuring region 8\n\n");
	}

	/**
	 * End of setup phase, disable RLB and set Whitelist Policy
	 **/
	setMMWP();
	setRLB(0);

	/**
	 * Start M mode tests
	 */	
	printf("\n****Starting tests********\n\n");
	Mtests();

	
	sim_halt();
}
