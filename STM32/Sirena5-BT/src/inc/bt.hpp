/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 24 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_BT_HPP_
#define INC_BT_HPP_ 1

#include <main.hpp>
#include <dbg.hpp>
#include <fifo.hpp>
#include <call.hpp>

#define BT_RX_USE_DMA		1

#define ANSWERBOOL(b) ((b) ? "OK" : "ERROR")

#define ATP(name) void __ATP_##name(__UNUSED BT::tCmdAT* at)
#define ANSWER(data) BT::answer(at, data)

#define __ATP_NULL NULL

#define AT(nameCommand, procName) \
	{ \
		.command	= nameCommand, \
		.proc		= __ATP_##procName \
	},

#define ATI(nameCommand, procName) \
	{ \
		.command	= nameCommand, \
		.proc		= procName \
	},

#define AT_NULL AT(NULL, NULL)

class BT
{
public:

	static const uint16 TXBufferSize	= 2048;
	static const uint16 RXBufferSize	= 2048;
	static const uint16 BufferSize		= RXBufferSize + 1;
	static const uint16 ATArgsMax		= 12;
	static const uint16 PINLength		= 4;

	typedef struct
	{
		char* data;
		uint16 length;
		bool isNumber;
		sint32 sNumber;
		uint32 uNumber;
	}
	tArgAT;

	typedef struct
	{
		char* command;
		uint16 argc;
		tArgAT* argv[ATArgsMax];
	}
	tCmdAT;

	typedef void (tProcAT)(tCmdAT* at);

	typedef struct
	{
		const char* command;
		tProcAT* proc;
	}
	tListAT;

	static const tListAT listAT[];
	static bool reset;

	static char pinCode[PINLength + 1];

	static void Init(void);
	static void DeInit(void);

	static void write(const char* data);
	static void send(const char* data);
	static void answer(tCmdAT* at, const char* data);

	static bool connected(void);

	static bool testUNumber(tArgAT* argv, uint32 min = 0, uint32 max = 0xFFFFFFFF);
	static bool testNumber(tArgAT* argv, int min = 0x80000000, int max = 0x7FFFFFFF);

	typedef struct
	{
		char* nextPTR;
		uint16 length;
		bool isNumber;
		sint32 sNumber;
		uint32 uNumber;
	}
	tStrVar;

	static tStrVar* _strvar(char* src);

private:

	static const uint32 baudTable[];
	static const char* resetCMD[];

	typedef struct
	{
		int timeoutHandle;
		//uint16 firstTry;
		uint16 baudIndex;
		uint16 commandIndex;
		bool testAT;
	}
	tReset;

	static FIFO8* tx;
#if BT_RX_USE_DMA == 0
	static FIFO8* rx;
#else
	static char* rx;
	static uint16 rxIn;
	static uint16 rxOut;
#endif
	static char* buffer;
	static tReset* resetVar;
	static uint16 length;
	static bool _reset;
	static bool ready;
	static bool _connected;
	//static bool _connectedPrimary;
	static bool jdy31;

	void __irq_usart(void);

	static void thread(void* param);
	static void execute(tCmdAT* at);
	static tCmdAT* parse(char* buff, uint16* length);
	static void remove(tCmdAT* at);

	static void bufferClear(void);
	static void bufferShift(uint16 shift);
#if BT_RX_USE_DMA != 0
	static void bufferAdd(const char* data, uint16 len);
#endif

	static void resetStart(void* param);
	static void resetTimeout(void* param);
	static void readyCmd(tCmdAT* at);
	static void testAT(tCmdAT* at);
	static void resetWait(void* param);
	static void resetCommand(void* param);

	static void connectedCmd(tCmdAT* at);
	static void disconnectedCmd(tCmdAT* at);
};

#endif
