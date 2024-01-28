/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 9 мар. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_SIRENA_HPP_
#define INC_SIRENA_HPP_ 1

#include <main.hpp>
#include <dbg.hpp>
#include <timer.hpp>
#include <call.hpp>
#include <ch.hpp>
#include <bt.hpp>
#include <amp.hpp>
#include <hash.hpp>
#include <encoders.hpp>
#include <fs.hpp>

class SIRENA
{
public:

	static const uint32 POWEROFF_TIMEOUT	= (3 * 60 * 1000);
	static const uint32 POWEROFF_DELAY		= (10 * 1000);

	static bool cfgSleep;

	static void Init(void);
	static void DeInit(void);

	static void resetPower();

	static void prePlay(uint8 index);
	static void preStop(void);

private:

	static const uint16 standardWaveSamples = 16000;
	static const uint8 standardWave[];

	typedef struct
	{
		CH::tEvent event;
		FS::tMelody melody;
		FS::tStreamRead* stream;
		struct
		{
			TIMER* time;
			uint16 length;
			uint16 offset;
			uint8 pulses;
			uint8 types;
			bool enable;
			bool active;
		}
		beep;
		bool playing;
		bool pause;
		bool stop;
		bool reset;
		bool poweroff;
		uint8 volume;
		sint8 melodyIndex;
	}
	tPlay;

	static TIMER* powerOff;
	static CH::tEvent event;
	static tPlay play[CH::CHANNELS + 1];
	static sint8 lostEventId;

	static void ThreadPower(void *param);
	static void Thread(void *param);

	static void findCallback(FS::tFindMelody* find);

	static void playStart(uint8 chIndex);
	static void playStop(uint8 chIndex);
	static void playOtherPause(uint8 chIndex);
	static bool playHasPause(void);
	static void beepRoutine(uint8 index);
	static void playRoutione(FS::tStreamRead* stream = NULL);

	static char* eventToStr(CH::tEvent* event);

};

#endif
