#include <main.hpp>
#include <timer.hpp>
#include <dbg.hpp>
#include <call.hpp>
#include <encoders.hpp>
#include <bt.hpp>
#include <remapVect.h>

void powerOn(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitTypeDef pin;
	pin.GPIO_Pin	= GPIO_Pin_9;
	pin.GPIO_Mode	= GPIO_Mode_OUT;
	pin.GPIO_Speed	= GPIO_Speed_Level_1;
	pin.GPIO_OType	= GPIO_OType_PP;
	pin.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &pin);
	GPIOB->BSRR = GPIO_Pin_9;
}

void channels(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	GPIO_InitTypeDef pin;
	pin.GPIO_Pin	= GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	pin.GPIO_Mode	= GPIO_Mode_IN;
	pin.GPIO_Speed	= GPIO_Speed_Level_1;
	pin.GPIO_OType	= GPIO_OType_PP;
	pin.GPIO_PuPd	= GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &pin);
}

bool isBoot()
{
	uint32 i = 2000000;
	__DSB();
	__ISB();
	while ((GPIOA->IDR & (GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11)) == (GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11) && i > 0)
	{
		i--;
	}
	return (!testMain() || i == 0 || USART3->BRR == 0xFEFE);
}

void callMainCallback(__UNUSED void* param)
{
	BT::DeInit();
	CALL::DeInit();
	TIMER::DeInit();
	DBG::DeInit();

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE);
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, DISABLE);
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, DISABLE);

	callMain();
}

void main(void)
{
	FLASH_SetLatency(FLASH_Latency_1);
	FLASH_PrefetchBufferCmd(ENABLE);

	IWDG_ReloadCounter();

	powerOn();
	channels();

	if (!isBoot()) callMain();

	#if AUTO_LOCK == 1
	if (FLASH_OB_GetRDP() == RESET)
	{
		FLASH_Unlock();
		FLASH_OB_Unlock();
		if (FLASH_OB_RDPConfig(OB_RDP_Level_1) == FLASH_COMPLETE) FLASH_OB_Launch();
		FLASH_OB_Lock();
		FLASH_Lock();
	}
	#endif

	DBG::Init();
	DBGP(BUILD);

	TIMER::Init();
	CALL::Init();
	BT::Init();

	LOOP
	{
		IWDG_ReloadCounter();
		CALL::Thread();
	}
}



