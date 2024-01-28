/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 7 мар. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_CH_HPP_
#define INC_CH_HPP_ 1

#include <main.hpp>
#include <dbg.hpp>
#include <timer.hpp>
#include <call.hpp>
#include <hash.hpp>

#define CH_IRQ_THREAD		1
#define CH_IRQ_PER_SECOND	10000

class PIN
{
public:

	static const uint16 ANTI_BOUNCE_HIGH				= 20;
	static const uint16 ANTI_BOUNCE_LOW					= 10;
	static const uint16 ANTI_BOUNCE_BUTTON_HIGH			= 50;
	static const uint16 ANTI_BOUNCE_BUTTON_LOW			= 50;

	static const uint16 PRESS_DEF						= 500;
	static const uint16 IDLE_DEF						= 1000;
	static const uint8  PULSES_DEVIATION_DEF			= 20;
	static const uint8  PULSES_DEF						= 20;

	static const uint8  FIRST_TIME_COMPENSATION			= 5;

	static const uint8  PULSES_TYPE_MAXIMUM				= 2;	/* min. 2 for measure */
	static const uint16 TIME_MAXIMUM					= 2000;
	static const uint16 DEVIATION_MAXIMUM				= 50;
	static const uint16 PULSES_MAXIMUM					= 20;

	typedef enum : sint8
	{
		eptErrMeasure	= -5,
		eptMeasure		= -4,
		eptCyclic		= -3,
		eptRelease		= -2,
		eptPress		= -1,
		eptNull			= 0
	}
	ePulseType;

	typedef struct
	{
		sint16 width;
		sint16 count;
	}
	tPulses;

	typedef struct
	{
		uint16 pressMax;
		uint16 idleMax;
		uint8 pulsesMax;
		uint8 deviation;
		bool button;
		bool measure;
		bool playOnce;
		bool playToEnd;
		bool noWidth;
		bool noBounceHigh;
		bool standard;
	}
	tCfg;

	tCfg cfg;
	tPulses pulseType[PULSES_TYPE_MAXIMUM];
	ePulseType lengthType;

	PIN(uint8 index, uint16 pin, bool button = false, bool standard = false);

	bool inline active(void);
	uint8 inline index(void);
	void reset(void);
	void set(ePulseType type);

	void thread(void);

	bool inline _get(void);

private:

	TIMER* bounceTime;
	TIMER* pressTime;
	TIMER* idleTime;
	TIMER* pulseWidth;
	uint16 _pin;
	uint8 _index;
	uint8 pulses;
	uint8 firstTime;
	bool stateBounce;
	bool state;
	bool pressTrigged;
	bool idleTrigged;
	bool multiPulses;
	bool cyclicPulses;
	bool _active;

};

class CH
{
public:

	static const uint8 CHANNELS		= 3;

	typedef struct
	{
		PIN::tPulses pulseType[PIN::PULSES_TYPE_MAXIMUM];
		PIN::ePulseType lengthType;
		uint8 index;
		bool playOnce;
		bool playToEnd;
	}
	tEvent;

	static void Init(void);
	static void DeInit(void);
	static void Start(void);

	static bool active(void);
	static bool event(tEvent* event);
	static void set(uint8 index, const PIN::tCfg* config);
	static PIN::tCfg* get(uint8 index);
	static uint8 deviation(uint8 index);
	static bool standard(uint8 index);

private:

	static PIN* ch[CHANNELS];

	static void __irq_thread(void);
	static void thread(void* param);

};

#endif
