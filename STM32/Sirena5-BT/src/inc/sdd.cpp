/**
 * HiSER (c)2020 Litvin Artem Vasilyevich
 * Date 16 июл. 2020 г.
 * sirenabt5@artemlitvin.com
 */

#include "sdd.hpp"

SDD::tPool SDD::pool;
SDD::tVars SDD::vars;

void SDD::Init()
{
	vars.sd.reset = true;
	vars.sd._reset = false;
	vars.sd.ready = false;
	vars.sd.cmd.dummy = 0xFF;
	InitSPI();
	vars.handleThread = CALL::setImmediate(Thread);
}

void SDD::DeInit()
{
	CALL::clear(vars.handleThread);
	DeInitSPI();
}

void SDD::Thread(void* param __UNUSED)
{
	if (vars.sd.reset && !vars.sd._reset)
	{
		//DBGP("SDD: power on (wait)");
		vars.sd._reset = true;
		vars.sd.ready = false;
		vars.sd.highCapacity = true;
#ifdef INC_FS_HPP_
		FS::reset();
#endif
		power(true);
		CALL::setTimeout(powerOn, PowerOnWait);
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
			if (pool.out == PoolCount) pool.out = 0;
			tasks++;
		}
		while (tasks < PoolCount);
		if (tasks < PoolCount)
		{
			if (vars.sd.ready)
			{
				if (task->status == statWait)
				{
					task->status = statBusy;
					if (task->read)
					{
						send(cmdReadBlock, (CALL::tCallback)readCallback, task->address, task);
					}
					else
					{
						send(cmdWriteBlock, (CALL::tCallback)writeCallback, task->address, task);
					}
				}
			}
			else
			{
				if (task->timeout == NULL)
				{
					pool.out++;
					if (pool.out == PoolCount) pool.out = 0;
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
			if (pool.length == PoolCount)
			{
				DBGP_WARNING("SD: tasks full");
				uint8 i;
				for (i = 0; i < PoolCount; i++)
				{
					clear(i);
				}
			}
		}
	}
	vars.handleThread = CALL::setImmediate(Thread);
}

void SDD::readCallback(uint8* buffer)
{
	if (buffer == NULL)
	{
		vars.sd.task->status = statError;
	}
	else
	{
		memcpy(vars.sd.task->data, buffer, vars.sd.task->length);
		vars.sd.task->status = statSusses;
	}
}

void SDD::writeCallback(uint8* buffer)
{
	if (buffer == NULL)
	{
		vars.sd.task->status = statError;
	}
	else
	{
		vars.sd.task->status = statSusses;
	}
}

/* ---------------------------------------------------- */

int SDD::read(uint32 address, void* data, uint16 length, int timeout)
{
	if (data == NULL) return -1;
	tTask* task;
	int handle;
	if (pool.length == PoolCount)
	{
		DBGP_WARNING("SD: read overflow");
		return -1;
	}
	do
	{
		task = &pool.task[pool.in];
		handle = pool.in;
		pool.in++;
		if (pool.in == PoolCount) pool.in = 0;
	}
	while (task->status != statEmpty);
	if (length > 512)
	{
		DBGF("SSD: read >512 %u", length);
		length = 512;
	}
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

int SDD::write(uint32 address, const void* data, uint16 length, int timeout)
{
	if (data == NULL) return -1;
	tTask* task;
	int handle;
	if (pool.length == PoolCount)
	{
		DBGP_WARNING("SD: write overflow");
		return -1;
	}
	do
	{
		task = &pool.task[pool.in];
		handle = pool.in;
		pool.in++;
		if (pool.in == PoolCount) pool.in = 0;
	}
	while (task->status != statEmpty);
	if (length > 512)
	{
		DBGF("SSD: write >512 %u", length);
		length = 512;
	}
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

bool SDD::clear(int handle)
{
	if (handle >= 0 && handle < PoolCount && pool.length > 0)
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

SDD::eStat SDD::status(int handle)
{
	if (handle >= 0 && handle < PoolCount)
	{
		return pool.task[handle].status;
	}
	else
	{
		return statError;
	}
}

bool SDD::error(int handle)
{
	return
		(
			   !(handle >= 0 && handle < PoolCount)
			|| pool.task[handle].status == statError
			|| pool.task[handle].status == statTimeout
			|| pool.task[handle].status == statEmpty
		);
}

bool SDD::busy(int handle)
{
	return
		(
			   handle >= 0 && handle < PoolCount
			&& (pool.task[handle].status == statWait || pool.task[handle].status == statBusy)
		);
}

uint8 SDD::getTasks(void)
{
	return pool.length;
}

uint8 SDD::getMaximumTasks(void)
{
	return pool.lengthMax;
}

/* ---------------------------------------------------- */

void SDD::powerReset()
{
	if (!vars.sd.reset)
	{
		//DBGP("SDD: reset (wait)");
		vars.sd.reset = true;
		vars.sd._reset = true;
		vars.sd.ready = false;
		power(false);
		CALL::setTimeout(_powerReset, ResetWait);
	}
}

void SDD::_powerReset(void* param __UNUSED)
{
	DBGP("SDD: reset");
	vars.sd._reset = false;
}

void SDD::powerOn(void* param __UNUSED)
{
	DBGP("SDD: power on");
	vars.sd.reset = false;
	vars.sd._reset = false;
	//DBGP("SDD: dummy");
	cs(true);
	ReadSPI(DummyFirst);
	CALL::setImmediate(waitDummy);
}

void SDD::waitDummy(void* param __UNUSED)
{
	if (ReadBusySPI())
	{
		CALL::setImmediate(waitDummy);
	}
	else
	{
		//DBGP("SDD: dummy end");
		send(cmdIdle, Idle);
	}
}

void SDD::Idle(void* param __UNUSED)
{
	if (vars.sd.response == SD_RESPONSE_IDLE)
	{
		DBGP("SDD: Idle ok");
		send(cmdIfCond, (CALL::tCallback)IfCond, 0x000001AA);
	}
	else
	{
		DBGF("SDD: Idle error response %02X", vars.sd.response);
		powerReset();
	}
}

void SDD::IfCond(uint8* cond)
{
	if (vars.sd.response == SD_RESPONSE_IDLE)
	{
		DBGF("SDD: IfCond %02X%02X%02X%02X", cond[0], cond[1], cond[2], cond[3]);
		vars.sd.counter = 0;
		AppCmdInit();
	}
	else if (vars.sd.response == (SD_RESPONSE_IDLE | SD_RESPONSE_ERROR_CMD))
	{
		DBGP("SDD: IfCond error");
		vars.sd.counter = 0;
		MMCInit();
	}
	else
	{
		DBGF("SDD: IfCond error response %02X", vars.sd.response);
		powerReset();
	}
}

void SDD::AppCmdInit(void* param __UNUSED)
{
	vars.sd.counter++;
	send(cmdAppCmd, AppInit);
}

void SDD::AppInit(void* param __UNUSED)
{
	if (vars.sd.response == SD_RESPONSE_IDLE || vars.sd.response == 0)
	{
		send(cmdAppInit, _AppInit, 0x40000000);
	}
	else
	{
		DBGF("SDD: APP Cmd error response %02X", vars.sd.response);
		powerReset();
	}
}

void SDD::_AppInit(void* param __UNUSED)
{
	if (vars.sd.response == 0)
	{
		DBGP("SDD: APP init");
		send(cmdReadOCR, (CALL::tCallback)ReadOCR);
	}
	else if (vars.sd.counter < APPInitCount)
	{
		CALL::setTimeout(AppCmdInit, InitWait);
	}
	else
	{
		DBGF("SDD: APP init error response %02X", vars.sd.response);
		powerReset();
		//Idle(NULL);
	}
}

void SDD::ReadOCR(uint8* ocr)
{
	if (vars.sd.response == 0)
	{
		DBGF("SDD: ReadOCR %02X%02X%02X%02X", ocr[0], ocr[1], ocr[2], ocr[3]);
		if ((ocr[0] & 0x40) != 0)
		{
			DBGP("SDD: SD init [HC]");
			setFullSpeed();
		}
		else
		{
			DBGP("SDD: SD init [SC]");
			SetBlock();
		}
	}
	else
	{
		DBGF("SDD: ReadOCR error response %02X", vars.sd.response);
		powerReset();
	}
}

void SDD::MMCInit(void* param __UNUSED)
{
	vars.sd.counter++;
	send(cmdMMCInit, _MMCInit);
}

void SDD::_MMCInit(void* param __UNUSED)
{
	if (vars.sd.response == 0)
	{
		DBGP("SDD: MMC init");
		send(cmdReadOCR, (CALL::tCallback)ReadOCR);
		//SetBlock();
	}
	else if (vars.sd.counter < MMCInitCount)
	{
		CALL::setTimeout(MMCInit, InitWait);
	}
	else
	{
		DBGF("SDD: MMC init error response %02X", vars.sd.response);
		powerReset();
	}
}

void SDD::SetBlock(void* param __UNUSED)
{
	send(cmdSetBlock, _SetBlock, 512);
}

void SDD::_SetBlock(void* param __UNUSED)
{
	if (vars.sd.response == 0)
	{
		DBGP("SDD: SetBlock ok");
		vars.sd.highCapacity = false;
		setFullSpeed();
	}
	else
	{
		DBGF("SDD: SetBlock error response %02X", vars.sd.response);
		powerReset();
	}
}

void SDD::setFullSpeed(void* param __UNUSED)
{
	speed(true);
	ReadSPI(DummySpeed);
	CALL::setImmediate(_setFullSpeed);
}

void SDD::_setFullSpeed(void* param __UNUSED)
{
	if (ReadBusySPI())
	{
		CALL::setImmediate(_setFullSpeed);
	}
	else
	{
		vars.sd.reset = false;
		vars.sd._reset = false;
		vars.sd.ready = true;
	}
}

void SDD::send(eCMD cmd, CALL::tCallback callback, uint32 address, tTask* task)
{
	vars.sd.callback = callback;
	vars.sd.task = task;
	vars.sd.response = 0xFF;
	vars.sd.cmd.index = cmd;
	if (!vars.sd.highCapacity) address *= 512;
	vars.sd.cmd.address.u8[3] = address & 0xFF;
	vars.sd.cmd.address.u8[2] = (address >> 8) & 0xFF;
	vars.sd.cmd.address.u8[1] = (address >> 16) & 0xFF;
	vars.sd.cmd.address.u8[0] = (address >> 24) & 0xFF;
	switch (cmd)
	{
	case cmdIdle:
		vars.sd.cmd.crc = 0x95;
		break;
	case cmdIfCond:
		vars.sd.cmd.crc = 0x87;
		break;
	default:
		vars.sd.cmd.crc = 0xFF;
		break;
	}
	//DBGF("SDD: send %02X %02X %08X %02X", vars.sd.cmd.dummy, vars.sd.cmd.index, (int)vars.sd.cmd.address.u32, vars.sd.cmd.crc);
	cs(true);
	WriteSPI(&vars.sd.cmd, sizeof(tCMD));
	CALL::setImmediate(_send);
}

void SDD::_send(void* param __UNUSED)
{
	if (WriteBusySPI())
	{
		CALL::setImmediate(_send);
	}
	else
	{
		response();
	}
}

void SDD::response(void* param __UNUSED)
{
	//DBGP("SDD: response (wait)")
	switch (vars.sd.cmd.index)
	{
		case cmdReadBlock:
			ReadSPI(DataBlockLength);
			break;
		case cmdWriteBlock:
		case cmdIdle:
		case cmdMMCInit:
		case cmdSetBlock:
		case cmdAppCmd:
		case cmdAppInit:
		case cmdIfCond:
		case cmdReadOCR:
			ReadSPI(ResponseLength);
			break;
	}
	CALL::setImmediate(_response);
}

void SDD::_response(void* param __UNUSED)
{
	if (ReadBusySPI())
	{
		CALL::setImmediate(_response);
	}
	else
	{
		//DBGP("SDD: response");
		///if (vars.sd.cmd.index == cmdReadBlock || vars.sd.cmd.index == cmdWriteBlock)
		//{
		//	DBG::printBuff("SDD:", vars.spi.buffer, vars.spi.length);
		//}
		uint16 i = 0;
		bool resp = false;
		bool data = false;
		while (i < vars.spi.length)
		{
			if ((vars.spi.buffer[i] & ~SD_RESPONSE_MASK) == 0)
			{
				vars.spi.start = i + 1;
				vars.sd.response = vars.spi.buffer[i] & SD_RESPONSE_MASK;
				resp = true;
				//DBGF("SDD: response %02X", vars.sd.response);
				break;
			}
			i++;
		}
		if (resp)
		{
			if (vars.sd.cmd.index == cmdReadBlock)
			{
				if (vars.sd.response == 0)
				{
					i = vars.spi.start;
					while (i < vars.spi.length)
					{
						if (vars.spi.buffer[i] == SD_DATA_TOKEN)
						{
							vars.spi.start = i + 1;
							data = true;
							//DBGF("SDD: response %02X", vars.sd.response);
							break;
						}
						i++;
					}
					if (data && (vars.spi.length - vars.spi.start) >= 512)
					{
						//DBGF("SDD: read token %u, %u", vars.spi.start, vars.spi.length);
						CALL::setImmediate(vars.sd.callback, &vars.spi.buffer[vars.spi.start]);
					}
					else
					{
						DBGF("SDD: read token error %u, %u", vars.spi.start, vars.spi.length);
						CALL::setImmediate(vars.sd.callback, NULL);
					}
				}
				else
				{
					DBGF("SDD: read error %02X %08X", vars.sd.response, (int)vars.sd.cmd.address.u32);
					CALL::setImmediate(vars.sd.callback, NULL);
					powerReset();
				}
			}
			else if (vars.sd.cmd.index == cmdWriteBlock)
			{
				if (vars.sd.response == 0)
				{
					vars.spi.buffer[0] = 0xFF;
					vars.spi.buffer[1] = SD_DATA_TOKEN;
					memcpy(&vars.spi.buffer[2], vars.sd.task->data, vars.sd.task->length);
					if (vars.sd.task->length < 512)
					{
						memset(&vars.spi.buffer[vars.sd.task->length + 2], 0, 512 - vars.sd.task->length);
					}
					vars.spi.buffer[514] = 0xFF;
					vars.spi.buffer[515] = 0xFF;
					WriteSPI(NULL, 2 + 512 + 2);
					CALL::setImmediate(sendWrite);
				}
				else
				{
					DBGF("SDD: write error %02X %08X", vars.sd.response, (int)vars.sd.cmd.address.u32);
					CALL::setImmediate(vars.sd.callback, NULL);
					powerReset();
				}
			}
			else
			{
				CALL::setImmediate(vars.sd.callback, &vars.spi.buffer[vars.spi.start]);
			}
		}
		else
		{
			DBGP("SDD: response error");
			powerReset();
		}
	}
}

void SDD::sendWrite(void* param __UNUSED)
{
	if (WriteBusySPI())
	{
		CALL::setImmediate(sendWrite);
	}
	else
	{
		ReadSPI(ResponseLength);
		CALL::setImmediate(writeResponse);
	}
}

void SDD::writeResponse(void* param __UNUSED)
{
	if (ReadBusySPI())
	{
		CALL::setImmediate(writeResponse);
	}
	else
	{
		//DBGP("SDD: data response");
		//DBG::printBuff("SDD:", vars.spi.buffer, vars.spi.length);
		uint16 i = 0;
		bool resp = false;
		while (i < vars.spi.length)
		{
			if ((vars.spi.buffer[i] & SD_DATA_RESPONSE_MASK) == SD_DATA_RESPONSE)
			{
				vars.spi.start = i + 1;
				vars.sd.response = vars.spi.buffer[i] & SD_DATA_RESPONSE_FLAGS_MASK;
				resp = true;
				//DBGF("SDD: data response %02X", vars.sd.response);
				break;
			}
			i++;
		}
		if (resp)
		{
			if (vars.sd.response == SD_DATA_RESPONSE_ACCEPT)
			{
				vars.sd.counter = 0;
				waitBusy();
			}
			else
			{
				DBGF("SDD: data token error %02X", vars.sd.response);
				CALL::setImmediate(vars.sd.callback, NULL);
			}
		}
		else
		{
			DBGP("SDD: data response error");
			CALL::setImmediate(vars.sd.callback, NULL);
			powerReset();
		}
	}
}

void SDD::waitBusy(void* param __UNUSED)
{
	//DBGP("SDD: wait busy");
	vars.sd.counter++;
	ReadSPI(ResponseLength);
	CALL::setImmediate(_waitBusy);
}

void SDD::_waitBusy(void* param __UNUSED)
{
	if (ReadBusySPI())
	{
		CALL::setImmediate(_waitBusy);
	}
	else
	{
		uint16 i = 0;
		bool busy = true;
		while (i < vars.spi.length)
		{
			if (vars.spi.buffer[i]  == 0xFF)
			{
				busy = false;
				break;
			}
			i++;
		}
		if (busy)
		{
			if (vars.sd.counter < BusyCount)
			{
				CALL::setTimeout(waitBusy, BusyWait);
			}
			else
			{
				DBGP("SDD: wait busy freeze");
				CALL::setImmediate(vars.sd.callback, NULL);
			}
		}
		else
		{
			vars.spi.start = 0;
			vars.spi.length = 0;
			CALL::setImmediate(vars.sd.callback, vars.spi.buffer);
		}
	}
}

/* ---------------------------------------------------- */

void SDD::InitSPI()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	SPI_I2S_DeInit(SPI1);
	DMA_DeInit(DMA1_Channel2);
	DMA_DeInit(DMA1_Channel3);

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
}

void SDD::DeInitSPI()
{
	power(false);
	DMA_Cmd(DMA1_Channel2, DISABLE);
	DMA_Cmd(DMA1_Channel3, DISABLE);
	GPIO_InitTypeDef pin;
	GPIO_StructInit(&pin);
	pin.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &pin);
}

void SDD::cs(bool on)
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

void SDD::power(bool on)
{
	GPIO_InitTypeDef pin;

	if (on)
	{
		vars.spi.routine = false;

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

		DMA_ClearFlag(DMA1_FLAG_TC2);
		DMA_ClearFlag(DMA1_FLAG_TC3);

		SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
		SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);

		speed(false);
	}
	else
	{
		SPI_Cmd(SPI1, DISABLE);
		SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
		SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);
		DMA_Cmd(DMA1_Channel2, DISABLE);
		DMA_Cmd(DMA1_Channel3, DISABLE);

		pin.GPIO_Pin	= GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_4;
		pin.GPIO_Mode	= GPIO_Mode_IN;
		pin.GPIO_Speed	= GPIO_Speed_Level_1;
		pin.GPIO_OType	= GPIO_OType_PP;
		pin.GPIO_PuPd	= GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOB, &pin);

		GPIOB->BSRR |= GPIO_Pin_0;
	}
}

void SDD::speed(bool full)
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

	SPI_Cmd(SPI1, ENABLE);
}

void SDD::ReadSPI(uint16 length)
{
	if (vars.spi.routine)
	{
		DBGP("SDD: ReadSPI collision");
		return;
	}
	vars.spi.routine = true;
	vars.spi.start = 0;
	vars.spi.length = length;
	static uint8 dummy = 0xFF;
	DMA_InitTypeDef dma;
	DMA_StructInit(&dma);
	dma.DMA_PeripheralBaseAddr		= (uint32)&SPI1->DR;
	dma.DMA_MemoryBaseAddr			= (uint32)&vars.spi.buffer[0];
	dma.DMA_BufferSize				= length;
	dma.DMA_MemoryInc				= DMA_MemoryInc_Enable;
	DMA_Init(DMA1_Channel2, &dma);
	DMA_StructInit(&dma);
	dma.DMA_PeripheralBaseAddr		= (uint32)&SPI1->DR;
	dma.DMA_DIR						= DMA_DIR_PeripheralDST;
	dma.DMA_MemoryBaseAddr			= (uint32)&dummy;
	dma.DMA_BufferSize				= length + 1;
	DMA_Init(DMA1_Channel3, &dma);
	DMA_Cmd(DMA1_Channel2, ENABLE);
	DMA_Cmd(DMA1_Channel3, ENABLE);
}

void SDD::WriteSPI(void* data, uint16 length)
{
	if (vars.spi.routine)
	{
		DBGP("SDD: WriteSPI collision");
		return;
	}
	vars.spi.routine = true;
	vars.spi.start = 0;
	vars.spi.length = length;
	if (data != NULL) memcpy(vars.spi.buffer, data, length);
	DMA_InitTypeDef dma;
	DMA_StructInit(&dma);
	dma.DMA_PeripheralBaseAddr		= (uint32)&SPI1->DR;
	dma.DMA_DIR						= DMA_DIR_PeripheralDST;
	dma.DMA_MemoryBaseAddr			= (uint32)&vars.spi.buffer[0];
	dma.DMA_BufferSize				= length;
	dma.DMA_MemoryInc				= DMA_MemoryInc_Enable;
	DMA_Init(DMA1_Channel3, &dma);
	DMA_Cmd(DMA1_Channel3, ENABLE);
}

bool SDD::ReadBusySPI()
{
	if (DMA_GetFlagStatus(DMA1_FLAG_TC2) != RESET && DMA_GetFlagStatus(DMA1_FLAG_TC3) != RESET)
	{
		DMA_ClearFlag(DMA1_FLAG_TC2);
		DMA_ClearFlag(DMA1_FLAG_TC3);
		DMA_Cmd(DMA1_Channel2, DISABLE);
		DMA_Cmd(DMA1_Channel3, DISABLE);
		vars.spi.routine = false;
	}
	return vars.spi.routine;
}

bool SDD::WriteBusySPI()
{
	if (DMA_GetFlagStatus(DMA1_FLAG_TC3) != RESET)
	{
		DMA_ClearFlag(DMA1_FLAG_TC3);
		DMA_Cmd(DMA1_Channel3, DISABLE);
		vars.spi.routine = false;
	}
	return vars.spi.routine;
}

/* ---------------------------------------------------- */
