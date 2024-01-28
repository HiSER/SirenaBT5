/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 13 мар. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_FS_HPP_
#define INC_FS_HPP_ 1

#include <main.hpp>
#include <dbg.hpp>
#include <sd.hpp>
#include <ch.hpp>
#include <call.hpp>
#include <bt.hpp>
#include <amp.hpp>

#define FS_WAIT_BUSY	0

class FS
{
public:

	static const uint8 MELODY_MAX			= 40;
	static const uint8 EVENT_MAX			= 30;

	static const uint32 BLOCK_MELODY_TABLE	= 1; /* 1 */
	static const uint32 BLOCK_EVENT_TABLE	= 2; /* 1 */
	static const uint32 BLOCK_MELODY_NAME	= 3; /* 50 */
	static const uint32 BLOCK_MELODY_DATA	= BLOCK_MELODY_NAME + MELODY_MAX;

	static const uint32 MELODY_LENGTH_MAX	= (2 * 1024 * 1024);

	static const int WRITE_TIMEOUT			= 1000;

	static const int FIND_TIMEOUT			= 2000;


	typedef struct __PACKED
	{
		/*CH::tEvent chEvent;*/
		struct
		{
			PIN::tPulses pulseType[PIN::PULSES_TYPE_MAXIMUM];
			PIN::ePulseType lengthType;
			uint8 index;
		}
		chEvent;
		sint8 melodyIndex;
		bool poweroff;
		uint8 volume;
		sint8 noPlayAfter;
	}
	tEvent;

	typedef struct
	{
		struct
		{
			uint8 index[MELODY_MAX];
			uint8 count;
		}
		melody;
		struct
		{
			uint8 index[EVENT_MAX];
			uint8 count;
		}
		event;
	}
	tList;

	typedef struct
	{
		char* name;
		uint8 index;
	}
	tMelody2;

	typedef struct
	{
		uint32 address;
		uint32 length;
		uint32 readed;
		CALL::tCallback callback;
		int timeout;
		int callHandle;
		int sdHandle;
		void* tag;
		struct
		{
			uint8 buffer[512];
			uint32 offset;
			uint32 length;
			bool eof;
			bool error;
		}
		data;
	}
	tStreamRead;

	typedef struct
	{
		uint32 address;
		uint32 length;
		uint32 writed;
		CALL::tCallback callback;
		int callHandle;
		int sdHandle;
		void* tag;
		//uint16 dataLength;
		bool error;
		uint8 index;
	}
	tStreamWrite;

	typedef struct __PACKED
	{
		uint32 address;
		uint32 length;
		uint16 samples;
	}
	tMelody;

	typedef struct
	{
		TIMER* timeout;
		CALL::tCallback callback;
		int handle;
		tMelody melody;
		CH::tEvent event;
		sint8 melodyIndex;
		bool poweroff;
		uint8 volume;
		sint8 noPlayAfter;
		sint8 eventId;
	}
	tFindMelody;

	typedef void (*tCBOpen)(tStreamRead*);
	typedef void (*tCBMelody2)(tMelody2*);
	typedef void (*tCBMelodyWrite)(tStreamWrite*);
	typedef void (*tCBSave)(bool);
	typedef void (*tCBFind)(tFindMelody*);

	static void Init(void);
	static void DeInit(void);

	static void reset(void);

	static tList* list(void);

	static tEvent* getEvent(uint8 index);
	static bool setEvent(uint8 index, tEvent* event);
	static bool deleteEvent(uint8 index);

	static void getMelody(uint8 index, tCBMelody2 callback);
	static sint8 setMelody(uint32 length, uint16 samples, char* name);				/* length <= MELODY_LENGTH_MAX */
	static bool dataMelody(uint8* data, uint16 length, tCBMelodyWrite callback);	/* dataLength <= 512 */
	static bool endMelody(void);
	static bool deleteMelody(uint8 index);

	static void melodyTableClear(void);
	static void eventTableClear(void);

	static bool saveTables(tCBSave callback);

	static tFindMelody* preMelodyCreate(uint8 index, tCBFind callback);
	static tFindMelody* findMelodyCreate(CH::tEvent* event, tCBFind callback);
	static void findMelodyRemove(tFindMelody* find);

	static tStreamRead* open(uint32 address, uint32 length, tCBOpen callback, int timeout = 0, void* tag = NULL); /* custom free callback param */
	static void next(tStreamRead* stream);
	static void close(tStreamRead* stream);

	static bool hasReady(void);

private:

	typedef struct __PACKED
	{
		tMelody melody[MELODY_MAX];
		uint16 crc;
	}
	tMelodyTable;

	typedef struct __PACKED
	{
		tEvent event[EVENT_MAX];
		uint16 crc;
	}
	tEventTable;

	typedef struct
	{
		tMelody2* melody;
		tCBMelody2 callback;
		int handle;
	}
	tMelody2Routine;

	typedef struct
	{
		tCBSave callback;
		int melodyHandle;
		int eventHandle;
		bool melodyComplete;
		bool eventComplete;
		bool melodyError;
		bool eventError;
	}
	tSaveTables;

	static tMelodyTable melodyTable;
	static tEventTable eventTable;
	static tStreamWrite* streamWrite;
	static tSaveTables* _saveTables;
	static bool readyMelodyTable;
	static bool readyEventTable;
	static bool errorMelodyTable;
	static bool errorEventTable;
	static bool _ready;
	static bool _reset;

	static void readMelodyTable(int handle);
	static void readEventTable(int handle);
	static void ready(void);

	static bool melodyTableCRC(void);
	static bool eventTableCRC(void);

	static void _read(tStreamRead* stream);
	static void _writeName(__UNUSED void* param);
	static void _write(void* param);
	static void _readName(tMelody2Routine* routine);

	static void writeMelodyTable(void* param);
	static void writeEventTable(void* param);
	static void writeSaveComplete(void);

	static void findMelodyRoutine(tFindMelody* find);
	static void preMelodyRoutine(tFindMelody* find);

};

#endif
