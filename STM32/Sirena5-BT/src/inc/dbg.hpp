/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 23 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_DBG_HPP_
#define INC_DBG_HPP_ 1

#include <main.hpp>
#include <fifo.hpp>
#include <bt.hpp>

#define DEBUG_LEVEL				DEBUG_LEVEL_NOTICE

#define DEBUG_LEVEL_OFF			0
#define DEBUG_LEVEL_NONE		1
#define DEBUG_LEVEL_FATAL		2
#define DEBUG_LEVEL_WARNING		3
#define DEBUG_LEVEL_NOTICE		4

/**
 * Example
 *
 * include <dbg.hpp>
 *
 * void main(void)
 * {
 *  	DBG::Init();
 *
 *  	//Stanrad library
 *
 *  	puts("Hello!");
 *
 *  	LOOP
 *  	{
 *
 *  	}
 * }
 */

#define STRBOOL(b) ((b) ? "TRUE" : "FALSE")

#if DEBUG_LEVEL > DEBUG_LEVEL_OFF
#define DBGP(text) {puts(text);}
#define DBGPN(text) {printf(text);}
#define DBGF(format, ...) {printf(format, __VA_ARGS__);putchar('\n');}
#else
#define DBGP(text)
#define DBGPN(text)
#define DBGF(format, ...)
#endif

#if DEBUG_LEVEL > DEBUG_LEVEL_NONE
#define DBGP_FATAL(text) DBGP(text)
#define DBGPN_FATAL(text) {DBGPN(text);}
#define DBGF_FATAL(format, ...) DBGF(format, __VA_ARGS__)
#else
#define DBGP_FATAL(text)
#define DBGPN_FATAL(text)
#define DBGF_FATAL(format, ...)
#endif

#if DEBUG_LEVEL > DEBUG_LEVEL_FATAL
#define DBGP_WARNING(text) DBGP(text)
#define DBGPN_WARNING(text) {DBGPN(text);}
#define DBGF_WARNING(format, ...) DBGF(format, __VA_ARGS__)
#else
#define DBGP_WARNING(text)
#define DBGPN_WARNING(text)
#define DBGF_WARNING(format, ...)
#endif

#if DEBUG_LEVEL > DEBUG_LEVEL_WARNING
#define DBGP_NOTICE(text) DBGP(text)
#define DBGPN_NOTICE(text) {DBGPN(text);}
#define DBGF_NOTICE(format, ...) DBGF(format, __VA_ARGS__)
#else
#define DBGP_NOTICE(text)
#define DBGPN_NOTICE(text)
#define DBGF_NOTICE(format, ...)
#endif

#define ATP2(name) void __ATP2_##name(__UNUSED DBG::tCmdAT* at)
#define ANSWER2(data) DBG::answer(at, data)
#define AT2(nameCommand, procName) \
	{ \
		.command	= nameCommand, \
		.proc		= __ATP2_##procName \
	},

class DBG
{
public:

	static const uint32 TXBaud			= 230400;
	static const uint16 TXBufferSize	= 1024;

	static const uint32 UARTBaud		= 19200;
	static const uint32 UARTBufferSize	= 128;
	static const uint32 BufferSize		= UARTBufferSize + 1;
	static const uint16 ATArgsMax		= 1;

	static FIFO8* tx;
	static FIFO8* rx;
	static bool _uartControl;
	static char* buffer;
	static uint16 length;

	static void Init(void);
	static void DeInit(void);

	static void ReInit(bool uartControl);
	static bool getUartControl();

	static void InitAT();

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

	static void write(const char* data);
	static void send(const char* data);
	static void answer(tCmdAT* at, const char* data);
	static void printBuff(const char* prefix, void* data, int length);

private:

	/*typedef struct
	{
		char* nextPTR;
		uint16 length;
		bool isNumber;
		sint32 sNumber;
		uint32 uNumber;
	}
	tStrVar;*/

	static int handleThread;

	static void __irq(void);

	static void Thread(void* param);

	static void bufferClear(void);
	static void bufferShift(uint16 shift);

	static void remove(tCmdAT* at);
	static tCmdAT* parse(char* buff, uint16* length);
	static void execute(tCmdAT* at);
	//static tStrVar* _strvar(char* src);

};

#endif
