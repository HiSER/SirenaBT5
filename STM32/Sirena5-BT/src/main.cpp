#include <main.hpp>
#include <timer.hpp>
#include <dbg.hpp>
#include <call.hpp>
#include <encoders.hpp>
#include <bt.hpp>
#include <amp.hpp>
#include <ch.hpp>
#include <sirena.hpp>
#include <sd.hpp>
#include <fs.hpp>
#include <config.hpp>
#include <remapVect.h>

void callBootCallback(__UNUSED void* param)
{
	CONFIG::DeInit();
	FS::DeInit();
	SD::DeInit();
	BT::DeInit();
	AMP::DeInit();
	CH::DeInit();
	CALL::DeInit();
	TIMER::DeInit();
	DBG::DeInit();
	SIRENA::DeInit();

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE);
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, DISABLE);
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, DISABLE);
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, DISABLE);

	USART3->BRR = 0xFEFE;

	callBoot();
}


char b[512];
TIMER* t;
int i;
int h;

void testThread(void* p __UNUSED);
void nextTest();
void endTest();
void startTest();

void testThread(void* p __UNUSED)
{
	if (SD::busy(h))
	{
		CALL::setImmediate(testThread);
	}
	else if (SD::error(h))
	{
		SD::clear(h);
		DBGF("TEST: error read %i", i);
	}
	else
	{
		SD::clear(h);
		i++;
		nextTest();
	}
}

void nextTest()
{
	if (i == 2048)
	{
		endTest();
	}
	else
	{
		h = SD::read(i, &b, 512);
		if (h == -1)
		{
			DBGF("TEST: error handle %i", i);
		}
		else
		{
			CALL::setImmediate(testThread);
		}
	}
}

void endTest()
{
	int e = t->get();
	int b = i * 512;
	DBGF("TEST: readed %i, time %i, %i kb*s", b, e, (b / e));
	delete(t);
}

void startTest()
{
	DBGP("TEST: start");
	t = new TIMER();
	t->set();
	i = 0;
	nextTest();
}

void main(void)
{
	FLASH_SetLatency(FLASH_Latency_1);
	FLASH_PrefetchBufferCmd(ENABLE);

	IWDG_ReloadCounter();

	DBG::Init();
	DBGP(BUILD);

	TIMER::Init();
	CALL::Init();
	CH::Init();
	AMP::Init();
	BT::Init();
	SD::Init();
	FS::Init();
	SIRENA::Init();

	CONFIG::Init();
	CH::Start();

#define IWDG_RELOAD ((40000 / 256) * IWDG_TIME_SECOND)
#if IWDG_RELOAD > 4095
#error IWDG_RELOAD > 4095, change IWDG_TIME_SECOND
#endif
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	IWDG_SetReload(IWDG_RELOAD);
	IWDG_ReloadCounter();
	IWDG_Enable();
#undef IWDG_RELOAD

	//startTest();

	LOOP
	{
		IWDG_ReloadCounter();
		CALL::Thread();
	}
}



