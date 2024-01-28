/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 23 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_DBG_HPP_
#define INC_DBG_HPP_ 1

#include <main.hpp>
#include <fifo.hpp>

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

class DBG
{
public:

	static const uint32 TXBaud			= 230400;
	static const uint16 TXBufferSize	= 1024;

	static FIFO8* tx;

	static void Init(void);
	static void DeInit(void);

private:

	static void __irq(void);

};

#endif
