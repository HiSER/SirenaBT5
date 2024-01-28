/**
 * HiSER (c)2020 Litvin Artem Vasilyevich
 * Date 16 июл. 2020 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_SDD_HPP_
#define INC_SDD_HPP_ 1

#include "main.hpp"
#include "call.hpp"
#include "dbg.hpp"
#include "fs.hpp"

#define SD_CMD(index)					(index | 0x40)

#define SD_RESPONSE_MASK				0x7F
#define SD_RESPONSE_IDLE				0x01
#define SD_RESPONSE_ERROR_CMD			0x04

#define SD_DATA_TOKEN					0xFE
#define SD_DATA_RESPONSE_MASK			0x11
#define SD_DATA_RESPONSE				0x01
#define SD_DATA_RESPONSE_FLAGS_MASK		0x0E
#define SD_DATA_RESPONSE_ACCEPT			0x04

class SDD
{
public:

	static const int PowerOnWait		= 50;
	static const int ResetWait			= 300;
	static const int InitWait			= 100;
	static const int BusyWait			= 10;
	static const int APPInitCount		= 20;
	static const int MMCInitCount		= 20;
	static const int BusyCount			= 100;
	static const int DummyFirst			= 64;
	static const int DummySpeed			= 4096;

	static const int SPIBuffer			= (4096 + 1024);
	static const int ResponseLength		= 24;
	static const int DataBlockLength	= (SPIBuffer - 1);

	static const int PoolCount			= 24;



	typedef enum : uint8
	{
		statEmpty = 0,
		statSusses,
		statWait,
		statBusy,
		statError,
		statTimeout,
	}
	eStat;



	static void Init();
	static void DeInit();

	static int read(uint32 address, void* data, uint16 length, int timeout = 0); /* length <= 512 bytes */
	static int write(uint32 address, const void* data, uint16 length, int timeout = 0); /* length <= 512 bytes */

	static bool clear(int handle);

	static eStat status(int handle);
	static bool error(int handle);
	static bool busy(int handle);

	static uint8 getTasks(void);
	static uint8 getMaximumTasks(void);

private:

	typedef enum : uint8
	{
		cmdIdle					= SD_CMD(0),
		cmdMMCInit				= SD_CMD(1),
		cmdIfCond				= SD_CMD(8),
		cmdSetBlock				= SD_CMD(16),
		cmdReadBlock			= SD_CMD(17),
		cmdWriteBlock			= SD_CMD(24),
		cmdAppCmd				= SD_CMD(55),
		cmdReadOCR				= SD_CMD(58),
		cmdAppInit				= SD_CMD(41)
	}
	eCMD;

	typedef struct __PACKED
	{
		uint8 dummy;
		eCMD index;
		union
		{
			uint8 u8[4];
			uint32 u32;
		}
		address;
		uint8 crc;
	}
	tCMD;

	typedef struct
	{
		TIMER* timeout;
		uint32 address;
		uint8* data;
		uint16 length;
		eStat status;
		bool read;
	}
	tTask;

	typedef struct
	{
		tTask task[PoolCount];
		uint8 in;
		uint8 out;
		uint8 length;
		uint8 lengthMax;
	}
	tPool;

	typedef struct
	{
		int handleThread;
		struct
		{
			uint8 buffer[SPIBuffer];
			uint16 length;
			uint16 start;
			bool routine;
		}
		spi;
		struct
		{
			CALL::tCallback callback;
			tTask* task;
			uint16 counter;
			bool reset;
			bool _reset;
			bool ready;
			bool highCapacity;
			uint8 response;
			tCMD cmd;
		}
		sd;
	}
	tVars;



	static tPool pool;
	static tVars vars;



	static void Thread(void* param);
	static void readCallback(uint8* buffer);
	static void writeCallback(uint8* buffer);

	static void powerReset();
	static void _powerReset(void* param = NULL);
	static void powerOn(void* param = NULL);
	static void waitDummy(void* param = NULL);
	static void Idle(void* param = NULL);
	static void IfCond(uint8* cond);
	static void AppCmdInit(void* param = NULL);
	static void AppInit(void* param = NULL);
	static void _AppInit(void* param = NULL);
	static void ReadOCR(uint8* ocr);
	static void MMCInit(void* param = NULL);
	static void _MMCInit(void* param = NULL);
	static void SetBlock(void* param = NULL);
	static void _SetBlock(void* param = NULL);
	static void setFullSpeed(void* param = NULL);
	static void _setFullSpeed(void* param = NULL);

	static void InitSPI();
	static void DeInitSPI();

	static void cs(bool on);
	static void power(bool on);
	static void speed(bool full);

	static void ReadSPI(uint16 length);
	static void WriteSPI(void* data, uint16 length);
	static bool ReadBusySPI();
	static bool WriteBusySPI();
	static void ThreadSPI();

	static void send(eCMD cmd, CALL::tCallback callback, uint32 address = 0, tTask* task = NULL);
	static void _send(void* param = NULL);
	static void response(void* param = NULL);
	static void _response(void* param = NULL);
	static void sendWrite(void* param = NULL);
	static void writeResponse(void* param = NULL);
	static void waitBusy(void* param = NULL);
	static void _waitBusy(void* param = NULL);

};

#endif
