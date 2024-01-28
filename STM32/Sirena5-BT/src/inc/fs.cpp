/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 13 мар. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <fs.hpp>

FS::tMelodyTable FS::melodyTable;
FS::tEventTable FS::eventTable;
FS::tStreamWrite* FS::streamWrite;
FS::tSaveTables* FS::_saveTables;
bool FS::readyMelodyTable;
bool FS::readyEventTable;
bool FS::errorMelodyTable;
bool FS::errorEventTable;
bool FS::_ready;
bool FS::_reset;

void FS::Init(void)
{
	DBGF("FS: tables - melodys %u, events %u", sizeof(tMelodyTable), sizeof(tEventTable));
	_reset = false;
	streamWrite = NULL;
	_saveTables = NULL;
	reset();
}

void FS::DeInit(void)
{

}

void FS::readMelodyTable(int handle)
{
	if (SD::busy(handle))
	{
		CALL::setImmediate((CALL::tCallback)readMelodyTable, (void*)handle);
	}
	else
	{
		if (SD::error(handle))
		{
			DBGP_WARNING("FS: error read melodyTable");
			errorMelodyTable = true;
			ready();
		}
		else
		{
			if (melodyTableCRC())
			{
				DBGP_NOTICE("FS: melodyTable loaded");
			}
			else
			{
				DBGP_WARNING("FS: melodyTable failure");
				melodyTableClear();
			}
			readyMelodyTable = true;
			ready();
		}
		SD::clear(handle);
	}
}

void FS::readEventTable(int handle)
{
	if (SD::busy(handle))
	{
		CALL::setImmediate((CALL::tCallback)readEventTable, (void*)handle);
	}
	else
	{
		if (SD::error(handle))
		{
			DBGP_WARNING("FS: error read eventTable");
			errorEventTable = true;
			ready();
		}
		else
		{
			if (eventTableCRC())
			{
				DBGP_NOTICE("FS: eventTable loaded");
			}
			else
			{
				DBGP_WARNING("FS: eventTable failure");
				eventTableClear();
			}
			readyEventTable = true;
			ready();
		}
		SD::clear(handle);
	}
}

void FS::melodyTableClear(void)
{
	uint8 i;
	for (i = 0; i < MELODY_MAX; i++)
	{
		deleteMelody(i);
	}
}

void FS::eventTableClear(void)
{
	uint8 i;
	for (i = 0; i < EVENT_MAX; i++)
	{
		deleteEvent(i);
	}
}

bool FS::melodyTableCRC(void)
{
	uint16 crc = melodyTable.crc;
	melodyTable.crc = CRC16::calc(&melodyTable, sizeof(tMelodyTable) - 2);
	return (melodyTable.crc == crc);
}

bool FS::eventTableCRC(void)
{
	uint16 crc = eventTable.crc;
	eventTable.crc = CRC16::calc(&eventTable, sizeof(tEventTable) - 2);
	return (eventTable.crc == crc);
}

void FS::ready(void)
{
	if (readyMelodyTable && readyEventTable)
	{
		_ready = true;
		_reset = false;
		DBGP_NOTICE("FS: ok");
		//BT::send("+FS:READY");
	}
	else if ((errorEventTable || readyEventTable) && (errorMelodyTable || readyMelodyTable))
	{
		_reset = false;
		DBGP_NOTICE("FS: error");
		FS::reset();
	}
}

void FS::reset(void)
{
	if (_reset) return;
	_reset = true;
	readyMelodyTable = false;
	readyEventTable = false;
	errorMelodyTable = false;
	errorEventTable = false;
	_ready = false;
	endMelody();
	melodyTableClear();
	eventTableClear();
	DBGP_NOTICE("FS: reset");
	BT::send("+FS:BUSY");
	CALL::setImmediate((CALL::tCallback)readMelodyTable, (void*)SD::read(BLOCK_MELODY_TABLE, &melodyTable, sizeof(tMelodyTable)));
	CALL::setImmediate((CALL::tCallback)readEventTable, (void*)SD::read(BLOCK_EVENT_TABLE, &eventTable, sizeof(tEventTable)));
}

void FS::findMelodyRoutine(tFindMelody* find)
{
	find->handle = -1;
	if (find->timeout->finish())
	{
		find->callback(find);
	}
	else
	{
		if (!_ready)
		{
			find->handle = CALL::setTimeout((CALL::tCallback)findMelodyRoutine, 10, find);
		}
		else
		{
			uint8 i, n;
			for (i = 0; i < EVENT_MAX; i++)
			{
				if
				(
					   eventTable.event[i].chEvent.lengthType != PIN::eptNull
					&& eventTable.event[i].chEvent.index == find->event.index
					&& eventTable.event[i].chEvent.lengthType == find->event.lengthType
				)
				{
					if (find->event.lengthType > 0)
					{
						for (n = 0; n < find->event.lengthType; n++)
						{
							uint8 d = CH::deviation(find->event.index);
							if
							(
								!(
									   eventTable.event[i].chEvent.pulseType[n].count == find->event.pulseType[n].count
									&& eventTable.event[i].chEvent.pulseType[n].width >= (find->event.pulseType[n].width - d)
									&& eventTable.event[i].chEvent.pulseType[n].width <= (find->event.pulseType[n].width + d)
								)
							)
							{
								break;
							}
						}
						if (n == find->event.lengthType)
						{
							find->melodyIndex = eventTable.event[i].melodyIndex;
							find->poweroff = eventTable.event[i].poweroff;
							find->volume = eventTable.event[i].volume;
							find->noPlayAfter = eventTable.event[i].noPlayAfter;
							find->eventId = i;
							break;
						}
					}
					else
					{
						find->melodyIndex = eventTable.event[i].melodyIndex;
						find->poweroff = eventTable.event[i].poweroff;
						find->volume = eventTable.event[i].volume;
						find->noPlayAfter = eventTable.event[i].noPlayAfter;
						find->eventId = i;
						break;
					}
				}
			}
			if (find->melodyIndex >= 0 && find->melodyIndex < MELODY_MAX && melodyTable.melody[find->melodyIndex].address != 0)
			{
				memcpy(&find->melody, &melodyTable.melody[find->melodyIndex], sizeof(tMelody));
			}
			find->callback(find);
		}
	}
}

FS::tFindMelody* FS::findMelodyCreate(CH::tEvent* event, tCBFind callback)
{
	tFindMelody* find = (tFindMelody*)malloc(sizeof(tFindMelody));
	find->timeout = new TIMER(FIND_TIMEOUT);
	find->callback = (CALL::tCallback)callback;
	find->melody.address = 0;
	find->melodyIndex = -1;
	find->poweroff = true;
	find->volume = AMP::cfgVolume;
	find->noPlayAfter = -1;
	find->eventId = -1;
	memcpy(&find->event, event, sizeof(CH::tEvent));
	find->handle = CALL::setImmediate((CALL::tCallback)findMelodyRoutine, find);
	return find;
}

void FS::findMelodyRemove(tFindMelody* find)
{
	if (find != NULL)
	{
		delete(find->timeout);
		if (find->handle >= 0) CALL::clear(find->handle);
		free(find);
	}
}

void FS::preMelodyRoutine(tFindMelody* find)
{
	find->handle = -1;
	if (find->timeout->finish())
	{
		find->callback(find);
	}
	else
	{
		if (!_ready)
		{
			find->handle = CALL::setTimeout((CALL::tCallback)preMelodyRoutine, 10, find);
		}
		else
		{
			find->melodyIndex = find->melody.length;
			if (find->melodyIndex >= 0 && find->melodyIndex < MELODY_MAX && melodyTable.melody[find->melodyIndex].address != 0)
			{
				memcpy(&find->melody, &melodyTable.melody[find->melodyIndex], sizeof(tMelody));
			}
			find->callback(find);
		}
	}
}

FS::tFindMelody* FS::preMelodyCreate(uint8 index, tCBFind callback)
{
	tFindMelody* find = (tFindMelody*)malloc(sizeof(tFindMelody));
	find->timeout = new TIMER(FIND_TIMEOUT);
	find->callback = (CALL::tCallback)callback;
	find->melodyIndex = -1;
	find->poweroff = false;
	find->melody.address = 0;
	find->melody.length = index;
	find->event.index = CH::CHANNELS + 1;
	find->event.lengthType = (PIN::ePulseType)1;
	find->event.pulseType[0].width = 100;
	find->event.pulseType[0].count = 1;
	find->event.playOnce = false;
	find->event.playToEnd = false;
	find->volume = AMP::cfgVolume;
	find->noPlayAfter = -1;
	find->eventId = -1;
	find->handle = CALL::setImmediate((CALL::tCallback)preMelodyRoutine, find);
	return find;
}

FS::tStreamRead* FS::open(uint32 address, uint32 length, tCBOpen callback, int timeout, void* tag)
{
	tStreamRead* stream = (tStreamRead*)malloc(sizeof(tStreamRead));
	stream->address = address;
	stream->length = length;
	stream->readed = 0;
	stream->callback = (CALL::tCallback)callback;
	stream->timeout = timeout;
	stream->callHandle = -1;
	stream->sdHandle = -1;
	stream->tag = tag;
	next(stream);
	return stream;
}

void FS::next(tStreamRead* stream)
{
	if (stream != NULL && stream->callHandle < 0 && stream->sdHandle < 0)
	{
		stream->callHandle = CALL::setImmediate((CALL::tCallback)_read, stream);
	}
}

void FS::close(tStreamRead* stream)
{
	if (stream != NULL)
	{
		if (stream->callHandle >= 0) CALL::clear(stream->callHandle);
		if (stream->sdHandle >= 0) SD::clear(stream->sdHandle);
		free(stream);
	}
}

void FS::_read(tStreamRead* stream)
{
	stream->callHandle = -1;
	if (stream->sdHandle < 0)
	{
		stream->data.length = 0;
		stream->data.offset = 0;
		stream->data.error = false;
		stream->data.eof = false;
		stream->sdHandle = SD::read(stream->address, &stream->data.buffer[0], 512, stream->timeout);
		if (stream->sdHandle < 0)
		{
			stream->data.eof = true;
			stream->data.error = true;
			CALL::setImmediate(stream->callback, stream);
		}
		else
		{
#if FS_WAIT_BUSY == 0
			stream->callHandle = CALL::setImmediate((CALL::tCallback)_read, stream);
#else
			stream->callHandle = CALL::setTimeout((CALL::tCallback)_read, FS_WAIT_BUSY, stream);
#endif
		}
	}
	else
	{
		if (SD::busy(stream->sdHandle))
		{
#if FS_WAIT_BUSY == 0
			stream->callHandle = CALL::setImmediate((CALL::tCallback)_read, stream);
#else
			stream->callHandle = CALL::setTimeout((CALL::tCallback)_read, FS_WAIT_BUSY, stream);
#endif
		}
		else
		{
			if (SD::error(stream->sdHandle))
			{
				stream->data.eof = true;
				stream->data.error = true;
				CALL::setImmediate(stream->callback, stream);
			}
			else
			{
				stream->address++;
				stream->data.length = stream->length - stream->readed;
				if (stream->data.length > 512) stream->data.length = 512;
				stream->readed += stream->data.length;
				if (stream->readed >= stream->length) stream->data.eof = true;
				CALL::setImmediate(stream->callback, stream);
			}
			SD::clear(stream->sdHandle);
			stream->sdHandle = -1;
		}
	}
}

bool FS::hasReady(void)
{
	return _ready;
}

FS::tList* FS::list(void)
{
	if (!_ready) return NULL;
	tList* result = (tList*)malloc(sizeof(tList));
	uint8 i;
	result->melody.count = 0;
	result->event.count = 0;
	for (i = 0; i < MELODY_MAX; i++)
	{
		if (melodyTable.melody[i].address != 0)
		{
			result->melody.index[result->melody.count] = i;
			result->melody.count++;
		}
	}
	for (i = 0; i < EVENT_MAX; i++)
	{
		if (eventTable.event[i].chEvent.lengthType != PIN::eptNull)
		{
			result->event.index[result->event.count] = i;
			result->event.count++;
		}
	}
	return result;
}

FS::tEvent* FS::getEvent(uint8 index)
{
	tEvent* result = (tEvent*)malloc(sizeof(tEvent));
	if (index >= EVENT_MAX || eventTable.event[index].chEvent.lengthType == PIN::eptNull)
	{
		result->chEvent.lengthType = PIN::eptNull;
	}
	else
	{
		memcpy(result, &eventTable.event[index], sizeof(tEvent));
	}
	return result;
}

bool FS::setEvent(uint8 index, tEvent* event)
{
	if (index >= EVENT_MAX) return false;
	memcpy(&eventTable.event[index], event , sizeof(tEvent));
	return true;
}

void FS::getMelody(uint8 index, tCBMelody2 callback)
{
	tMelody2Routine* routine = (tMelody2Routine*)malloc(sizeof(tMelody2Routine));
	routine->melody = (tMelody2*)malloc(sizeof(tMelody2));
	routine->melody->name = NULL;
	routine->melody->index = index;
	routine->callback = callback;
	routine->handle = -1;
	if (index < MELODY_MAX && melodyTable.melody[index].address != 0)
	{
		routine->melody->name = (char*)malloc(513);
		routine->handle = SD::read(BLOCK_MELODY_NAME + index, routine->melody->name, 512, 1000);
	}
	CALL::setImmediate((CALL::tCallback)_readName, routine);
}

bool FS::deleteEvent(uint8 index)
{
	if (index < EVENT_MAX)
	{
		eventTable.event[index].chEvent.lengthType = PIN::eptNull;
		eventTable.event[index].chEvent.index = 0;
		eventTable.event[index].melodyIndex = -1;
		eventTable.event[index].poweroff = false;
		memset(&eventTable.event[index].chEvent.pulseType[0], 0, sizeof(eventTable.event[index].chEvent.pulseType));
		return true;
	}
	return false;
}

sint8 FS::setMelody(uint32 length, uint16 samples, char* name)
{
	if (length == 0 || length > MELODY_LENGTH_MAX || samples < 8000 || samples > 48000 || name == NULL) return -3;
	if (streamWrite != NULL) return -2;
	sint8 index = -1;
	uint8 i, n, e;
	uint32 address;
	tMelody* melody;
	melody = (tMelody*)malloc(sizeof(tMelody) * (MELODY_MAX + 1));
	for (i = 0, e = 0; i < MELODY_MAX; i++)
	{
		if (melodyTable.melody[i].address != 0)
		{
			memcpy(&melody[e], &melodyTable.melody[i], sizeof(tMelody));
			melody[e].length = (melody[e].length - 1) / 512 + 1;
			e++;
		}
		else if (index == -1)
		{
			index = i;
		}

	}
	if (index == -1)
	{
		free(melody);
		return -1;
	}
	if (e > 0)
	{
		i = 0;
		n = 1;
		while (i < (e - 1))
		{
			if (melody[i].address > melody[i + 1].address)
			{
				memcpy(&melody[MELODY_MAX], &melody[i], sizeof(tMelody));
				memcpy(&melody[i], &melody[i + 1], sizeof(tMelody));
				memcpy(&melody[i + 1], &melody[MELODY_MAX], sizeof(tMelody));
				if (i > 0)
				{
					i--;
					n++;
				}
			}
			else
			{
				i += n;
				n = 1;
			}
		}
		/*for (i = 0; i < e; i++)
		{
			DBGF("SETMELODYSORT: %i/%i", (int)melody[i].address, (int)melody[i].length);
		}*/
		address = BLOCK_MELODY_DATA;
		for (i = 0; i < e; i++)
		{
			if (((melody[i].address - address) * 512) >= length)
			{
				break;
			}
			address = melody[i].address + melody[i].length;
		}
	}
	else
	{
		address = BLOCK_MELODY_DATA;
	}
	free(melody);
	//DBGF("SETMELODY: %i %i/%i", index, (int)address, (int)length);
	melodyTable.melody[index].address = address;
	melodyTable.melody[index].length = length;
	melodyTable.melody[index].samples = samples;
	streamWrite = (tStreamWrite*)malloc(sizeof(tStreamWrite));
	streamWrite->address = address;
	streamWrite->length = length;
	streamWrite->writed = 0;
	/*streamWrite->sdHandle = -1;
	streamWrite->callHandle = -1;*/
	streamWrite->error = false;
	streamWrite->index = index;
	streamWrite->tag = name;
	streamWrite->sdHandle = SD::write(BLOCK_MELODY_NAME + index, name, strlen(name), WRITE_TIMEOUT);
	streamWrite->callHandle = CALL::setImmediate(_writeName);
	return index;
}

void FS::_writeName(__UNUSED void* param)
{
	streamWrite->callHandle = -1;
	if (streamWrite->sdHandle < 0)
	{
		DBGF_WARNING("FS: error sd write name %u", streamWrite->index);
		BT::write("+SETMELODY:ERROR;IO");
		free(streamWrite->tag);
	}
	else
	{
		if (SD::busy(streamWrite->sdHandle))
		{
#if FS_WAIT_BUSY == 0
			streamWrite->callHandle = CALL::setImmediate(_writeName);
#else
			streamWrite->callHandle = CALL::setTimeout(_writeName, FS_WAIT_BUSY);
#endif
		}
		else
		{
			if (SD::error(streamWrite->sdHandle))
			{
				DBGF_WARNING("FS: error write name %u", streamWrite->index);
				BT::write("+SETMELODY:ERROR;IO");
			}
			else
			{
				char* tmp = (char*)malloc(32);
				sprintf(tmp, "{\"index\":%i}", streamWrite->index);
				BT::write("+SETMELODY:");
				BT::send(tmp);
				free(tmp);
			}
			SD::clear(streamWrite->sdHandle);
			streamWrite->sdHandle = -1;
			free(streamWrite->tag);
		}
	}
}

void FS::_write(__UNUSED void* param)
{
	streamWrite->callHandle = -1;
	if (streamWrite->sdHandle < 0)
	{
		DBGF_WARNING("FS: error sd write %u", streamWrite->index);
		streamWrite->error = true;
		streamWrite->callback(streamWrite);
	}
	else
	{
		if (SD::busy(streamWrite->sdHandle))
		{
#if FS_WAIT_BUSY == 0
			streamWrite->callHandle = CALL::setImmediate(_write);
#else
			streamWrite->callHandle = CALL::setTimeout(_write, FS_WAIT_BUSY);
#endif
		}
		else
		{
			if (SD::error(streamWrite->sdHandle))
			{
				DBGF_WARNING("FS: error write %u", streamWrite->index);
				streamWrite->error = true;
				streamWrite->callback(streamWrite);
			}
			else
			{
				streamWrite->writed += 512;
				streamWrite->callback(streamWrite);
				streamWrite->address++;
			}
			SD::clear(streamWrite->sdHandle);
			streamWrite->sdHandle = -1;
		}
	}
}

bool FS::dataMelody(uint8* data, uint16 length, tCBMelodyWrite callback)
{
	if
		(
			   streamWrite == NULL
			|| length > 512
			|| streamWrite->sdHandle >= 0
			|| streamWrite->writed >= streamWrite->length
			|| streamWrite->error
		) return false;
	streamWrite->callback = (CALL::tCallback)callback;
	//streamWrite->dataLength = length;
	streamWrite->sdHandle = SD::write(streamWrite->address, data, length, WRITE_TIMEOUT);
	streamWrite->callHandle = CALL::setImmediate(_write);
	return true;
}

bool FS::endMelody(void)
{
	if (streamWrite != NULL)
	{
		if (streamWrite->callHandle >= 0) CALL::clear(streamWrite->callHandle);
		if (streamWrite->sdHandle >= 0) SD::clear(streamWrite->sdHandle);
		free(streamWrite);
		streamWrite = NULL;
		return true;
	}
	return false;
}

bool FS::deleteMelody(uint8 index)
{
	if (index < MELODY_MAX && (streamWrite == NULL || streamWrite->index != index))
	{
		melodyTable.melody[index].address = 0;
		melodyTable.melody[index].length = 0;
		melodyTable.melody[index].samples = 8000;
		return true;
	}
	return false;
}

void FS::_readName(tMelody2Routine* routine)
{
	if (routine->handle < 0)
	{
		if (routine->melody->name != NULL)
		{
			sprintf(routine->melody->name, "- error read name -");
		}
		routine->callback(routine->melody);
		if (routine->melody->name != NULL)
		{
			free(routine->melody->name);
		}
		free(routine->melody);
		free(routine);
	}
	else
	{
		if (SD::busy(routine->handle))
		{
#if FS_WAIT_BUSY == 0
			CALL::setImmediate((CALL::tCallback)_readName, routine);
#else
			CALL::setTimeout((CALL::tCallback)_readName, FS_WAIT_BUSY, routine);
#endif
		}
		else
		{
			if (SD::error(routine->handle))
			{
				sprintf(routine->melody->name, "- error read name -");
			}
			else
			{
				routine->melody->name[512] = '\0';
				routine->callback(routine->melody);
			}
			SD::clear(routine->handle);
			free(routine->melody->name);
			free(routine->melody);
			free(routine);
		}
	}
}

bool FS::saveTables(tCBSave callback)
{
	if (_saveTables != NULL) return false;
	_saveTables = (tSaveTables*)malloc(sizeof(tSaveTables));
	melodyTableCRC();
	eventTableCRC();
	_saveTables->callback = callback;
	_saveTables->melodyHandle = SD::write(BLOCK_MELODY_TABLE, &melodyTable, sizeof(tMelodyTable), WRITE_TIMEOUT);
	_saveTables->eventHandle = SD::write(BLOCK_EVENT_TABLE, &eventTable, sizeof(tEventTable), WRITE_TIMEOUT);
	_saveTables->melodyError = false;
	_saveTables->eventError = false;
	_saveTables->melodyComplete = false;
	_saveTables->eventComplete = false;
	CALL::setImmediate(writeMelodyTable);
	CALL::setImmediate(writeEventTable);
	return true;
}

void FS::writeMelodyTable(__UNUSED void* param)
{
	if (SD::busy(_saveTables->melodyHandle))
	{
		CALL::setImmediate(writeMelodyTable);
	}
	else
	{
		if (SD::error(_saveTables->melodyHandle))
		{
			_saveTables->melodyError = true;
		}
		_saveTables->melodyComplete = true;
		SD::clear(_saveTables->melodyHandle);
		writeSaveComplete();
	}
}

void FS::writeEventTable(__UNUSED void* param)
{
	if (SD::busy(_saveTables->eventHandle))
	{
		CALL::setImmediate(writeEventTable);
	}
	else
	{
		if (SD::error(_saveTables->eventHandle))
		{
			_saveTables->eventError = true;
		}
		_saveTables->eventComplete = true;
		SD::clear(_saveTables->eventHandle);
		writeSaveComplete();
	}
}

void FS::writeSaveComplete(void)
{
	if (_saveTables->melodyComplete && _saveTables->eventComplete)
	{
		_saveTables->callback((!_saveTables->melodyError && !_saveTables->eventError));
		free(_saveTables);
		_saveTables = NULL;
	}
}
