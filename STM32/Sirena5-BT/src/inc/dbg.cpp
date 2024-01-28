/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 23 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <dbg.hpp>
#include <timer.hpp>
#include <call.hpp>

#if DEBUG_LEVEL > DEBUG_LEVEL_OFF
extern "C" void USART3_6_IRQHandler(void) __attribute__((__alias__("_ZN3DBG5__irqEv")));
#endif

FIFO8* DBG::tx;
FIFO8* DBG::rx;
bool DBG::_uartControl;
int DBG::handleThread;
char* DBG::buffer;
uint16 DBG::length;

void DBG::__irq(void)
{
	if (_uartControl && USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		uint8 data = USART_ReceiveData(USART3);
		if (!rx->full())
		{
			rx->push(data);
		}
		USART_ClearITPendingBit(USART3, USART_IT_ORE);
	}
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
}

void DBG::Init(void)
{
	_uartControl = false;
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
	if (_uartControl)
	{
		CALL::clear(handleThread);

		USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
		USART_Cmd(USART3, DISABLE);
		USART_HalfDuplexCmd(USART3, DISABLE);
		NVIC_SetPriority(USART3_6_IRQn, 0);
		NVIC_DisableIRQ(USART3_6_IRQn);

		free(buffer);
		delete(rx);
		delete(tx);
	}
	else
	{
#if DEBUG_LEVEL > DEBUG_LEVEL_OFF
		USART_Cmd(USART3, DISABLE);
		NVIC_SetPriority(USART3_6_IRQn, 0);
		NVIC_DisableIRQ(USART3_6_IRQn);
		GPIO_InitTypeDef pinTX;
		GPIO_StructInit(&pinTX);
		pinTX.GPIO_Pin		= GPIO_Pin_10;
		GPIO_Init(GPIOB, &pinTX);
		delete(tx);
#endif
	}
}

void DBG::InitAT()
{
	tx = new FIFO8(UARTBufferSize);
	rx = new FIFO8(UARTBufferSize);
	buffer = (char*)malloc(BufferSize);
	bufferClear();

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	USART_InitTypeDef uart;
	uart.USART_BaudRate				= UARTBaud;
	uart.USART_WordLength			= USART_WordLength_8b;
	uart.USART_StopBits				= USART_StopBits_1;
	uart.USART_Parity				= USART_Parity_No;
	uart.USART_Mode					= USART_Mode_Tx | USART_Mode_Rx;
	uart.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	USART_Init(USART3, &uart);

	GPIO_InitTypeDef pin;
	pin.GPIO_Pin		= GPIO_Pin_10;
	pin.GPIO_Mode		= GPIO_Mode_AF;
	pin.GPIO_Speed		= GPIO_Speed_Level_2;
	pin.GPIO_OType		= GPIO_OType_OD;
	pin.GPIO_PuPd		= GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &pin);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_4);

	NVIC_EnableIRQ(USART3_6_IRQn);
	NVIC_SetPriority(USART3_6_IRQn, UART_PRIORITY);

	USART_HalfDuplexCmd(USART3, ENABLE);

	USART_Cmd(USART3, ENABLE);

	handleThread = CALL::setImmediate(Thread);
}

void DBG::ReInit(bool uartControl)
{
	if (_uartControl != uartControl)
	{
		if (uartControl)
		{
			DBGP("UART Control: enable");
			while (!tx->empty());
			TIMER::delay(10);
			DeInit();
			InitAT();
			_uartControl = true;
		}
		else
		{
			DeInit();
			Init();
			_uartControl = false;
			DBGP("UART Control: disable");
		}
	}
}

bool DBG::getUartControl()
{
	return _uartControl;
}

void DBG::Thread(void* param __UNUSED)
{
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
	if (!rx->empty())
	{
		length += rx->read(&buffer[length], BufferSize - length - 1);
		buffer[length] = '\0';
		if (!rx->empty())
		{
			USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
			bufferClear();
		}
		else
		{
			USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
		}
		uint16 commandLength;
		tCmdAT* at;
		while ((at = parse(&buffer[0], &commandLength)) != NULL)
		{
			execute(at);
			remove(at);
			bufferShift(commandLength);
		}
	}
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	handleThread = CALL::setImmediate(Thread);
}

void DBG::bufferClear(void)
{
	length = 0;
	*buffer = '\0';
}

void DBG::bufferShift(uint16 shift)
{
	if (shift > length)
	{
		length = shift;
	}
	length -= shift;
	memcpy(&buffer[0], &buffer[0] + shift, length);
	buffer[length] = '\0';
}

void DBG::remove(tCmdAT* at)
{
	uint16 i;
	for (i = 0; i < at->argc; i++)
	{
		free(at->argv[i]);
	}
	free(at);
}

DBG::tCmdAT* DBG::parse(char* buff, uint16* length)
{
	char* end;
	tCmdAT* at = NULL;
	end = strstr(buff, "\r\n");
	if (end != NULL)
	{
		if (length != NULL) *length = end - buff + 2;
		at = (tCmdAT*)malloc(sizeof(tCmdAT));
		*end = '\0';
		end--;
		if (*end == '\r') *end = '\0';
		at->command = buff;
		at->argc = 0;
		char* vars = NULL;
		end = strchr(buff, '=');
		if (end != NULL)
		{
			vars = end + 1;
			*end = '\0';
		}
		else
		{
			end = strstr(buff, "<<");
			if (end != NULL)
			{
				vars = end + 2;
				*end = '\0';
			}
			else
			{
				end = strchr(buff, ':');
				if (end != NULL)
				{
					vars = end + 1;
					*end = '\0';
				}
			}
		}
		if (vars != NULL)
		{
			BT::tStrVar* e = NULL;
			do
			{
				if (e != NULL)
				{
					vars = e->nextPTR;
					if (at->argc >= ATArgsMax)
					{
						break;
					}
					free(e);
				}
				at->argv[at->argc] = (tArgAT*)malloc(sizeof(tArgAT));
				at->argv[at->argc]->data = vars;
				e = BT::_strvar(vars);
				at->argv[at->argc]->length = e->length;
				at->argv[at->argc]->isNumber = e->isNumber;
				at->argv[at->argc]->sNumber = e->sNumber;
				at->argv[at->argc]->uNumber = e->uNumber;
				at->argc++;
			}
			while (/*end != NULL*/ e->nextPTR != NULL);
			free(e);
		}
	}
	return at;
}

void DBG::execute(tCmdAT* at)
{
	const tListAT* list = &listAT[0];
	while (list->command != NULL)
	{
		if (strcmp(list->command, at->command) == 0)
		{
			if (list->proc != NULL) list->proc(at);
			break;
		}
		list++;
	}
}

/*DBG::tStrVar* DBG::_strvar(char* src)
{
	tStrVar* r = (tStrVar*)malloc(sizeof(tStrVar));
	char* ptr = src;
	uint16 length = 0;
	r->nextPTR = NULL;
	r->isNumber = false;
	if (src != NULL)
	{
		r->isNumber = true;
		while (*ptr != '\0')
		{
			if (*ptr == ';')
			{
				*ptr++ = '\0';
				r->nextPTR = ptr;
				break;
			}
			if ((*ptr < '0' || *ptr > '9') && *ptr != '-')
			{
				r->isNumber = false;
			}
			ptr++;
			length++;
		}
		if (r->isNumber)
		{
			sscanf(src, "%i", (int*)&r->sNumber);
			sscanf(src, "%u", (unsigned int*)&r->uNumber);
		}
		else
		{
			r->sNumber = 0;
			r->uNumber = 0;
		}
	}
	r->length = length;
	return r;
}*/

void DBG::write(const char* data)
{
	if (DBG::_uartControl)
	{
		size_t length = strlen(data);
		USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
		tx->write(data, length);
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	}
}

void DBG::send(const char* data)
{
	if (DBG::_uartControl)
	{
		size_t length = strlen(data);
		USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
		tx->write(data, length);
		tx->push('\r');
		tx->push('\n');
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	}
}

void DBG::answer(tCmdAT* at, const char* data)
{
	size_t length = strlen(at->command);
	size_t length2 = strlen(data);
	USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
	tx->push('+');
	tx->write(at->command, length);
	tx->push(':');
	tx->write(data, length2);
	tx->push('\r');
	tx->push('\n');
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
}

void DBG::printBuff(const char* prefix, void* data, int length)
{
	uint8* buff = (uint8*)data;
	uint8 i, n;
	n = 0;
	printf("%s    |", prefix);
	for (i = 0; i < 32; i++)
	{
		printf(" %02X", i);
	}
	putchar('\n');
	while (length > 0)
	{
		printf("%s %02X | ", prefix, n);
		i = 0;
		while (i++ < 32 && length-- > 0)
		{
			printf("%02X ", *buff++);
		}
		putchar('\n');
		n++;
	}
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
		if (DBG::_uartControl)
		{
			length = 0;
		}
		else
		{
			USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
			length = DBG::tx->write(ptr, len);
			USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
		}
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
