/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 22 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_TIMER_HPP_
#define INC_TIMER_HPP_ 1

#include <main.hpp>

#define TIMER_TICK_HZ	(10000UL)

/**
 * Example
 *
 * include <timer.hpp>
 *
 * void main(void)
 * {
 *  	TIMER::Init();
 *
 *  	TIMER* t1 = new TIMER(1000);	// set 1000 ms
 *  	TIMER* t2 = new TIMER();		// set 0 ms
 *
 *  	LOOP
 *  	{
 *  		if (t1->interval())	// first call 1000 ms
 *  		{
 *  			// call interval 1000 ms
 *  		}
 *  		if (t2->finish())	// first call 0 ms
 *  		{
 *  			t2->set(500);
 *
 *  			// call interval 500 ms
 *  		}
 *  	}
 * }
 */

class TIMER
{
public:

	static void Init(void);
	static void DeInit(void);
	static uint32 getTick(void);

	TIMER(uint32 milli = 0);

	void uset(uint32 micro);
	void set(uint32 milli = 0);

	uint32 uget(void);
	uint32 get(void);

	bool finish(void);
	bool interval(void);

protected:

	uint32 tick;
	uint32 time;

private:

	static uint32 __tick;

	static void __irq(void);

};

#endif
