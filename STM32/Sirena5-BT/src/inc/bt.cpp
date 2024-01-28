/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 24 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <bt.hpp>

extern "C" void USART1_IRQHandler(void) __attribute__((__alias__("_ZN2BT11__irq_usartEv")));

const uint32 BT::baudTable[] = {115200, 230400, 9600, 1200, 2400, 4800, 19200, 38400, 57600, 128000, 0};
const char* BT::resetCMD[] =
	{
		"AT+ROLE0",
		"pin",
		"AT+NAME" NAME "-" ADDQUOTES(VERSION) " ",
		"baud",
		NULL
	};

/**
 *  	JDY-30		JDY-31
 * 1	1200
 * 2	2400
 * 3	4800
 * 4	9600		9600
 * 5	19200		19200
 * 6	38400		38400
 * 7	57600		57600
 * 8	115200		115200
 * 9	230400		128000
 */

FIFO8* BT::tx;
#if BT_RX_USE_DMA == 0
FIFO8* BT::rx;
#else
char* BT::rx;
uint16 BT::rxIn;
uint16 BT::rxOut;
#endif
char* BT::buffer;
BT::tReset* BT::resetVar;
uint16 BT::length;
bool BT::reset;
bool BT::_reset;
bool BT::ready;
bool BT::_connected;
//bool BT::_connectedPrimary;
char BT::pinCode[5];
bool BT::jdy31;

void BT::__irq_usart(void)
{
	uint8 data;
#if BT_RX_USE_DMA == 0
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		data = USART_ReceiveData(USART1);
		if (!rx->full())
		{
			if (data == '\0') data = ' ';
			rx->push(data);
		}
		USART_ClearITPendingBit(USART1, USART_IT_ORE);
	}
#endif
	if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
	{
		if (tx->empty())
		{
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		}
		else
		{
			data = tx->pop();
			USART_SendData(USART1, data);
		}
	}
}

void BT::Init(void)
{
	tx = new FIFO8(TXBufferSize);

#if BT_RX_USE_DMA == 0
	rx = new FIFO8(RXBufferSize);
#else
	rx = (char*)malloc(RXBufferSize);
	rxIn = 0;
#endif
	buffer = (char*)malloc(BufferSize);
	bufferClear();

	_reset = false;
	reset = true;
	ready = false;

	resetVar = NULL;

	strcpy(pinCode, BT_PIN_DEAFULT);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	GPIO_InitTypeDef pin;
	pin.GPIO_Pin	= GPIO_Pin_7;
	pin.GPIO_Mode	= GPIO_Mode_AF;
	pin.GPIO_Speed	= GPIO_Speed_Level_2;
	pin.GPIO_OType	= GPIO_OType_PP;
	pin.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &pin);
	pin.GPIO_Pin	= GPIO_Pin_6;
	pin.GPIO_OType	= GPIO_OType_OD;
	pin.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &pin);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_0);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_0);

	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn, BT_PRIORITY);

	pin.GPIO_Pin	= GPIO_Pin_7;
	pin.GPIO_Mode	= GPIO_Mode_OUT;
	pin.GPIO_Speed	= GPIO_Speed_Level_2;
	pin.GPIO_OType	= GPIO_OType_PP;
	pin.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &pin);

	pin.GPIO_Pin	= GPIO_Pin_8;
	pin.GPIO_Mode	= GPIO_Mode_IN;
	pin.GPIO_PuPd	= GPIO_PuPd_DOWN;
	GPIO_Init(GPIOB, &pin);

#if BT_RX_USE_DMA != 0
	DMA1->RMPCR &= 0xFFF0FFFF;
	DMA1->RMPCR |= DMA_RMPCR1_CH5_USART1_RX;
	DMA_InitTypeDef dma;
	DMA_StructInit(&dma);
	dma.DMA_PeripheralBaseAddr		= (uint32)&USART1->RDR;
	dma.DMA_MemoryBaseAddr			= (uint32)rx;
	dma.DMA_BufferSize				= RXBufferSize;
	dma.DMA_MemoryInc				= DMA_MemoryInc_Enable;
	dma.DMA_Mode					= DMA_Mode_Circular;
	DMA_Init(DMA1_Channel5, &dma);
	DMA_Cmd(DMA1_Channel5, ENABLE);

	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
#endif

	CALL::setImmediate(thread);
}

void BT::DeInit(void)
{
#if BT_RX_USE_DMA != 0
	USART_DMACmd(USART1, USART_DMAReq_Rx, DISABLE);
	DMA_Cmd(DMA1_Channel5, DISABLE);
	DMA1->RMPCR &= 0xFFF0FFFF;
#endif
	USART_Cmd(USART1, DISABLE);
	NVIC_DisableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn, 0);
	GPIO_InitTypeDef pin;
	GPIO_StructInit(&pin);
	pin.GPIO_Pin	= GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_8;
	GPIO_Init(GPIOB, &pin);
	pin.GPIO_Pin	= GPIO_Pin_7;
	GPIO_Init(GPIOA, &pin);
}

void BT::bufferClear(void)
{
	length = 0;
	*buffer = '\0';
}

void BT::write(const char* data)
{
	size_t length = strlen(data);
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	tx->write(data, length);
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

void BT::send(const char* data)
{
	size_t length = strlen(data);
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	tx->write(data, length);
	tx->push('\r');
	tx->push('\n');
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

void BT::answer(tCmdAT* at, const char* data)
{
	size_t length = strlen(at->command);
	size_t length2 = strlen(data);
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	tx->push('+');
	tx->write(at->command, length);
	tx->push(':');
	tx->write(data, length2);
	tx->push('\r');
	tx->push('\n');
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

void BT::thread(__UNUSED void* param)
{
	if (reset && !_reset)
	{
		_reset = true;
		resetVar = (tReset*)malloc(sizeof(tReset));
		//resetVar->firstTry = 3;
		resetVar->baudIndex = 0;
		resetStart(NULL);
	}
	/*if (!_connectedPrimary)
	{
		if (!_connected && (GPIOB->IDR & GPIO_Pin_8))
		{
			BT::connectedCmd(NULL);
		}
		if (_connected && !(GPIOB->IDR & GPIO_Pin_8))
		{
			BT::disconnectedCmd(NULL);
		}
	}*/
#if BT_RX_USE_DMA == 0
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	if (!rx->empty())
	{
		length += rx->read(&buffer[length], BufferSize - length - 1);
		buffer[length] = '\0';
		if (!rx->empty())
		{
			USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
			bufferClear();
			DBGP_WARNING("BT: overflow");
		}
		else
		{
			USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
		}
		uint16 commandLength;
		tCmdAT* at;
		while ((at = parse(&buffer[0], &commandLength)) != NULL)
		{
			execute(at);
			remove(at);
			if (commandLength == 0)
			{
				bufferClear();
				break;
			}
			else
			{
				bufferShift(commandLength);
			}
		}
	}
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
#else
	rxOut = RXBufferSize - DMA_GetCurrDataCounter(DMA1_Channel5);
	if (rxOut != rxIn)
	{
		if (rxOut > rxIn)
		{
			bufferAdd(&rx[rxIn], rxOut - rxIn);
		}
		else
		{
			bufferAdd(&rx[rxIn], RXBufferSize - rxIn);
			bufferAdd(&rx[0], rxOut);
		}
		rxIn = rxOut;
		uint16 commandLength;
		tCmdAT* at;
		while ((at = parse(&buffer[0], &commandLength)) != NULL)
		{
			execute(at);
			remove(at);
			if (commandLength == 0)
			{
				bufferClear();
				break;
			}
			else
			{
				bufferShift(commandLength);
			}
		}
	}
#endif
	CALL::setImmediate(thread);
}

void BT::bufferShift(uint16 shift)
{
	if (shift > length)
	{
		DBGF_WARNING("BT: error bufferShift %u/%u", shift, length);
		length = shift;
	}
	length -= shift;
	memcpy(&buffer[0], &buffer[0] + shift, length);
	buffer[length] = '\0';
}

#if BT_RX_USE_DMA != 0
void BT::bufferAdd(const char* data, uint16 len)
{
	memcpy(&buffer[length], data, len);
	uint16 i;
	for (i = length; i < (length + len); i++)
	{
		if (buffer[i] == '\0') buffer[i] = ' ';
	}
	length += len;
	buffer[length] = '\0';
}
#endif

void BT::remove(tCmdAT* at)
{
	uint16 i;
	for (i = 0; i < at->argc; i++)
	{
		free(at->argv[i]);
	}
	free(at);
}

BT::tCmdAT* BT::parse(char* buff, uint16* length)
{
	char* end;
	tCmdAT* at = NULL;
	end = strstr(buff, "\r\n");
	if (end == NULL)
	{
		*length = 0;
	}
	else
	{
		if (length != NULL) *length = end - buff + 2;
		at = (tCmdAT*)malloc(sizeof(tCmdAT));
		*end = '\0';
		end--;
		if (*end == '\r') *end = '\0';
		at->command = buff;
		while (*at->command == ' ' && at->command < end)
		{
			at->command++;
		}
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
			/*end = NULL;
			do
			{
				if (end != NULL)
				{
					*end = '\0';
					vars = end + 1;
					if (at->argc >= ATArgsMax)
					{
						break;
					}
				}
				at->argv[at->argc] = (tArgAT*)malloc(sizeof(tArgAT));
				at->argv[at->argc]->data = vars;
				at->argc++;
				end = strchr(vars, ';');
			}
			while (end != NULL);*/
			tStrVar* e = NULL;
			//end = NULL;
			do
			{
				if (/*end != NULL*/ e != NULL)
				{
					//*end = '\0';
					//vars = end + 1;
					vars = e->nextPTR;
					if (at->argc >= ATArgsMax)
					{
						break;
					}
					free(e);
				}
				at->argv[at->argc] = (tArgAT*)malloc(sizeof(tArgAT));
				at->argv[at->argc]->data = vars;
				//end = strchr(vars, ';');
				e = _strvar(vars);
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

void BT::execute(tCmdAT* at)
{
	/*DBGF("BT: command '%s'", at->command);
	uint16 i;
	for (i = 0; i < at->argc; i++)
	{
		DBGF("BT: arg '%s'", at->argv[i]->data);
	}*/
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
	if (list->command == NULL)
	{
		if (at->argc > 0)
		{
			DBGF_NOTICE("BT: no '%s':'%s' %u", at->command, at->argv[0]->data, at->argc);
		}
		else
		{
			DBGF_NOTICE("BT: no '%s'", at->command);
		}
	}
}

BT::tStrVar* BT::_strvar(char* src)
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
}

void BT::resetStart(__UNUSED void* param)
{
	GPIOA->BRR = GPIO_Pin_7;

	USART_Cmd(USART1, DISABLE);

#if BT_RX_USE_DMA == 0
	rx->clear();
#endif

	bufferClear();

	tx->clear();

	USART_InitTypeDef uart;
	uart.USART_BaudRate				= baudTable[resetVar->baudIndex];
	uart.USART_WordLength			= USART_WordLength_8b;
	uart.USART_StopBits				= USART_StopBits_1;
	uart.USART_Parity				= USART_Parity_No;
	uart.USART_Mode					= USART_Mode_Tx | USART_Mode_Rx;
	uart.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	USART_Init(USART1, &uart);

	USART_Cmd(USART1, ENABLE);

	DBGF_NOTICE("BT: init %i", (int)baudTable[resetVar->baudIndex]);

	ready = false;
	_connected = false;
	//_connectedPrimary = false;
	resetVar->testAT = false;
	jdy31 = false;

	CALL::setTimeout(resetWait, 100);
}

void BT::resetWait(__UNUSED void* param)
{
	resetVar->timeoutHandle = CALL::setTimeout(resetTimeout, 300);
	GPIOA->BSRR = GPIO_Pin_7;
}

void BT::resetTimeout(__UNUSED void* param)
{
	if (resetVar->testAT)
	{
		/*if (resetVar->firstTry > 0) resetVar->firstTry--;
		if (resetVar->firstTry == 0)
		{*/
			resetVar->baudIndex++;
			if (baudTable[resetVar->baudIndex] == 0)
			{
				free(resetVar);
				_reset = false;
				reset = false;
				GPIOA->BRR = GPIO_Pin_7;
				DBGP_FATAL("BT: failure");
				return;
			}
		//}
		resetStart(NULL);
	}
	else
	{
		resetVar->testAT = true;
		send("AT+VERSION");
		//DBGP_NOTICE("BT: test AT");
		resetWait(NULL);
	}
}

void BT::readyCmd(__UNUSED tCmdAT* at)
{
	if (!ready)
	{
		CALL::clear(resetVar->timeoutHandle);
		ready = true;
		resetVar->commandIndex = 0;
		resetCommand(NULL);
	}
}

void BT::testAT(tCmdAT* at)
{
	if (at->argc > 0)
	{
		if (strstr(at->argv[0]->data, "JDY-31") != NULL)
		{
			jdy31 = true;
		}
		DBGF_NOTICE("BT: ver %s", at->argv[0]->data);
	}
	readyCmd(at);
}

void BT::resetCommand(__UNUSED void* param)
{
	if (resetCMD[resetVar->commandIndex] == NULL)
	{
		if ((jdy31 && resetVar->baudIndex == 0) || (!jdy31 && resetVar->baudIndex == 1))
		{
			free(resetVar);
			resetVar = NULL;
			_reset = false;
			reset = false;
			DBGP_NOTICE((jdy31) ? "BT: ok (JDY-31)" : "BT: ok (JDY-30)");
		}
		else
		{
			resetVar->baudIndex = 0;
			send("AT+RESET");
			CALL::setTimeout(resetStart, 100);
		}
	}
	else
	{
		if (strcmp(resetCMD[resetVar->commandIndex], "pin") == 0)
		{
			write("AT+PIN");
			send(pinCode);
		}
		else if (strcmp(resetCMD[resetVar->commandIndex], "baud") == 0)
		{
			send((jdy31) ? "AT+BAUD8" : "AT+BAUD9");
		}
		else
		{
			send(resetCMD[resetVar->commandIndex]);
		}
		resetVar->commandIndex++;
		CALL::setTimeout(resetCommand, 100);
	}
}

void BT::connectedCmd(__UNUSED tCmdAT* at)
{
	if (!_connected)
	{
		//if (at != NULL) _connectedPrimary = true;
		_connected = true;
		DBGP_NOTICE("BT: connected");
	}
	if (resetVar != NULL)
	{
		free(resetVar);
		resetVar = NULL;
		_reset = false;
		reset = false;
		ready = true;
		DBGP_WARNING("BT: ok from connected");
	}
}

void BT::disconnectedCmd(tCmdAT* at)
{
	if (_connected)
	{
		_connected = false;
		if (at == NULL || at->argc == 0)
		{
			DBGP_NOTICE("BT: disconnected");
		}
		else
		{
			DBGF_NOTICE("BT: disconnected '%s'", at->argv[0]->data);
		}
	}
}

bool BT::connected(void)
{
	return _connected;
}

bool BT::testUNumber(tArgAT* argv, uint32 min, uint32 max)
{
	return (argv->isNumber && argv->uNumber >= min && argv->uNumber <= max);
}

bool BT::testNumber(tArgAT* argv, int min, int max)
{
	return (argv->isNumber && argv->sNumber >= min && argv->sNumber <= max);
}
