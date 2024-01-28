/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 28 мар. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_CONFIG_HPP_
#define INC_CONFIG_HPP_ 1

#include <main.hpp>
#include <dbg.hpp>
#include <ch.hpp>
#include <hash.hpp>
#include <bt.hpp>

class CONFIG
{
public:

	static void Init(void);
	static void DeInit(void);

	static bool save(void);

private:

	typedef struct
	{
		PIN::tCfg ch[CH::CHANNELS];
		char pinCode[BT::PINLength + 1];
		bool uartControl;
		bool logStore;
		bool sleep;
		uint8 volume;
		uint8 volumeBeep;
		uint32 crc;
	}
	tSaveCfg;

	static tSaveCfg saveCfg;

};

#endif
