/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 3 апр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_REMAPVECT_H_
#define INC_REMAPVECT_H_ 1

#include <stm32f0xx.h>

#define JMP(handler) \
	asm volatile \
	( \
		"ldr %0, [%0]"	"\n" \
		"bx %0"			"\n" \
		:: "r" (&handler) \
	)

#define REMAP(handler) \
	__attribute__((weak, alias("DefaultHandler"))) \
	void handler##_Boot(void); \
	__attribute__((section(".after_vectors"), naked)) \
	void handler(void) \
	{ \
		asm volatile \
		( \
			"ldr %0, [%0]"	"\n" \
			"bx %0"			"\n" \
			:: "r" (&ramVect.handler) \
		); \
	};

#define REMAP_SET(handler) .handler = handler##_Boot,

typedef void (*const pHandler)(void);

#if defined(STM32F030xC)
typedef struct
{
	uint32_t estack;
	pHandler Reset_Handler;
	pHandler NMI_Handler;
	pHandler HardFault_Handler;
	pHandler res1[7];
	pHandler SVC_Handler;
	pHandler res2[2];
	pHandler PendSV_Handler;
	pHandler SysTick_Handler;
	pHandler WWDG_IRQHandler;
	pHandler res3;
	pHandler RTC_IRQHandler;
	pHandler FLASH_IRQHandler;
	pHandler RCC_IRQHandler;
	pHandler EXTI0_1_IRQHandler;
	pHandler EXTI2_3_IRQHandler;
	pHandler EXTI4_15_IRQHandler;
	pHandler res4;
	pHandler DMA1_Channel1_IRQHandler;
	pHandler DMA1_Channel2_3_IRQHandler;
	pHandler DMA1_Channel4_5_IRQHandler;
	pHandler ADC1_IRQHandler;
	pHandler TIM1_BRK_UP_TRG_COM_IRQHandler;
	pHandler TIM1_CC_IRQHandler;
	pHandler res5;
	pHandler TIM3_IRQHandler;
	pHandler TIM6_IRQHandler;
	pHandler TIM7_IRQHandler;
	pHandler TIM14_IRQHandler;
	pHandler TIM15_IRQHandler;
	pHandler TIM16_IRQHandler;
	pHandler TIM17_IRQHandler;
	pHandler I2C1_IRQHandler;
	pHandler I2C2_IRQHandler;
	pHandler SPI1_IRQHandler;
	pHandler SPI2_IRQHandler;
	pHandler USART1_IRQHandler;
	pHandler USART2_IRQHandler;
	pHandler USART3_6_IRQHandler;
}
isrHandlers;
#else
#error "remapVect not supported MCU"
#endif

#define callBoot() \
	extern const isrHandlers bootProgramm; \
	__DSB(); \
	__ISB(); \
	__set_MSP(bootProgramm.estack); \
	JMP(bootProgramm.Reset_Handler)

#ifdef __cplusplus
extern "C" {
#endif

void initVect(void);	/* call in boot, Reset_Handler */
void callMain(void);	/* call main application */

void appCall(const isrHandlers* handlers);
void setVect(const isrHandlers* handlers);

void DefaultHandler(void);

#ifdef __cplusplus
}
#endif

#endif
