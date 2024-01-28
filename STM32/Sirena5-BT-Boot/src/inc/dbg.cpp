/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 23 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <dbg.hpp>

#if DEBUG_LEVEL > DEBUG_LEVEL_OFF
extern "C" void USART3_6_IRQHandler_Boot(void) __attribute__((__alias__("_ZN3DBG5__irqEv")));
#endif

FIFO8* DBG::tx;

void DBG::__irq(void)
{
#if DEBUG_LEVEL > DEBUG_LEVEL_OFF
	if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
	{
		if (tx->empty())
		{
			USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
		}
		else
		{
			USART_SendData(USART3, tx->pop());
		}
	}
#endif
}

void DBG::Init(void)
{
#if DEBUG_LEVEL > DEBUG_LEVEL_OFF
	tx = new FIFO8(TXBufferSize);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	USART_InitTypeDef uartTX;
	uartTX.USART_BaudRate				= TXBaud;
	uartTX.USART_WordLength				= USART_WordLength_8b;
	uartTX.USART_StopBits				= USART_StopBits_1;
	uartTX.USART_Parity					= USART_Parity_No;
	uartTX.USART_Mode					= USART_Mode_Tx;
	uartTX.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	USART_Init(USART3, &uartTX);

	GPIO_InitTypeDef pinTX;
	pinTX.GPIO_Pin		= GPIO_Pin_10;
	pinTX.GPIO_Mode		= GPIO_Mode_AF;
	pinTX.GPIO_Speed	= GPIO_Speed_Level_2;
	pinTX.GPIO_OType	= GPIO_OType_PP;
	pinTX.GPIO_PuPd		= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &pinTX);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_4);

	NVIC_EnableIRQ(USART3_6_IRQn);
	NVIC_SetPriority(USART3_6_IRQn, DBG_PRIORITY);

	USART_Cmd(USART3, ENABLE);
#endif
}

void DBG::DeInit(void)
{
	USART_Cmd(USART3, DISABLE);
	NVIC_SetPriority(USART3_6_IRQn, 0);
	NVIC_DisableIRQ(USART3_6_IRQn);
	GPIO_InitTypeDef pinTX;
	GPIO_StructInit(&pinTX);
	pinTX.GPIO_Pin		= GPIO_Pin_10;
	GPIO_Init(GPIOB, &pinTX);
}

/* stdlib */

#include <sys/stat.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
extern "C"
{
	int _read (int file, char *ptr, int len)
	{
		return 0;
	}

	int _write(int file, char *ptr, int len)
	{
#if DEBUG_LEVEL > DEBUG_LEVEL_OFF
		int length;
		USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
		length = DBG::tx->write(ptr, len);
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
		return length;
#else
		return 0;
#endif
	}

	int _close(int file)
	{
		return -1;
	}

	int _fstat(int file, struct stat *st)
	{
		st->st_mode = S_IFCHR;
		return 0;
	}

	int _isatty(int file)
	{
		return 1;
	}

	int _lseek(int file, int ptr, int dir)
	{
		return 0;
	}
}
#pragma GCC diagnostic pop
