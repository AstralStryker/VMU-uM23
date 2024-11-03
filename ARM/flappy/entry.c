#define WEAK __attribute__ ((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

#include <stdint.h>


WEAK void IntDefaultHandler(void);
WEAK void NMI_Handler(void) ALIAS(IntDefaultHandler);
WEAK void HardFault_Handler(void) ALIAS(IntDefaultHandler);
WEAK void SVC_Handler(void) ALIAS(IntDefaultHandler);
WEAK void PendSV_Handler(void) ALIAS(IntDefaultHandler);
WEAK void SysTick_Handler(void) ALIAS(IntDefaultHandler);

//main must exist
extern int main(void);

//stack top (provided by linker)
extern void __stack_top();
extern void __data_data();
extern void __data_start();
extern void __data_end();
extern void __bss_start();
extern void __bss_end();



#define INFINITE_LOOP_LOW_POWER		while (1) {				\
							asm("wfi":::"memory");	\
						}

//shove some thigns into first flash block as we might as well...
void __attribute__((noreturn)) IntDefaultHandler(void)
{
	INFINITE_LOOP_LOW_POWER
}

//shove some thigns into first flash block as we might as well...
static void __attribute__((noreturn)) ResetISR(void)
{
	uint32_t *dst, *src, *end;

	//copy data
	dst = (uint32_t*)&__data_start;
	src = (uint32_t*)&__data_data;
	end = (uint32_t*)&__data_end;
	while(dst != end)
		*dst++ = *src++;

	//init bss
	dst = (uint32_t*)&__bss_start;
	end = (uint32_t*)&__bss_end;
	while(dst != end)
		*dst++ = 0;

//maybe this?
	//SystemInit();

	main();

//if main returns => bad
	HardFault_Handler();
	
//do not return
	while(1);
}


//vector table

__attribute__ ((section(".vectors"))) void (*const __VECTORS[]) (void) =
{
	&__stack_top,		// The initial stack pointer
	ResetISR,		// The reset handler
	NMI_Handler,		// The NMI handler
	HardFault_Handler,	// The hard fault handler
	
	0,			// Reserved
	0,			// Reserved
	0,			// Reserved
	0,			// Reserved
	0,			// Reserved
	0,			// Reserved
	0,			// Reserved
	
	SVC_Handler,		// SVCall handler
	0,			// Reserved
	0,			// Reserved
	PendSV_Handler,		// The PendSV handler
	SysTick_Handler,	// The SysTick handler
};


