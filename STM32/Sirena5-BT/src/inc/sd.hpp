/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 11 мар. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_SD_HPP_
#define INC_SD_HPP_ 1

#include "main.hpp"
#include "dbg.hpp"
#include "call.hpp"
#include "fs.hpp"

#define SD_REPLACE_TO_SDD	1


#if SD_REPLACE_TO_SDD == 0

#define SD_CMD(index)					(index | 0x40)

#define SD_RESPONSE_MASK				0x7F
#define SD_RESPONSE_IDLE				0x01
#define SD_RESPONSE_ERROR_CMD			0x04

#define SD_DATA_TOKEN					0xFE
#define SD_DATA_RESPONSE_MASK			0x11
#define SD_DATA_RESPONSE				0x01
#define SD_DATA_RESPONSE_FLAGS_MASK		0x0E
#define SD_DATA_RESPONSE_ACCEPT			0x04

class SD
{
public:

	static const uint8 POOL_SIZE		= 24;
	static const uint16 BYTES_FREEZE	= 2000;
	static const uint16 REINIT_TIMEOUT	= 300;
	static const uint16 FIRST_ON_WAIT	= 200;
	static const uint8 APPINIT_COUNTER	= 10;

	typedef enum : uint8
	{
		cmdNull					= 0,
		cmdDummy				= 1,
		cmdReadBlockData		= 2,
		cmdWriteBlockData		= 3,
		cmdWriteBlockResponse	= 4,
		cmdWriteBlockBusy		= 5,
		cmdIdle					= SD_CMD(0),
		cmdMMCInit				= SD_CMD(1),
		cmdIfCond				= SD_CMD(8),
		cmdSetBlock				= SD_CMD(16),
		cmdReadBlock			= SD_CMD(17),
		cmdWriteBlock			= SD_CMD(24),
		cmdAppCmd				= SD_CMD(55),
		cmdReadOCR				= SD_CMD(58),
		cmdAppInit				= SD_CMD(41),
	}
	eCmd;

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

	typedef enum : uint8
	{
		errSusses = 0,
		errFreeze
	}
	eError;

	static void Init(void);
	static void DeInit(void);

	static int read(uint32 address, void* data, uint16 length, int timeout = 0); /* length <= 512 bytes */
	static int write(uint32 address, const void* data, uint16 length, int timeout = 0); /* length <= 512 bytes */

	static bool clear(int handle);

	static eStat status(int handle);
	static bool error(int handle);
	static bool busy(int handle);

	static uint8 getTasks(void);
	static uint8 getMaximumTasks(void);

	static bool reset;

private:

	typedef struct
	{
		union
		{
			struct __PACKED
			{
				uint8 dummy;
				eCmd index;
				union
				{
					uint8 u8[4];
					uint32 u32;
				}
				address;
				uint8 crc;
			}
			s;
			uint8 u8[7];
		}
		cmd;
		uint8 response;
		uint8 dataResponse;
		uint8 data[515];
		uint16 dataLength;
		uint16 sendLength;
		uint16 recvLength;
		bool highCapacity;
		bool rxSync;
		uint8 error;
		uint8 counter;
	}
	tSD;

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
		tTask task[POOL_SIZE];
		uint8 in;
		uint8 out;
		uint8 length;
		uint8 lengthMax;
	}
	tPool;

	static tPool pool;

	static tSD sd;

	static bool ready;
	static bool _reset;

	static void __irq(void);

	static void cs(bool on);
	static void speed(bool full);
	static void power(bool on);
	static bool _error(void);
	static bool _busy(void);

	static void Thread(void* param);

	static void resetPowerOff(void* param);
	static void resetRoutine(void* param);
	static void resetPowerOn(void* param);
	static void resetSendDummy(void* param);
	static void resetWaitDummy(void* param);
	static void resetIdle(void* param);
	static void resetWaitIdle(void* param);
	static void resetIfCond(void* param);
	static void resetWaitIfCond(void* param);
	static void resetAppCmd(CALL::tCallback appCallback);
	static void resetWaitAppCmd(CALL::tCallback appCallback);
	static void resetAppInit(void* param);
	static void resetWaitAppInit(void* param);
	static void resetMMCInit(void* param);
	static void resetWaitMMCInit(void* param);
	static void resetReadOCR(void* param);
	static void resetWaitReadOCR(void* param);
	static void resetSetBlock(void* param);
	static void resetWaitSetBlock(void* param);

	static void send(eCmd index, uint32 address = 0, uint8* data = NULL, uint16 length = 0);

};

#else

#include "sdd.hpp"

#define SD SDD

#endif

#endif
