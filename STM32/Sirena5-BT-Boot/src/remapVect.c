/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 3 апр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <remapVect.h>

__attribute__((section(".isrHandlers")))
isrHandlers ramVect;

REMAP(NMI_Handler);
REMAP(HardFault_Handler);
REMAP(SVC_Handler);
REMAP(PendSV_Handler);
REMAP(SysTick_Handler);
REMAP(WWDG_IRQHandler);
REMAP(RTC_IRQHandler);
REMAP(FLASH_IRQHandler);
REMAP(RCC_IRQHandler);
REMAP(EXTI0_1_IRQHandler);
REMAP(EXTI2_3_IRQHandler);
REMAP(EXTI4_15_IRQHandler);
REMAP(DMA1_Channel1_IRQHandler);
REMAP(DMA1_Channel2_3_IRQHandler);
REMAP(DMA1_Channel4_5_IRQHandler);
REMAP(ADC1_IRQHandler);
REMAP(TIM1_BRK_UP_TRG_COM_IRQHandler);
REMAP(TIM1_CC_IRQHandler);
REMAP(TIM3_IRQHandler);
REMAP(TIM6_IRQHandler);
REMAP(TIM7_IRQHandler);
REMAP(TIM14_IRQHandler);
REMAP(TIM15_IRQHandler);
REMAP(TIM16_IRQHandler);
REMAP(TIM17_IRQHandler);
REMAP(I2C1_IRQHandler);
REMAP(I2C2_IRQHandler);
REMAP(SPI1_IRQHandler);
REMAP(SPI2_IRQHandler);
REMAP(USART1_IRQHandler);
REMAP(USART2_IRQHandler);
REMAP(USART3_6_IRQHandler);

const isrHandlers bootProgramm =
	{
		REMAP_SET(NMI_Handler)
		REMAP_SET(HardFault_Handler)
		REMAP_SET(SVC_Handler)
		REMAP_SET(PendSV_Handler)
		REMAP_SET(SysTick_Handler)
		REMAP_SET(WWDG_IRQHandler)
		REMAP_SET(RTC_IRQHandler)
		REMAP_SET(FLASH_IRQHandler)
		REMAP_SET(RCC_IRQHandler)
		REMAP_SET(EXTI0_1_IRQHandler)
		REMAP_SET(EXTI2_3_IRQHandler)
		REMAP_SET(EXTI4_15_IRQHandler)
		REMAP_SET(DMA1_Channel1_IRQHandler)
		REMAP_SET(DMA1_Channel2_3_IRQHandler)
		REMAP_SET(DMA1_Channel4_5_IRQHandler)
		REMAP_SET(ADC1_IRQHandler)
		REMAP_SET(TIM1_BRK_UP_TRG_COM_IRQHandler)
		REMAP_SET(TIM1_CC_IRQHandler)
		REMAP_SET(TIM3_IRQHandler)
		REMAP_SET(TIM6_IRQHandler)
		REMAP_SET(TIM7_IRQHandler)
		REMAP_SET(TIM14_IRQHandler)
		REMAP_SET(TIM15_IRQHandler)
		REMAP_SET(TIM16_IRQHandler)
		REMAP_SET(TIM17_IRQHandler)
		REMAP_SET(I2C1_IRQHandler)
		REMAP_SET(I2C2_IRQHandler)
		REMAP_SET(SPI1_IRQHandler)
		REMAP_SET(SPI2_IRQHandler)
		REMAP_SET(USART1_IRQHandler)
		REMAP_SET(USART2_IRQHandler)
		REMAP_SET(USART3_6_IRQHandler)
	};

extern
const isrHandlers mainProgramm;

__attribute__((section(".after_vectors")))
void initVect(void)
{
	setVect(&bootProgramm);
}

__attribute__((section(".after_vectors"), naked, noreturn))
void callMain(void)
{
	appCall(&mainProgramm);
}

__attribute__((section(".after_vectors")))
bool testMain(void)
{
	return
		(
			   FLASH_OB_GetRDP() != RESET
			&& ((uint32_t)mainProgramm.estack & 0xFFF00000UL) == 0x20000000UL
			&& ((uint32_t)mainProgramm.Reset_Handler & 0xFFF00000UL) == 0x08000000UL
		);
}

__attribute__((section(".after_vectors")))
void setVect(const isrHandlers* handlers)
{
	uint32_t i;
	uint32_t* d = (uint32_t*)&ramVect;
	uint32_t* s = (uint32_t*)handlers;
	for (i = 0; i < sizeof(isrHandlers); i += 4)
	{
		*d++ = *s++;
	}
	__DSB();
}

__attribute__((section(".after_vectors"), naked, noreturn))
void appCall(const isrHandlers* handlers)
{
	setVect(handlers);
	__ISB();
	__set_MSP(handlers->estack);
	JMP(handlers->Reset_Handler);
}

__attribute__((section(".after_vectors"), naked, noreturn))
void DefaultHandler(void)
{
	while (1) {};
}
