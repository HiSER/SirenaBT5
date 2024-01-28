/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 24 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_AMP_HPP_
#define INC_AMP_HPP_ 1

#include <main.hpp>
#include <dbg.hpp>
#include <fifo.hpp>
#include <call.hpp>

#define AMP_OFF     1

class AMP
{
public:

	static const uint16 MAIN_BUFFER_SIZE		= 4096;
	static const uint16 SPEAK_BUFFER_SIZE		= 3072;

	static const uint16 SAMPLES_MAXIMUM			= 48000;
	static const uint16 SAMPLES_MINIMUM			= 8000;

	static const uint16 SAMPLES_SPEAK			= 4000;
	static const uint16 SPEAK_WAIT_START		= 1024;
	static const uint16 SPEAK_WAIT_END			= 1024;

	/*static const uint8 MAIN_VOLUME				= 1;
	static const uint8 MAIN_VOLUME_WITH_SPEAK	= 3;*/

	static const uint8 MAIN_VOLUME_MIN			= 1;
	static const uint8 MAIN_VOLUME_STD			= 8;
	static const uint8 MAIN_VOLUME_MAX			= 10;

	static uint8 cfgVolume;
	static uint8 cfgVolumeBeep;

	static void Init(void);
	static void DeInit(void);

	static void set(uint16 samples = SAMPLES_MINIMUM);
	static void volume(uint8 value = MAIN_VOLUME_STD);

	static int write(const uint8* data, int length);	/* SAMPLES_MINIMUM - SAMPLES_MAXIMUM samples, use 'set' */
	static int speak(const uint8* data, int length);	/* SAMPLES_SPEAK samples */

private:

	static const uint8 volumeTable[MAIN_VOLUME_MAX - MAIN_VOLUME_MIN + 1];

	static FIFO8* mainBuffer;
	static FIFO8* speakBuffer;

	static sint16 mainSample;
	static sint16 speakSample;
	static uint8 mainVolume;
	static uint8 _mainVolume;
	static bool speakIsWait;
	static uint16 speakWait;
	static bool outEnable;
	static bool outDisable;
	static uint8 disableCounter;
	static uint16 volumeWait;

	//static void __irq_main(void);
	static void __irq_mld(void);
	static void __irq_speak(void);

#if AMP_OFF == 1
	static void outOn();
	static void outOffThread(void* param);
#endif

};

#endif
