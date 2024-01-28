/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 28 мар. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <config.hpp>
#include <amp.hpp>
#include <sirena.hpp>

__attribute__((__section__(".save")))
CONFIG::tSaveCfg CONFIG::saveCfg =
	{
		.ch =
			{/* press  idle  ps dev   btn measure plonce pltoed nowidth nobh   std*/
				{ 500, 1000, 20, 20, false, false, false, false, false, true,  true},
				{ 300, 600,  20, 20, true,  false, false, true,  true,  false, false},
				{ 300, 600,  20, 20, true,  false, false, false, true,  false, false}
			},
		.pinCode = "0000",
		.uartControl = false,
		.logStore = false,
		.sleep = true,
		.volume = 8,
		.volumeBeep = 10,
		.crc = 0x00005CD6
	};

void CONFIG::Init(void)
{
	if (CRC16::calc(&saveCfg, sizeof(tSaveCfg) - 4) == saveCfg.crc)
	{
		uint8 i;
		for (i = 0; i < CH::CHANNELS; i++)
		{
			CH::set(i, &saveCfg.ch[i]);
		}
		strcpy(BT::pinCode, saveCfg.pinCode);
		SIRENA::cfgSleep = saveCfg.sleep;
		AMP::cfgVolume = saveCfg.volume;
		AMP::cfgVolumeBeep = saveCfg.volumeBeep;
		DBGF_NOTICE("CONFIG: config loaded, pin '%s'", BT::pinCode);
		DBG::ReInit(saveCfg.uartControl);
	}
	else
	{
		DBGP_WARNING("CONFIG: error config");
	}
}

void CONFIG::DeInit(void)
{

}

bool CONFIG::save(void)
{
	tSaveCfg* buffer = (tSaveCfg*)malloc(sizeof(tSaveCfg));
	memset(buffer, 0, sizeof(tSaveCfg));
	uint32 i;
	PIN::tCfg* cfg;
	bool flag = false;
	for (i = 0; i < CH::CHANNELS; i++)
	{
		cfg = CH::get(i);
		memcpy(&buffer->ch[i], cfg, sizeof(PIN::tCfg));
		free(cfg);
	}
	strcpy(buffer->pinCode, BT::pinCode);
	buffer->uartControl = DBG::getUartControl();
	buffer->logStore = false;
	buffer->sleep = SIRENA::cfgSleep;
	buffer->volume = AMP::cfgVolume;
	buffer->volumeBeep = AMP::cfgVolumeBeep;
	buffer->crc = CRC16::calc(buffer, sizeof(tSaveCfg) - 4);

	/*printf("CONFIS:");
	for (i = 0; i < sizeof(tSaveCfg); i++)
	{
		printf("%02X", ((uint8*)&saveCfg)[i]);
	}
	putchar('\n');

	printf("CONFIG:");
	for (i = 0; i < sizeof(tSaveCfg); i++)
	{
		printf("%02X", ((uint8*)buffer)[i]);
	}
	putchar('\n');*/

	FLASH_Unlock();
	if (FLASH_ErasePage((uint32)&saveCfg) == FLASH_COMPLETE)
	{
		flag = true;
		for (i = 0; i < sizeof(tSaveCfg); i += 4)
		{
			if (FLASH_ProgramWord((uint32)&saveCfg + i, *((uint32*)((uint32)buffer + i))) != FLASH_COMPLETE)
			{
				flag = false;
				break;
			}
		}
		if (flag)
		{
			DBGF_NOTICE("CONFIG: config saved, crc 0x%08X", (int)buffer->crc);
		}
		else
		{
			DBGP_NOTICE("CONFIG: error save config");
		}
	}
	FLASH_Lock();
	free(buffer);
	return flag;
}
