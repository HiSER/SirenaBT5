/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 11 мар. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <sd.hpp>

#if SD_REPLACE_TO_SDD == 0

extern "C" void SPI1_IRQHandler(void) __attribute__((__alias__("_ZN2SD5__irqEv")));

SD::tPool SD::pool;
SD::tSD SD::sd;
bool SD::ready;
bool SD::_reset;
bool SD::reset;

int SD::read(uint32 address, void* data, uint16 length, int timeout)
{
	if (data == NULL) return -1;
	tTask* task;
	int handle;
	if (pool.length == POOL_SIZE)
	{
		DBGP_WARNING("SD: read overflow");
		return -1;
	}
	do
	{
		task = &pool.task[pool.in];
		handle = pool.in;
		pool.in++;
		if (pool.in == POOL_SIZE) pool.in = 0;
	}
	while (task->status != statEmpty);
	task->timeout = ((timeout > 0) ? new TIMER(timeout) : NULL);
	task->address = address;
	task->data = (uint8*)data;
	task->length = length;
	task->status = statWait;
	task->read = true;
	pool.length++;
	if (pool.length > pool.lengthMax) pool.lengthMax = pool.length;
	return handle;
}

int SD::write(uint32 address, const void* data, uint16 length, int timeout)
{
	if (data == NULL) return -1;
	tTask* task;
	int handle;
	if (pool.length == POOL_SIZE)
	{
		DBGP_WARNING("SD: write overflow");
		return -1;
	}
	do
	{
		task = &pool.task[pool.in];
		handle = pool.in;
		pool.in++;
		if (pool.in == POOL_SIZE) pool.in = 0;
	}
	while (task->status != statEmpty);
	task->timeout = ((timeout > 0) ? new TIMER(timeout) : NULL);
	task->address = address;
	task->data = (uint8*)data;
	task->length = length;
	task->status = statWait;
	task->read = false;
	pool.length++;
	if (pool.length > pool.lengthMax) pool.lengthMax = pool.length;
	return handle;
}

bool SD::clear(int handle)
{
	if (handle >= 0 && handle < POOL_SIZE && pool.length > 0)
	{
		tTask* task = &pool.task[handle];
		if (task->status == statSusses || task->status == statError || task->status == statTimeout || task->status == statWait)
		{
			if (task->timeout != NULL) delete(task->timeout);
			task->status = statEmpty;
			pool.length--;
		}
		return true;
	}
	else
	{
		DBGF_WARNING("SD: no clear %i", handle);
		return false;
	}
}

SD::eStat SD::status(int handle)
{
	if (handle >= 0 && handle < POOL_SIZE)
	{
		return pool.task[handle].status;
	}
	else
	{
		return statError;
	}
}

bool SD::error(int handle)
{
	return
		(
			   !(handle >= 0 && handle < POOL_SIZE)
			|| pool.task[handle].status == statError
			|| pool.task[handle].status == statTimeout
			|| pool.task[handle].status == statEmpty
		);
}

bool SD::busy(int handle)
{
	return
		(
			   handle >= 0 && handle < POOL_SIZE
			&& (pool.task[handle].status == statWait || pool.task[handle].status == statBusy)
		);
}

uint8 SD::getTasks(void)
{
	return pool.length;
}

uint8 SD::getMaximumTasks(void)
{
	return pool.lengthMax;
}

void SD::__irq(void)
{
	if (SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_RXNE) != RESET)
	{
		uint8 data = SPI_ReceiveData8(SPI1);
		sd.recvLength++;
		sd.rxSync = true;
		switch (sd.cmd.s.index)
		{
		case cmdNull:
			break;

		case cmdReadBlockData:
			if (sd.dataLength < 512)
			{
				sd.data[sd.dataLength] = data;
				sd.dataLength++;
			}
			else
			{
				sd.dataLength++;
				if (sd.dataLength >= 514)
				{
					sd.dataLength = 512;
					sd.cmd.s.index = cmdNull;
				}
			}
			break;

		case cmdReadBlock:
			if (sd.response == 0xFF)
			{
				if (sd.recvLength >= 8 && (data & ~SD_RESPONSE_MASK) == 0)
				{
					sd.response = data & SD_RESPONSE_MASK;
					if (sd.response != 0)
					{
						sd.cmd.s.index = cmdNull;
					}
				}
			}
			else
			{
				if (data == SD_DATA_TOKEN)
				{
					sd.cmd.s.index = cmdReadBlockData;
				}
			}
			break;

		case cmdWriteBlockData:
			if (sd.recvLength >= (sd.dataLength + 3))
			{
				sd.cmd.s.index = cmdWriteBlockResponse;
			}
			break;

		case cmdWriteBlock:
			if (sd.response == 0xFF)
			{
				if (sd.recvLength >= 8 && (data & ~SD_RESPONSE_MASK) == 0)
				{
					sd.response = data & SD_RESPONSE_MASK;
					if (sd.response != 0)
					{
						sd.cmd.s.index = cmdNull;
					}
				}
			}
			else
			{
				sd.cmd.s.index = cmdWriteBlockData;
				sd.sendLength = 0;
				sd.recvLength = 0;
			}
			break;

		case cmdWriteBlockResponse:
			if ((data & SD_DATA_RESPONSE_MASK) == SD_DATA_RESPONSE)
			{
				sd.dataResponse = data & SD_DATA_RESPONSE_FLAGS_MASK;
				sd.cmd.s.index = cmdWriteBlockBusy;
			}
			break;

		case cmdWriteBlockBusy:
			if (data == 0xFF)
			{
				sd.cmd.s.index = cmdNull;
			}
			else
			{
				sd.recvLength--;
			}
			break;

		case cmdIdle:
		case cmdAppCmd:
		case cmdAppInit:
		case cmdMMCInit:
		case cmdSetBlock:
			if (sd.recvLength >= 8 && (data & ~SD_RESPONSE_MASK) == 0)
			{
				sd.cmd.s.index = cmdNull;
				sd.response = data & SD_RESPONSE_MASK;
			}
			break;

		case cmdIfCond:
		case cmdReadOCR:
			if (sd.response == 0xFF)
			{
				if (sd.recvLength >= 8 && (data & ~SD_RESPONSE_MASK) == 0)
				{
					sd.response = data & SD_RESPONSE_MASK;
					if ((sd.response & ~SD_RESPONSE_IDLE) != 0)
					{
						sd.cmd.s.index = cmdNull;
					}
				}
			}
			else
			{
				sd.data[sd.dataLength] = data;
				sd.dataLength++;
				if (sd.dataLength >= 4)
				{
					sd.cmd.s.index = cmdNull;
				}
			}
			break;

		case cmdDummy:
			if (sd.recvLength >= 10)
			{
				sd.cmd.s.index = cmdNull;
			}
			break;

		default:
			DBGF_FATAL("SD: spi rx error %02X", sd.cmd.s.index);
			sd.cmd.s.index = cmdNull;
			break;
		}
		if (sd.recvLength > BYTES_FREEZE)
		{
			DBGP_WARNING("SD: detected freeze rx");
			sd.cmd.s.index = cmdNull;
			sd.error = errFreeze;
		}
	}
	if (SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_TXE) != RESET && sd.rxSync)
	{
		sd.rxSync = false;
		switch (sd.cmd.s.index)
		{
		case cmdNull:
			SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE, DISABLE);
			cs(false);
			break;

		case cmdReadBlockData:
		case cmdWriteBlock:
		case cmdReadBlock:
		case cmdIdle:
		case cmdIfCond:
		case cmdAppCmd:
		case cmdAppInit:
		case cmdMMCInit:
		case cmdReadOCR:
		case cmdSetBlock:
			if (sd.sendLength < 7)
			{
				SPI_SendData8(SPI1, sd.cmd.u8[sd.sendLength]);
			}
			else
			{
				SPI_SendData8(SPI1, 0xFF);
			}
			break;

		case cmdWriteBlockData:
		case cmdWriteBlockResponse:
		case cmdWriteBlockBusy:
			if (sd.sendLength == 0)
			{
				SPI_SendData8(SPI1, SD_DATA_TOKEN);
			}
			else if (sd.sendLength < (sd.dataLength + 1))
			{
				SPI_SendData8(SPI1, sd.data[sd.sendLength - 1]);
			}
			else
			{
				sd.sendLength--;
				SPI_SendData8(SPI1, 0xFF);
			}
			break;

		case cmdDummy:
			if (sd.sendLength < 10)
			{
				SPI_SendData8(SPI1, 0xFF);
			}
			break;

		default:
			DBGF_FATAL("SD: spi tx error %02X", sd.cmd.s.index);
			sd.cmd.s.index = cmdNull;
			break;
		}
		sd.sendLength++;
		if (sd.sendLength > (BYTES_FREEZE + 20))
		{
			DBGP_WARNING("SD: detected freeze tx");
			SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE, DISABLE);
			cs(false);
			sd.cmd.s.index = cmdNull;
			sd.error = errFreeze;
		}
	}
}

void SD::Init(void)
{
	reset = true;
	_reset = false;
	ready = false;

	memset(&pool, 0, sizeof(tPool));

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	GPIO_InitTypeDef pin;
	pin.GPIO_Pin	= GPIO_Pin_0;
	pin.GPIO_Mode	= GPIO_Mode_OUT;
	pin.GPIO_Speed	= GPIO_Speed_Level_1;
	pin.GPIO_OType	= GPIO_OType_PP;
	pin.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &pin);

	GPIOB->BSRR |= GPIO_Pin_0;

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_0);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_0);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_0);

	NVIC_EnableIRQ(SPI1_IRQn);
	NVIC_SetPriority(SPI1_IRQn, SD_PRIORITY);

	CALL::setImmediate(Thread);
}

void SD::DeInit(void)
{
	SPI_Cmd(SPI1, DISABLE);
	NVIC_SetPriority(SPI1_IRQn, 0);
	NVIC_DisableIRQ(SPI1_IRQn);

	GPIO_InitTypeDef pin;
	GPIO_StructInit(&pin);
	pin.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &pin);
}

void SD::cs(bool on)
{
	if (on)
	{
		GPIOB->BRR = GPIO_Pin_1;
	}
	else
	{
		GPIOB->BSRR = GPIO_Pin_1;
	}
}

void SD::speed(bool full)
{

	SPI_InitTypeDef spi;
	SPI_Cmd(SPI1, DISABLE);

	if (full)
	{
		spi.SPI_BaudRatePrescaler	= SPI_BaudRatePrescaler_4;
	}
	else
	{
		spi.SPI_BaudRatePrescaler	= SPI_BaudRatePrescaler_256;
	}
	spi.SPI_Direction			= SPI_Direction_2Lines_FullDuplex;
	spi.SPI_Mode				= SPI_Mode_Master;
	spi.SPI_DataSize			= SPI_DataSize_8b;
	spi.SPI_CPOL				= SPI_CPOL_High;
	spi.SPI_CPHA				= SPI_CPHA_2Edge;
	spi.SPI_NSS					= SPI_NSS_Soft;
	spi.SPI_FirstBit			= SPI_FirstBit_MSB;
	spi.SPI_CRCPolynomial		= 0;
	SPI_Init(SPI1, &spi);

	SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_RXNE);
	SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_TXE);

	SPI_Cmd(SPI1, ENABLE);
}

void SD::power(bool on)
{

	GPIO_InitTypeDef pin;

	if (on)
	{
		GPIOB->BRR |= GPIO_Pin_0;
		cs(false);

		pin.GPIO_Pin	= GPIO_Pin_1;
		pin.GPIO_Mode	= GPIO_Mode_OUT;
		pin.GPIO_Speed	= GPIO_Speed_Level_3;
		pin.GPIO_OType	= GPIO_OType_PP;
		pin.GPIO_PuPd	= GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOB, &pin);
		pin.GPIO_Pin	= GPIO_Pin_3 | GPIO_Pin_5;
		pin.GPIO_Mode	= GPIO_Mode_AF;
		GPIO_Init(GPIOB, &pin);
		pin.GPIO_Pin	= GPIO_Pin_4;
		pin.GPIO_PuPd	= GPIO_PuPd_UP;
		GPIO_Init(GPIOB, &pin);

		speed(false);

		SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);

		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
		SPI_SendData8(SPI1, 0xFF);
	}
	else
	{
		SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE, DISABLE);
		SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, DISABLE);

		SPI_Cmd(SPI1, DISABLE);

		pin.GPIO_Pin	= GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_4;
		pin.GPIO_Mode	= GPIO_Mode_IN;
		pin.GPIO_Speed	= GPIO_Speed_Level_1;
		pin.GPIO_OType	= GPIO_OType_PP;
		pin.GPIO_PuPd	= GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOB, &pin);

		GPIOB->BSRR |= GPIO_Pin_0;
	}
}

bool SD::_error(void)
{
	if (sd.error != errSusses)
	{
		switch (sd.error)
		{
		default:
			DBGF_WARNING("SD: error %02X", sd.error);
			break;
		}
		CALL::setImmediate(resetPowerOff);
		return true;
	}
	return false;
}

bool SD::_busy(void)
{
	return (sd.cmd.s.index != cmdNull);
}

void SD::Thread(__UNUSED void* param)
{
	if (reset && !_reset)
	{
		DBGP_NOTICE("SD: reset");
		power(false);
		_reset = true;
		ready = false;
		sd.cmd.s.dummy = 0xFF;
		sd.cmd.s.index = cmdNull;
		sd.highCapacity = true;
		sd.sendLength = 0;
		sd.recvLength = 0;
#ifdef INC_FS_HPP_
		FS::reset();
#endif
		CALL::setTimeout(resetPowerOn, 10);
	}
	if (pool.length != 0)
	{
		tTask* task;
		uint8 tasks = 0;
		do
		{
			task = &pool.task[pool.out];
			if (task->status == statWait || task->status == statBusy) break;
			pool.out++;
			if (pool.out == POOL_SIZE) pool.out = 0;
			tasks++;
		}
		while (tasks < POOL_SIZE);
		if (tasks < POOL_SIZE)
		{
			if (ready)
			{
				if (task->status == statWait)
				{
					task->status = statBusy;
					if (task->read)
					{
						send(cmdReadBlock, task->address);
					}
					else
					{
						send(cmdWriteBlock, task->address, task->data, task->length);
					}
				}
				else if (!_busy())
				{
					if (_error())
					{
						task->status = statError;
					}
					else
					{
						if (task->read)
						{
							if (sd.response == 0)
							{
								if (task->length > sd.dataLength)
								{
									DBGF_WARNING("SD: read >512 %08X", (int)sd.cmd.s.address.u32);
									task->length = sd.dataLength;
								}
								memcpy(task->data, sd.data, task->length);
								task->status = statSusses;
							}
							else
							{
								DBGF_WARNING("SD: read error %02X %08X", sd.response, (int)sd.cmd.s.address.u32);
								task->status = statError;
							}
						}
						else
						{
							if (sd.response == 0 && sd.dataResponse == SD_DATA_RESPONSE_ACCEPT)
							{
								task->status = statSusses;
							}
							else
							{
								DBGF_WARNING("SD: write error %02X %02X %08X", sd.response, sd.dataResponse, (int)sd.cmd.s.address.u32);
								task->status = statError;
							}
						}
					}
				}
			}
			else
			{
				if (task->timeout == NULL)
				{
					pool.out++;
					if (pool.out == POOL_SIZE) pool.out = 0;
				}
				else if (task->timeout->finish())
				{
					task->status = statTimeout;
					DBGF_WARNING("SD: timeout %08X, read %s", (int)task->address, STRBOOL(task->read));
				}
			}
		}
		else
		{
			if (pool.length == POOL_SIZE)
			{
				DBGP_WARNING("SD: tasks full");
				uint8 i;
				for (i = 0; i < POOL_SIZE; i++)
				{
					clear(i);
				}
			}
		}
	}
	CALL::setImmediate(Thread);
}

void SD::resetPowerOff(__UNUSED void* param)
{
	DBGP_NOTICE("SD: off");
	power(false);
	reset = false;
	_reset = false;
	ready = false;
#ifdef INC_FS_HPP_
	FS::reset();
#endif
	CALL::setTimeout(resetRoutine, REINIT_TIMEOUT);
}

void SD::resetRoutine(__UNUSED void* param)
{
	reset = true;
}

void SD::resetPowerOn(__UNUSED void* param)
{
	//DBGP_NOTICE("SD: on");
	power(true);
	CALL::setTimeout(resetSendDummy, FIRST_ON_WAIT);
}

void SD::resetSendDummy(__UNUSED void* param)
{
	send(cmdDummy);
	CALL::setImmediate(resetWaitDummy);
}

void SD::resetWaitDummy(__UNUSED void* param)
{
	if (_busy())
	{
		CALL::setImmediate(resetWaitDummy);
	}
	else if (!_error())
	{
		CALL::setTimeout(resetIdle, 10);
	}
}

void SD::resetIdle(__UNUSED void* param)
{
	send(cmdIdle);
	CALL::setImmediate(resetWaitIdle);
}

void SD::resetWaitIdle(__UNUSED void* param)
{
	if (_busy())
	{
		CALL::setImmediate(resetWaitIdle);
	}
	else if (!_error())
	{
		if (sd.response == SD_RESPONSE_IDLE)
		{
			DBGP_NOTICE("SD: idle");
			speed(true);
			CALL::setImmediate(resetIfCond);
		}
		else
		{
			DBGF_WARNING("SD: idle error %02X", sd.response);
			CALL::setImmediate(resetPowerOff);
		}
	}
}

void SD::resetIfCond(__UNUSED void* param)
{
	send(cmdIfCond, 0x000001AA);
	CALL::setImmediate(resetWaitIfCond);
}

void SD::resetWaitIfCond(__UNUSED void* param)
{
	if (_busy())
	{
		CALL::setImmediate(resetWaitIfCond);
	}
	else if (!_error())
	{
		if (sd.response == SD_RESPONSE_IDLE)
		{
			DBGF_NOTICE("SD: ifcond 0x%02X%02X%02X%02X", sd.data[0], sd.data[1], sd.data[2], sd.data[3]);
			sd.counter = APPINIT_COUNTER;
			CALL::setImmediate((CALL::tCallback)resetAppCmd, (void*)resetAppInit);
		}
		else if (sd.response == (SD_RESPONSE_IDLE | SD_RESPONSE_ERROR_CMD))
		{
			sd.counter = APPINIT_COUNTER;
			CALL::setImmediate(resetMMCInit);
		}
		else
		{
			DBGF_WARNING("SD: ifcond error %02X", sd.response);
			CALL::setImmediate(resetPowerOff);
		}
	}
}

void SD::resetAppCmd(CALL::tCallback appCallback)
{
	send(cmdAppCmd);
	CALL::setImmediate((CALL::tCallback)resetWaitAppCmd, (void*)appCallback);
}

void SD::resetWaitAppCmd(CALL::tCallback appCallback)
{
	if (_busy())
	{
		CALL::setImmediate((CALL::tCallback)resetWaitAppCmd, (void*)appCallback);
	}
	else if (!_error())
	{
		if ((sd.response & ~SD_RESPONSE_IDLE) == 0x00)
		{
			//DBGP_NOTICE("SD: appCmd");
			CALL::setImmediate(appCallback);
		}
		else
		{
			DBGF_WARNING("SD: appCmd error %02X", sd.response);
			CALL::setImmediate(resetPowerOff);
		}
	}
}

void SD::resetAppInit(__UNUSED void* param)
{
	send(cmdAppInit, 0x40000000);
	CALL::setImmediate(resetWaitAppInit);
}

void SD::resetWaitAppInit(__UNUSED void* param)
{
	if (_busy())
	{
		CALL::setImmediate(resetWaitAppInit);
	}
	else if (!_error())
	{
		if (sd.response == 0x00)
		{
			DBGP_NOTICE("SD: appInit");
			CALL::setImmediate(resetReadOCR);
		}
		else if (sd.response == SD_RESPONSE_IDLE)
		{
			//DBGP_NOTICE("SD: appInit no init");
			sd.counter--;
			if (sd.counter == 0)
			{
				CALL::setImmediate(resetPowerOff);
			}
			else
			{
				CALL::setTimeout((CALL::tCallback)resetAppCmd, 100, (void*)resetAppInit);
			}
		}
		else
		{
			DBGF_WARNING("SD: appInit error %02X", sd.response);
			CALL::setImmediate(resetPowerOff);
		}
	}
}

void SD::resetMMCInit(__UNUSED void* param)
{
	send(cmdMMCInit);
	CALL::setImmediate(resetWaitMMCInit);
}

void SD::resetWaitMMCInit(__UNUSED void* param)
{
	if (_busy())
	{
		CALL::setImmediate(resetWaitMMCInit);
	}
	else if (!_error())
	{
		if (sd.response == 0x00)
		{
			DBGP_NOTICE("SD: mmcInit");
			CALL::setImmediate(resetSetBlock);
		}
		else if (sd.response == SD_RESPONSE_IDLE)
		{
			//DBGP_NOTICE("SD: no mmcInit");
			sd.counter--;
			if (sd.counter == 0)
			{
				CALL::setImmediate(resetPowerOff);
			}
			else
			{
				CALL::setTimeout(resetMMCInit, 100);
			}
		}
		else
		{
			DBGF_WARNING("SD: mmcInit error %02X", sd.response);
			CALL::setImmediate(resetPowerOff);
		}
	}
}

void SD::resetReadOCR(__UNUSED void* param)
{
	send(cmdReadOCR);
	CALL::setImmediate(resetWaitReadOCR);
}

void SD::resetWaitReadOCR(__UNUSED void* param)
{
	if (_busy())
	{
		CALL::setImmediate(resetWaitReadOCR);
	}
	else if (!_error())
	{
		if (sd.response == 0x00)
		{

			DBGF_NOTICE("SD: readOCR 0x%02X%02X%02X%02X", sd.data[0], sd.data[1], sd.data[2], sd.data[3]);
			sd.highCapacity = ((sd.data[0] & 0x40) != 0);
			if (sd.highCapacity)
			{
				DBGP_NOTICE("SD: ok [HC]");
				ready = true;
				reset = false;
				_reset = false;
			}
			else
			{
				CALL::setImmediate(resetSetBlock);
			}
		}
		else if (sd.response == SD_RESPONSE_ERROR_CMD)
		{
			sd.highCapacity = false;
			CALL::setImmediate(resetSetBlock);
		}
		else
		{
			DBGF_WARNING("SD: mmcInit error %02X", sd.response);
			CALL::setImmediate(resetPowerOff);
		}
	}
}

void SD::resetSetBlock(__UNUSED void* param)
{
	send(cmdSetBlock, 1);
	CALL::setImmediate(resetWaitSetBlock);
}

void SD::resetWaitSetBlock(__UNUSED void* param)
{
	if (_busy())
	{
		CALL::setImmediate(resetWaitSetBlock);
	}
	else if (!_error())
	{
		if (sd.response == 0x00)
		{
			DBGP_NOTICE("SD: setBlock");
			DBGP_NOTICE("SD: ok [LC]");
			ready = true;
			reset = false;
			_reset = false;
		}
		else
		{
			DBGF_WARNING("SD: setBlock error %02X", sd.response);
			CALL::setImmediate(resetPowerOff);
		}
	}
}

void SD::send(eCmd index, uint32 address, uint8* data, uint16 length)
{
	sd.cmd.s.index = index;
	if (!sd.highCapacity) address *= 512;
	sd.cmd.s.address.u8[3] = address & 0xFF;
	sd.cmd.s.address.u8[2] = (address >> 8) & 0xFF;
	sd.cmd.s.address.u8[1] = (address >> 16) & 0xFF;
	sd.cmd.s.address.u8[0] = (address >> 24) & 0xFF;
	switch (index)
	{
	case cmdIdle:
		sd.cmd.s.crc = 0x95;
		break;
	case cmdIfCond:
		sd.cmd.s.crc = 0x87;
		break;
	default:
		sd.cmd.s.crc = 0xFF;
		break;
	}
	if (data != NULL)
	{
		if (length > 512)
		{
			DBGF_WARNING("SD: write >512 %08X", (int)sd.cmd.s.address.u32);
			length = 512;
		}
		memcpy(sd.data, data, length);
		if (length < 512)
		{
			memset(&sd.data[length], 0, 512 - length);
		}
		sd.dataLength = 512;
	}
	else
	{
		sd.dataLength = 0;
	}
	sd.rxSync = true;
	sd.sendLength = 0;
	sd.recvLength = 0;
	sd.response = 0xFF;
	sd.dataResponse = 0xFF;
	sd.error = errSusses;
	if (index >= 0x40) cs(true);

	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE, ENABLE);
}

#endif
