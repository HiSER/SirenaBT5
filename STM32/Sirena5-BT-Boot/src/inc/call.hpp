/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 24 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_CALL_HPP_
#define INC_CALL_HPP_ 1

#include <main.hpp>
#include <timer.hpp>
#include <dbg.hpp>

/**
 * Example
 *
 * include <call.hpp>
 *
 * void cb(__UNUSED void* param)
 * {
 *  	// Call interval 1000 ms
 * }
 *
 * void main(void)
 * {
 *  	CALL::Init();
 *
 *  	CALL::setInterval(cb, 1000);
 *
 *  	LOOP
 *  	{
 *  		CALL::Thread();
 *  	}
 * }
 */

class CALL
{
public:

	static const uint16 CallStackSize		= 32;

	typedef void (*tCallback)(void*);

	static void Init(void);
	static void DeInit(void);
	static void Thread(void);

	static int setInterval(tCallback callback, int milli, void* param = NULL);
	static int setTimeout(tCallback callback, int milli, void* param = NULL);
	static int setImmediate(tCallback callback, void* param = NULL);

	static void clear(int handle);

	static uint16 getCalls(void);
	static uint16 getMaximumCalls(void);

private:

	typedef struct
	{
		tCallback callback;
		void* param;
		TIMER* time;
		bool interval;
		bool queue;
	}
	tProc;

	static tProc stack[CallStackSize];
	static uint16 in;
	static uint16 out;
	static uint16 length;
	static uint16 lengthMax;

	static void __fatal(const char* text);

};

#endif
