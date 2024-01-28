/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 22 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <timer.hpp>

extern "C" void SysTick_Handler_Boot(void) __attribute__((alias("_ZN5TIMER5__irqEv")));

uint32 TIMER::__tick;

void TIMER::__irq(void)
{
	__tick++;
}

void TIMER::Init(void)
{
	__tick = 0;
	SysTick->LOAD = SystemCoreClock / TIMER_TICK_HZ - 1;
	SysTick->VAL = 0;
	NVIC_SetPriority(SysTick_IRQn, TIMER_PRIORITY);
	SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk;
}

void TIMER::DeInit(void)
{
	SysTick->CTRL = 0;
	NVIC_SetPriority(SysTick_IRQn, 0);
}

TIMER::TIMER(uint32 milli)
{
	set(milli);
}

uint32 TIMER::getTick(void)
{
	uint32 tick;
	__DMB();
	tick = __tick;
	__DMB();
	return tick;
}

void TIMER::uset(uint32 micro)
{
#if TIMER_TICK_HZ > 1000UL
	time = micro / (1000000UL / TIMER_TICK_HZ);
	tick = getTick();
#else
	set(micro / 1000UL);
#endif
}

void TIMER::set(uint32 milli)
{
#if TIMER_TICK_HZ > 1000UL
	uset(milli * 1000UL);
#elif TIMER_TICK_HZ < 1000UL
	time = milli / (1000UL / TIMER_TICK_HZ);
	tick = getTick();
#else
	time = milli;
	tick = getTick();
#endif
}

uint32 TIMER::uget(void)
{
#if TIMER_TICK_HZ > 1000UL
	uint32 currTick = getTick();
	if (currTick < tick)
	{
		currTick = ~tick + currTick + 1;
	}
	else
	{
		currTick -= tick;
	}
	return currTick * (1000000UL / TIMER_TICK_HZ);
#else
	return get() * 1000UL;
#endif
}

uint32 TIMER::get(void)
{
#if TIMER_TICK_HZ > 1000UL
	return uget() / 1000UL;
#else
	uint32 currTick = getTick();
	if (currTick < tick)
	{
		currTick = ~tick + currTick + 1;
	}
	else
	{
		currTick -= tick;
	}
#if TIMER_TICK_HZ < 1000UL
	return currTick * (1000UL / TIMER_TICK_HZ);
#else
	return currTick;
#endif
#endif
}

bool TIMER::finish(void)
{
	if (time == 0) return true;
	uint32 currTick = getTick();
	uint32 newTick = tick + time;
	if (newTick < tick)
	{
		if (currTick < time && newTick <= currTick)
		{
			time = 0;
			return true;
		}
	}
	else if (newTick <= currTick)
	{
		time = 0;
		return true;
	}
	return false;
}

bool TIMER::interval(void)
{
	if (time == 0) return true;
	uint32 currTick = getTick();
	uint32 newTick = tick + time;
	if (newTick < tick)
	{
		if (currTick < time && newTick <= currTick)
		{
			tick = currTick;
			return true;
		}
	}
	else if (newTick <= currTick)
	{
		tick = currTick;
		return true;
	}
	return false;
}
