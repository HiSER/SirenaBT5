/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 24 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <bt.hpp>
#include <ch.hpp>
#include <call.hpp>
#include <encoders.hpp>
#include <amp.hpp>
#include <sd.hpp>
#include <fs.hpp>
#include <sirena.hpp>
#include <config.hpp>

extern void callBootCallback(__UNUSED void* param);
ATP(BOOT)
{
	CALL::setTimeout(callBootCallback, 500);
	ANSWER("OK");
}

void SAVEFS_callback(bool result)
{
    BT::write("+SAVEFS:");
    BT::send(ANSWERBOOL(result));
}

ATP(MAIN)
{
	ANSWER("READY");
}

/* <data[1792]:SPEAK_ENCODE> */
ATP(SPEAK)
{
	char* tmp = (char*)malloc(48);
	if (at->argc == 1 && at->argv[0]->length > 0)
	{
		int l = at->argv[0]->length;
		uint8* data = SPEAK_ENCODE::decode(at->argv[0]->data, &l);
		l = AMP::speak(data, l);
		free(data);
		sprintf(tmp, "{\"writed\":%u}", l);
	}
	else
	{
		sprintf(tmp, "{\"encode\":\"" ADDQUOTES(SPEAK_ENCODE) "\",\"samples\":%u}", AMP::SAMPLES_SPEAK);
	}
	ANSWER(tmp);
	free(tmp);
}

/* <index> */
ATP(PLAY)
{
	if (at->argc == 1 && BT::testNumber(at->argv[0], 0, FS::MELODY_MAX - 1))
	{
		SIRENA::prePlay(at->argv[0]->uNumber);
		char* tmp = (char*)malloc(32);
		sprintf(tmp, "{\"index\":%u}", (int)at->argv[0]->uNumber);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

ATP(PLAYSTOP)
{
	if (at->argc == 0)
	{
		SIRENA::preStop();
		ANSWER("OK");
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* no param */
ATP(LIST)
{
	FS::tList* list = FS::list();
	if (list == NULL)
	{
		ANSWER("NULL");
	}
	else
	{
		char* tmp = (char*)malloc(4 * (FS::MELODY_MAX + FS::EVENT_MAX) + 64);
		int l;
		uint8 i;
		bool f;
		l = sprintf(tmp, "{\"melody\":[");
		for (i = 0, f = false; i < list->melody.count; i++)
		{
			if (f)
			{
				tmp[l] = ',';
				l++;
				tmp[l] = '\0';
			}
			else
			{
				f = true;
			}
			l += sprintf(&tmp[l], "%u", list->melody.index[i]);
		}
		l += sprintf(&tmp[l], "],\"event\":[");
		for (i = 0, f = false; i < list->event.count; i++)
		{
			if (f)
			{
				tmp[l] = ',';
				l++;
				tmp[l] = '\0';
			}
			else
			{
				f = true;
			}
			l += sprintf(&tmp[l], "%u", list->event.index[i]);
		}
		l += sprintf(&tmp[l], "],\"melodyMax\":%u", FS::MELODY_MAX);
		l += sprintf(&tmp[l], ",\"eventMax\":%u", FS::EVENT_MAX);
		sprintf(&tmp[l], "}");
		ANSWER(tmp);
		free(tmp);
		free(list);
	}
}

void GETMELODY_callback(FS::tMelody2* melody)
{
	char* tmp;
	if (melody->name == NULL)
	{
		tmp = (char*)malloc(32);
		sprintf(tmp, "{\"index\":%u,\"name\":null}", melody->index);
		BT::write("+GETMELODY:");
		BT::send(tmp);
	}
	else
	{
		int l = -1;
		char* name = BASE64::encode((uint8*)melody->name, &l);
		tmp = (char*)malloc(l + 32);
		sprintf(tmp, "{\"index\":%u,\"name\":\"%s\"}", melody->index, name);
		BT::write("+GETMELODY:");
		BT::send(tmp);
		free(name);
	}
	free(tmp);
}

/* <index> */
ATP(GETMELODY)
{
	if (at->argc == 1 && BT::testNumber(at->argv[0], 0, FS::MELODY_MAX - 1))
	{
		FS::getMelody(at->argv[0]->uNumber, (FS::tCBMelody2)GETMELODY_callback);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <length>;<samples>;<name[<=512]:base64> */
ATP(SETMELODY)
{
	if
	(
		   at->argc == 3
		&& BT::testNumber(at->argv[0], 1, FS::MELODY_LENGTH_MAX)
		&& BT::testNumber(at->argv[1], AMP::SAMPLES_MINIMUM, AMP::SAMPLES_MAXIMUM)
		&& at->argv[2]->length > 0
	)
	{
		int l = at->argv[2]->length;
		char* name = (char*)BASE64::decode(at->argv[2]->data, &l);
		sint8 index = FS::setMelody(at->argv[0]->uNumber, at->argv[1]->uNumber, name);
		//free(name);
		if (index < 0)
		{
			free(name);
			char* tmp = (char*)malloc(32);
			switch (index)
			{
			case -3:
				sprintf(tmp, /*"{\"error\":\"param\"}"*/ "ERROR;PARAM");
				break;
			case -2:
				sprintf(tmp, /*"{\"error\":\"busy\"}"*/ "ERROR;BUSY");
				break;
			case -1:
				sprintf(tmp, /*"{\"error\":\"full\"}"*/ "ERROR;FULL");
				break;
			default:
				//sprintf(tmp, "{\"index\":%i}", index);
				break;
			}
			ANSWER(tmp);
			free(tmp);
		}
		else
		{
		    FS::saveTables(SAVEFS_callback);
		}
	}
	else
	{
		ANSWER("ERROR");
	}
}

void DATAMELODY_callback(FS::tStreamWrite* streamWrite)
{
	if (streamWrite->error)
	{
		BT::write("+DATAMELODY:");
		BT::send("ERROR;WRITE");
	}
	else
	{
		BT::write("+DATAMELODY:");
		BT::send((streamWrite->writed >= streamWrite->length) ? "END" : "OK");
	}
}

/* <data[<=512]:base64> */
ATP(DATAMELODY)
{
	if (at->argc == 1 && at->argv[0]->length > 0)
	{
		int l = at->argv[0]->length;
		uint8* data = BASE64::decode(at->argv[0]->data, &l);
		if (!FS::dataMelody(data, l, DATAMELODY_callback))
		{
			ANSWER("ERROR;DATA");
		}
		free(data);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* no param */
ATP(ENDMELODY)
{
	ANSWER(ANSWERBOOL(FS::endMelody()));
}

/* <index> */
ATP(DELETEMELODY)
{
	if (at->argc == 1 && BT::testNumber(at->argv[0], 0, FS::MELODY_MAX - 1))
	{
		char* tmp = (char*)malloc(32);
		if (FS::deleteMelody(at->argv[0]->uNumber))
		{
			sprintf(tmp, "{\"index\":%u,\"delete\":true}", (int)at->argv[0]->uNumber);
		}
		else
		{
			sprintf(tmp, "{\"index\":%u,\"delete\":true}", (int)at->argv[0]->uNumber);
		}
		ANSWER(tmp);
		free(tmp);
		FS::saveTables(SAVEFS_callback);
	}
	else
	{
		ANSWER("ERROR");
	}
}

ATP(CLEARMELODY)
{
	FS::melodyTableClear();
	FS::saveTables(SAVEFS_callback);
	ANSWER("OK");
}

/* <index> */
ATP(GETEVENT)
{
	if (at->argc == 1 && BT::testNumber(at->argv[0], 0, FS::EVENT_MAX - 1))
	{
		FS::tEvent* event = FS::getEvent(at->argv[0]->uNumber);
		char* tmp = (char*)malloc(256);
		int l;
		uint8 i;
		bool f;
		l = sprintf(tmp, "{\"index\":%u", (uint8)at->argv[0]->uNumber);
		if (event->chEvent.lengthType == PIN::eptNull)
		{
			l += sprintf(&tmp[l], ",\"type\":null");
		}
		else
		{
			if (event->chEvent.lengthType > PIN::eptNull)
			{
				l += sprintf(&tmp[l], ",\"type\":\"pulse\",\"pulse\":[");
				for (i = 0, f = false; i < event->chEvent.lengthType; i++)
				{
					if (f)
					{
						tmp[l] = ',';
						l++;
						tmp[l] = '\0';
					}
					else
					{
						f = true;
					}
					l += sprintf(&tmp[l], "{\"width\":%u,\"count\":%u}", event->chEvent.pulseType[i].width, event->chEvent.pulseType[i].count);
				}
				l += sprintf(&tmp[l], "]");
			}
			else if (event->chEvent.lengthType == PIN::eptPress)
			{
				l += sprintf(&tmp[l], ",\"type\":\"press\"");
			}
			else if (event->chEvent.lengthType == PIN::eptCyclic)
			{
				l += sprintf(&tmp[l], ",\"type\":\"cyclic\"");
			}
			else
			{
				l += sprintf(&tmp[l], ",\"type\":\"error\"");
			}
			l += sprintf(&tmp[l], ",\"chIndex\":%i", event->chEvent.index);
			l += sprintf(&tmp[l], ",\"melodyIndex\":%i", event->melodyIndex);
			l += sprintf(&tmp[l], ",\"poweroff\":%s", ((event->poweroff) ? "true" : "false"));
			l += sprintf(&tmp[l], ",\"volume\":%u", event->volume);
			l += sprintf(&tmp[l], ",\"noPlayAfter\":%i", event->noPlayAfter);
		}
		l += sprintf(&tmp[l], "}");
		ANSWER(tmp);
		free(tmp);
		free(event);
		//DBGF("GETEVENT: length %i", l);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <index>;<melodyIndex>;<poweroff>;<chIndex>;<volume>;<noplayafter>;<type>[;<width>;<count>[;...]] */
ATP(SETEVENT)
{
	if
	(
		   at->argc >= 7
		&& BT::testNumber(at->argv[0], 0, FS::EVENT_MAX - 1)
		&& BT::testNumber(at->argv[1], -1, FS::MELODY_MAX - 1)
		&& BT::testNumber(at->argv[2], 0, 1)
		&& BT::testNumber(at->argv[3], 1, 3)
		&& BT::testNumber(at->argv[4], AMP::MAIN_VOLUME_MIN, AMP::MAIN_VOLUME_MAX)
		&& BT::testNumber(at->argv[5], -1, FS::EVENT_MAX - 1)
	)
	{
		FS::tEvent* event = (FS::tEvent*)malloc(sizeof(FS::tEvent));
		uint8 i, n;
		bool f;
		event->melodyIndex = at->argv[1]->uNumber;
		event->poweroff = (at->argv[2]->uNumber != 0);
		event->chEvent.index = at->argv[3]->uNumber;
		event->volume = at->argv[4]->uNumber;
		event->noPlayAfter = at->argv[5]->sNumber;
		if (strcmp(at->argv[6]->data, "null") == 0)
		{
			event->chEvent.lengthType = PIN::eptNull;
		}
		else if (strcmp(at->argv[6]->data, "press") == 0)
		{
			event->chEvent.lengthType = PIN::eptPress;
		}
		else if (strcmp(at->argv[6]->data, "cyclic") == 0)
		{
			event->chEvent.lengthType = PIN::eptCyclic;
		}
		else if (BT::testNumber(at->argv[6], 1, PIN::PULSES_TYPE_MAXIMUM) && (at->argc - 7) >= ((int)at->argv[6]->sNumber * 2))
		{
			event->chEvent.lengthType = (PIN::ePulseType)at->argv[6]->uNumber;
			for (n = 0, i = 7, f = true; n < at->argv[6]->uNumber; n++, i += 2)
			{
				if
				(
					!(
						   BT::testNumber(at->argv[i], /*PIN::ANTI_BOUNCE_HIGH*/ 1, PIN::TIME_MAXIMUM)
						&& BT::testNumber(at->argv[i + 1], 1, PIN::PULSES_MAXIMUM)
					)
				)
				{
					f = false;
					break;
				}
				event->chEvent.pulseType[n].width = at->argv[i]->sNumber;
				event->chEvent.pulseType[n].count = at->argv[i + 1]->sNumber;
			}
			if (!f)
			{
				ANSWER("ERROR;PULSE");
				free(event);
				event = NULL;
			}
		}
		else
		{
			ANSWER("ERROR;TYPE");
			free(event);
			event = NULL;
		}
		if (event != NULL)
		{
			char* tmp = (char*)malloc(32);
			if (FS::setEvent(at->argv[0]->uNumber, event))
			{
				sprintf(tmp, "{\"index\":%u,\"change\":true}", (int)at->argv[0]->uNumber);
			}
			else
			{
				sprintf(tmp, "{\"index\":%u,\"change\":false}", (int)at->argv[0]->uNumber);
			}
			ANSWER(tmp);
			free(tmp);
			free(event);
			FS::saveTables(SAVEFS_callback);
		}
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <index> */
ATP(DELETEEVENT)
{
	if (at->argc == 1 && BT::testNumber(at->argv[0], 0, FS::EVENT_MAX - 1))
	{
		char* tmp = (char*)malloc(32);
		if (FS::deleteEvent(at->argv[0]->uNumber))
		{
			sprintf(tmp, "{\"index\":%u,\"delete\":true}", (int)at->argv[0]->uNumber);
		}
		else
		{
			sprintf(tmp, "{\"index\":%u,\"delete\":false}", (int)at->argv[0]->uNumber);
		}
		ANSWER(tmp);
		free(tmp);
		FS::saveTables(SAVEFS_callback);
	}
	else
	{
		ANSWER("ERROR");
	}
}

ATP(CLEAREVENT)
{
	FS::eventTableClear();
	FS::saveTables(SAVEFS_callback);
	ANSWER("OK");
}

/* no param */
ATP(FS)
{
	ANSWER((FS::hasReady() ? "READY" : "BUSY"));
}

/* no param */
ATP(SAVEFS)
{
	if (!FS::saveTables(SAVEFS_callback))
	{
		ANSWER("BUSY");
	}
}

/* no param */
ATP(GETCONFIG)
{
	char* tmp = (char*)malloc(640);
	PIN::tCfg* config;
	int l;
	uint8 i;
	bool f;
	l = sprintf(tmp, "[");
	for (i = 0, f = false; i < CH::CHANNELS; i++)
	{
		config = CH::get(i);
		if (f)
		{
			tmp[l] = ',';
			l++;
			tmp[l] = '\0';
		}
		else
		{
			f = true;
		}
		l += sprintf(&tmp[l], "{\"pressMax\":%u", config->pressMax);
		l += sprintf(&tmp[l], ",\"idleMax\":%u", config->idleMax);
		l += sprintf(&tmp[l], ",\"pulsesMax\":%u", config->pulsesMax);
		l += sprintf(&tmp[l], ",\"deviation\":%u", config->deviation);
		l += sprintf(&tmp[l], ",\"button\":%s", ((config->button) ? "true" : "false"));
		l += sprintf(&tmp[l], ",\"measure\":%s", ((config->measure) ? "true" : "false"));
		l += sprintf(&tmp[l], ",\"playOnce\":%s", ((config->playOnce) ? "true" : "false"));
		l += sprintf(&tmp[l], ",\"playToEnd\":%s", ((config->playToEnd) ? "true" : "false"));
		l += sprintf(&tmp[l], ",\"noWidth\":%s", ((config->noWidth) ? "true" : "false"));
		l += sprintf(&tmp[l], ",\"noBounceHigh\":%s", ((config->noBounceHigh) ? "true" : "false"));
		l += sprintf(&tmp[l], ",\"standard\":%s}", ((config->standard) ? "true" : "false"));
		free(config);
	}
	l += sprintf(&tmp[l], ",{\"volume\":%u", AMP::cfgVolume);
	l += sprintf(&tmp[l], ",\"volumeBeep\":%u", AMP::cfgVolumeBeep);
	l += sprintf(&tmp[l], ",\"sleep\":%s}", ((SIRENA::cfgSleep) ? "true" : "false"));
	l += sprintf(&tmp[l], "]");
	ANSWER(tmp);
	free(tmp);
	//DBGF("GETCONFIG: length %i", l);
}

/* <index>;<pressMax>;<idleMax>;<pulsesMax>;<deviation>;<button>;<measure>;<playOnce>;<playToEnd>;<noWidth>;<noBounceHigh> */
/*ATP(SETCONFIG)
{
	if
	(
		   at->argc == 7
		&& BT::testNumber(at->argv[0], 0, CH::CHANNELS - 1)
		&& BT::testNumber(at->argv[1], 1, PIN::TIME_MAXIMUM)
		&& BT::testNumber(at->argv[2], 1, PIN::TIME_MAXIMUM)
		&& BT::testNumber(at->argv[3], 1, PIN::PULSES_MAXIMUM)
		&& BT::testNumber(at->argv[4], 1, PIN::DEVIATION_MAXIMUM)
		&& BT::testNumber(at->argv[5], 0, 1)
		&& BT::testNumber(at->argv[6], 0, 1)
		&& BT::testNumber(at->argv[7], 0, 1)
		&& BT::testNumber(at->argv[8], 0, 1)
		&& BT::testNumber(at->argv[9], 0, 1)
		&& BT::testNumber(at->argv[10], 0, 1)
	)
	{
		PIN::tCfg* config = CH::get(at->argv[0]->uNumber);
		config->pressMax		= at->argv[1]->uNumber;
		config->idleMax			= at->argv[2]->uNumber;
		config->pulsesMax		= at->argv[3]->uNumber;
		config->deviation		= at->argv[4]->uNumber;
		config->button			= (at->argv[5]->uNumber != 0);
		config->measure			= (at->argv[6]->uNumber != 0);
		config->playOnce		= (at->argv[7]->uNumber != 0);
		config->playToEnd		= (at->argv[8]->uNumber != 0);
		config->noWidth			= (at->argv[9]->uNumber != 0);
		config->noBounceHigh	= (at->argv[10]->uNumber != 0);
		CH::set(at->argv[0]->uNumber, config);
		free(config);
		char* tmp = (char*)malloc(32);
		sprintf(tmp, "{\"index\":%u}", (int)at->argv[0]->uNumber);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}*/

/* <index>;<button> */
ATP(SETBUTTON)
{
	if
	(
		   at->argc == 2
		&& BT::testNumber(at->argv[0], 0, CH::CHANNELS - 1)
		&& BT::testNumber(at->argv[1], 0, 1)
	)
	{
		PIN::tCfg* config = CH::get(at->argv[0]->uNumber);
		config->button		= (at->argv[1]->uNumber != 0);
		if (config->button) config->measure = false;
		CH::set(at->argv[0]->uNumber, config);
		char* tmp = (char*)malloc(64);
		int l = sprintf(tmp, "{\"index\":%u", (int)at->argv[0]->uNumber);
		l += sprintf(&tmp[l], ",\"button\":%s", ((config->button) ? "true" : "false"));
		l += sprintf(&tmp[l], ",\"measure\":%s}", ((config->measure) ? "true" : "false"));
		free(config);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <index>;<measure> */
ATP(SETMEASURE)
{
	if
	(
		   at->argc == 2
		&& BT::testNumber(at->argv[0], 0, CH::CHANNELS - 1)
		&& BT::testNumber(at->argv[1], 0, 1)
	)
	{
		PIN::tCfg* config = CH::get(at->argv[0]->uNumber);
		config->measure		= (at->argv[1]->uNumber != 0);
		if (config->measure) config->button = false;
		CH::set(at->argv[0]->uNumber, config);
		char* tmp = (char*)malloc(64);
		int l = sprintf(tmp, "{\"index\":%u", (int)at->argv[0]->uNumber);
		l += sprintf(&tmp[l], ",\"button\":%s", ((config->button) ? "true" : "false"));
		l += sprintf(&tmp[l], ",\"measure\":%s}", ((config->measure) ? "true" : "false"));
		free(config);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <index>;<pressmax> */
ATP(SETPRESSMAX)
{
	if
	(
		   at->argc == 2
		&& BT::testNumber(at->argv[0], 0, CH::CHANNELS - 1)
		&& BT::testNumber(at->argv[1], 1, PIN::TIME_MAXIMUM)
	)
	{
		PIN::tCfg* config = CH::get(at->argv[0]->uNumber);
		config->pressMax		= at->argv[1]->uNumber;
		CH::set(at->argv[0]->uNumber, config);
		char* tmp = (char*)malloc(48);
		int l = sprintf(tmp, "{\"index\":%u", (int)at->argv[0]->uNumber);
		l += sprintf(&tmp[l], ",\"pressMax\":%u}", config->pressMax);
		free(config);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <index>;<idlemax> */
ATP(SETIDLEMAX)
{
	if
	(
		   at->argc == 2
		&& BT::testNumber(at->argv[0], 0, CH::CHANNELS - 1)
		&& BT::testNumber(at->argv[1], 1, PIN::TIME_MAXIMUM)
	)
	{
		PIN::tCfg* config = CH::get(at->argv[0]->uNumber);
		config->idleMax		= at->argv[1]->uNumber;
		CH::set(at->argv[0]->uNumber, config);
		char* tmp = (char*)malloc(48);
		int l = sprintf(tmp, "{\"index\":%u", (int)at->argv[0]->uNumber);
		l += sprintf(&tmp[l], ",\"idleMax\":%u}", config->idleMax);
		free(config);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <index>;<pulsesmax> */
ATP(SETPULSESMAX)
{
	if
	(
		   at->argc == 2
		&& BT::testNumber(at->argv[0], 0, CH::CHANNELS - 1)
		&& BT::testNumber(at->argv[1], 1, PIN::PULSES_MAXIMUM)
	)
	{
		PIN::tCfg* config = CH::get(at->argv[0]->uNumber);
		config->pulsesMax		= at->argv[1]->uNumber;
		CH::set(at->argv[0]->uNumber, config);
		char* tmp = (char*)malloc(48);
		int l = sprintf(tmp, "{\"index\":%u", (int)at->argv[0]->uNumber);
		l += sprintf(&tmp[l], ",\"pulsesMax\":%u}", config->pulsesMax);
		free(config);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <index>;<deviation> */
ATP(SETDEVIATION)
{
	if
	(
		   at->argc == 2
		&& BT::testNumber(at->argv[0], 0, CH::CHANNELS - 1)
		&& BT::testNumber(at->argv[1], 1, PIN::DEVIATION_MAXIMUM)
	)
	{
		PIN::tCfg* config = CH::get(at->argv[0]->uNumber);
		config->deviation		= at->argv[1]->uNumber;
		CH::set(at->argv[0]->uNumber, config);
		char* tmp = (char*)malloc(48);
		int l = sprintf(tmp, "{\"index\":%u", (int)at->argv[0]->uNumber);
		l += sprintf(&tmp[l], ",\"deviation\":%u}", config->deviation);
		free(config);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <index>;<playOnce> */
ATP(SETPLAYONCE)
{
	if
	(
		   at->argc == 2
		&& BT::testNumber(at->argv[0], 0, CH::CHANNELS - 1)
		&& BT::testNumber(at->argv[1], 0, 1)
	)
	{
		int l;
		PIN::tCfg* config = CH::get(at->argv[0]->uNumber);
		config->playOnce = (at->argv[1]->uNumber != 0);
		CH::set(at->argv[0]->uNumber, config);
		char* tmp = (char*)malloc(48);
		l = sprintf(tmp, "{\"index\":%u", (int)at->argv[0]->uNumber);
		l += sprintf(&tmp[l], ",\"playOnce\":%s}", ((config->playOnce) ? "true" : "false"));
		free(config);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <index>;<playToEnd> */
ATP(SETPLAYTOEND)
{
	if
	(
		   at->argc == 2
		&& BT::testNumber(at->argv[0], 0, CH::CHANNELS - 1)
		&& BT::testNumber(at->argv[1], 0, 1)
	)
	{
		int l;
		PIN::tCfg* config = CH::get(at->argv[0]->uNumber);
		config->playToEnd = (at->argv[1]->uNumber != 0);
		CH::set(at->argv[0]->uNumber, config);
		char* tmp = (char*)malloc(48);
		l = sprintf(tmp, "{\"index\":%u", (int)at->argv[0]->uNumber);
		l += sprintf(&tmp[l], ",\"playToEnd\":%s}", ((config->playToEnd) ? "true" : "false"));
		free(config);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <index>;<noWidth> */
ATP(SETNOWIDTH)
{
	if
	(
		   at->argc == 2
		&& BT::testNumber(at->argv[0], 0, CH::CHANNELS - 1)
		&& BT::testNumber(at->argv[1], 0, 1)
	)
	{
		int l;
		PIN::tCfg* config = CH::get(at->argv[0]->uNumber);
		config->noWidth = (at->argv[1]->uNumber != 0);
		CH::set(at->argv[0]->uNumber, config);
		char* tmp = (char*)malloc(48);
		l = sprintf(tmp, "{\"index\":%u", (int)at->argv[0]->uNumber);
		l += sprintf(&tmp[l], ",\"noWidth\":%s}", ((config->noWidth) ? "true" : "false"));
		free(config);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <index>;<noBounceHigh> */
ATP(SETNOBOUNCEHIGH)
{
	if
	(
		   at->argc == 2
		&& BT::testNumber(at->argv[0], 0, CH::CHANNELS - 1)
		&& BT::testNumber(at->argv[1], 0, 1)
	)
	{
		int l;
		PIN::tCfg* config = CH::get(at->argv[0]->uNumber);
		config->noBounceHigh = (at->argv[1]->uNumber != 0);
		CH::set(at->argv[0]->uNumber, config);
		char* tmp = (char*)malloc(48);
		l = sprintf(tmp, "{\"index\":%u", (int)at->argv[0]->uNumber);
		l += sprintf(&tmp[l], ",\"noBounceHigh\":%s}", ((config->noBounceHigh) ? "true" : "false"));
		free(config);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <volume> */
ATP(SETVOLUME)
{
	if
	(
		   at->argc == 1
		&& BT::testNumber(at->argv[0], AMP::MAIN_VOLUME_MIN, AMP::MAIN_VOLUME_MAX)
	)
	{
		AMP::cfgVolume = at->argv[0]->uNumber;
		char* tmp = (char*)malloc(48);
		sprintf(tmp, "{\"index\":3,\"volume\":%u}", AMP::cfgVolume);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <volume> */
ATP(SETVOLUMEBEEP)
{
	if
	(
		   at->argc == 1
		&& BT::testNumber(at->argv[0], AMP::MAIN_VOLUME_MIN, AMP::MAIN_VOLUME_MAX)
	)
	{
		AMP::cfgVolumeBeep = at->argv[0]->uNumber;
		char* tmp = (char*)malloc(48);
		sprintf(tmp, "{\"index\":3,\"volumeBeep\":%u}", AMP::cfgVolumeBeep);
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <sleep> */
ATP(SETSLEEP)
{
	if
	(
		   at->argc == 1
		&& BT::testNumber(at->argv[0], 0, 1)
	)
	{
		SIRENA::cfgSleep = (at->argv[0]->uNumber != 0);
		char* tmp = (char*)malloc(48);
		sprintf(tmp, "{\"index\":3,\"sleep\":%s}", ((SIRENA::cfgSleep) ? "true" : "false"));
		ANSWER(tmp);
		free(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* no param */
ATP(GETUART)
{
	char* tmp = (char*)malloc(8);
	sprintf(tmp, "%s", ((DBG::getUartControl()) ? "1" : "0"));
	ANSWER(tmp);
	free(tmp);
}

/* <bool> */
ATP(SETUART)
{
	if (at->argc == 1 && at->argv[0]->isNumber)
	{
		DBG::ReInit(at->argv[0]->uNumber != 0);
		char* tmp = (char*)malloc(8);
		sprintf(tmp, "%s", ((DBG::getUartControl()) ? "1" : "0"));
		ANSWER(tmp);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* <index>;<standard> */
ATP(SETSTANDARD)
{
    if
    (
           at->argc == 2
        && BT::testNumber(at->argv[0], 0, CH::CHANNELS - 1)
        && BT::testNumber(at->argv[1], 0, 1)
    )
    {
        int l;
        PIN::tCfg* config = CH::get(at->argv[0]->uNumber);
        config->standard = (at->argv[1]->uNumber != 0);
        CH::set(at->argv[0]->uNumber, config);
        char* tmp = (char*)malloc(48);
        l = sprintf(tmp, "{\"index\":%u", (int)at->argv[0]->uNumber);
        l += sprintf(&tmp[l], ",\"standard\":%s}", ((config->standard) ? "true" : "false"));
        free(config);
        ANSWER(tmp);
        free(tmp);
    }
    else
    {
        ANSWER("ERROR");
    }
}

/* <pin[4]> */
ATP(PINCODE)
{
	if (at->argc == 1 && at->argv[0]->isNumber && at->argv[0]->length == BT::PINLength)
	{
		strcpy(BT::pinCode, at->argv[0]->data);
		ANSWER("OK");
	}
	else if (at->argc == 0)
	{
		ANSWER(BT::pinCode);
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* no param */
ATP(SAVECONFIG)
{
	ANSWER(ANSWERBOOL(CONFIG::save()));
}

/* no param */
ATP(INFO)
{
	char* tmp = (char*)malloc(256);
	char* serial = SERIAL::get();
	sprintf(tmp,
		"{\"fsReady\":%s,\"version\":%s,\"build\":%s,\"pincode\":\"%s\",\"speakSamples\":%u,\"serial\":\"%s\"}",
		(FS::hasReady() ? "true" : "false"),
		ADDQUOTES(VERSION), ADDQUOTES(__BUILD__),
		BT::pinCode,
		AMP::SAMPLES_SPEAK,
		serial);
	ANSWER(tmp);
	free(tmp);
	free(serial);
}

/* <data[128]> */
ATP(CHECK)
{
	bool error = true;
	char* serial;
	char* h;
	uint8* e;
	uint8* d;
	int l;
	if (at->argc == 1 && at->argv[0]->length == 128)
	{
		//DBGF("CHECK: < %s", at->argv[0]->data);
		serial = SERIAL::get();
		l = at->argv[0]->length;
		d = HEX::decode(at->argv[0]->data, &l);
		if (l == 64)
		{
			if (CHECK::decode(d, serial, 77))
			{
				e = CHECK::encode(serial, 12);
				l = 64;
				h = HEX::encode(e, &l);
				//DBGF("CHECK: > %s", h);
				ANSWER(h);
				free(h);
				free(e);
				error = false;
				//DBGP("CHECK: ok");
			}
			else
			{
				//DBGP("CHECK: error serial");
			}
		}
		else
		{
			//DBGP("CHECK: error HEX");
		}
		free(d);
		free(serial);
	}
	if (error)
	{
		e = CHECK::encodeRandom();
		l = 64;
		h = HEX::encode(e, &l);
		ANSWER(h);
		free(h);
		free(e);
		//DBGP("CHECK: error");
	}
}

/* no param */
ATP(EMPTY)
{
	ANSWER(ANSWERBOOL(at->argc == 0));
}

/* no param */
ATP(ATI)
{
	ANSWER("Device;" BUILD);
	ANSWER("Build;" ADDQUOTES(VERSION) ";" ADDQUOTES(__BUILD__));
	ANSWER("Manufacturer;BeepHorn");
	ANSWER("Author;Artem Litvin;sirenabt5@artemlitvin.com");
}

/* no param */
ATP(MEM)
{
	char* tmp = (char*)malloc(32);
	sprintf
		(
			tmp, "%u;%u;%u;%u;%u;%u",
			getFreeHeap(), getMinimumEverFreeHeap(),
			CALL::getCalls(), CALL::getMaximumCalls(),
			SD::getTasks(), SD::getMaximumTasks()
		);
	ANSWER(tmp);
	free(tmp);
}

/*ATP(TEST)
{
	char* tmp = (char*)malloc(256);
	uint16 i;
	for (i = 0; i < at->argc; i++)
	{
		sprintf
			(
				tmp, "%u;%s;%i;%u;%u;%s",
				i,
				STRBOOL(at->argv[i]->isNumber),
				(int)at->argv[i]->sNumber,
				(int)at->argv[i]->uNumber,
				at->argv[i]->length,
				at->argv[i]->data
			);
		ANSWER(tmp);
	}
	ANSWER("OK");
	free(tmp);
}

ATP(TEST2)
{
	ANSWER("FREEZE");
	DBGP_FATAL("----- FREEZE -----\n\n");
	LOOP {}
}*/

/*ATP(HELP)
{
	const BT::tListAT* list = &BT::listAT[0];
	int length;
	while (list->command != NULL)
	{
		length = strlen(list->command);
		if (length > 0 && list->command[length - 1] == '?')
		{
			ANSWER(list->command);
		}
		list++;
	}
}

ATP(SPEAKq)
{
	BT::send("? SPEAK=<buffer[1024-2048]:encode>");
	BT::send("? +SPEAK:{\"writed\":?}");
	BT::send("? SPEAK");
	BT::send("? +SPEAK:{\"encode\":?,\"samples\":?}");
}

ATP(PLAYq)
{
	BT::send("? PLAY=<melodyIndex>");
	BT::send("? +PLAY:{\"index\":?}");
	BT::send("? +PLAY:OK|ERROR");
}

ATP(PLAYSTOPq)
{
	BT::send("? PLAYSTOP");
	BT::send("? +PLAYSTOP:OK");
}

ATP(LISTq)
{
	BT::send("? LIST");
	BT::send("? +LIST:{\"melody\":[?],\"event\":[?],\"melodyMax\":?,\"eventMax\":?}");
}

ATP(GETMELODYq)
{
	BT::send("? GETMELODY=<melodyIndex>");
	BT::send("? +GETMELODY:{\"index\":?,\"name\":null}");
	BT::send("? +GETMELODY:{\"index\":?,\"name\":<?:base64>}");
	BT::send("? +GETMELODY:ERROR");
}

ATP(SETMELODYq)
{
	BT::send("? SETMELODY=<lengthWave>;<samplesWave>;<name[<=512]:base64>");
	BT::send("? +SETMELODY:{\"index\":?}");
	BT::send("? +SETMELODY:ERROR[;PARAM|BUSY|FULL]}");
}

ATP(DATAMELODYq)
{
	BT::send("? DATAMELODY=<buffer[<=512]:base64>");
	BT::send("? +DATAMELODY:ERROR[;DATA|WRITE]");
	BT::send("? +DATAMELODY:OK|END");
}

ATP(ENDMELODYq)
{
	BT::send("? ENDMELODY");
	BT::send("? +ENDMELODY:OK|ERROR");
}

ATP(DELETEMELODYq)
{
	BT::send("? DELETEMELODY=<melodyIndex>");
	BT::send("? +DELETEMELODY:{\"index\":?,\"delete\":true|false}");
	BT::send("? +DELETEMELODY:ERROR");
}

ATP(CLEARMELODYq)
{
	BT::send("? CLEARMELODY");
	BT::send("? +CLEARMELODY:OK");
}

ATP(GETEVENTq)
{
	BT::send("? GETEVENT=<eventIndex>");
	BT::send("? +GETEVENT:{\"index\":?,\"type\":null}");
	BT::send("? +GETEVENT:{\"index\":?,...,\"chIndex\":?,\"melodyIndex\":?,\"poweroff\":true|false\"}");
	BT::send("? +GETEVENT:{...,\"type\":\"press\",...}");
	BT::send("? +GETEVENT:{...,\"type\":\"cyclic\",...}");
	BT::send("? +GETEVENT:{...,\"type\":\"release\",...}");
	BT::send("? +GETEVENT:{...,\"type\":\"error\"},...");
	BT::send("? +GETEVENT:{...,\"type\":\"pulse\",\"pulse\":[{\"width\":?,\"count\":?}<,...>],...}");
	BT::send("? +GETEVENT:ERROR");
}

ATP(SETEVENTq)
{
	BT::send("? SETEVENT=<eventIndex>;<melodyIndex>;<poweroff>;<chIndex>;<type>[;<width>;<count>[;...]]");
	BT::send("? +SETEVENT:{\"index\":?,\"change\":true|false}");
	BT::send("? +SETEVENT:ERROR[;TYPE|PULSE]");
}

ATP(DELETEEVENTq)
{
	BT::send("? DELETEEVENT=<eventIndex>");
	BT::send("? +DELETEEVENT:{\"index\":?,\"delete\":true|false}");
	BT::send("? +DELETEEVENT:ERROR");
}

ATP(CLEAREVENTq)
{
	BT::send("? CLEAREVENT");
	BT::send("? +CLEAREVENT:OK");
}

ATP(SAVEFSq)
{
	BT::send("? SAVEFS");
	BT::send("? +SAVEFS:OK|BUSY|ERROR");
}

ATP(FSq)
{
	BT::send("? FS");
	BT::send("? +FS:READY|BUSY");
}

ATP(GETCONFIGq)
{
	BT::send("? GETCONFIG");
	BT::send("? +GETCONFIG:[{\"pressMax\":?,\"idleMax\":?,\"pulsesMax\":?,\"deviation\":?,\"button\":true|false,\"measure\":true|false}<,...>]");
}

ATP(SETCONFIGq)
{
	BT::send("? SETCONFIG=<chIndex>;<pressMax>;<idleMax>;<pulsesMax>;<deviation>;<button>;<measure>");
	BT::send("? +SETCONFIG:{\"index\":?}");
	BT::send("? +SETCONFIG:ERROR");
}

ATP(SETBUTTONq)
{
	BT::send("? SETBUTTON=<chIndex>;<button>");
	BT::send("? +SETBUTTON:{\"index\":?}");
	BT::send("? +SETBUTTON:ERROR");
}

ATP(SETMEASUREq)
{
	BT::send("? SETMEASUR=<chIndex>;<measure>");
	BT::send("? +SETMEASUR:{\"index\":?}");
	BT::send("? +SETMEASUR:ERROR");
}

ATP(PINCODEq)
{
	BT::send("? PINCODE");
	BT::send("? +PINCODE:xxxx");
	BT::send("? PINCODE=xxxx");
	BT::send("? +PINCODE:OK|ERROR");
}

ATP(SAVECONFIGq)
{
	BT::send("? SAVECONFIG");
	BT::send("? +SAVECONFIG:OK|ERROR");
}

ATP(EVENTq)
{
	BT::send("? +EVENT:{\"index\":?,\"type\":null}");
	BT::send("? +EVENT:{\"index\":?,\"type\":\"press\"}");
	BT::send("? +EVENT:{\"index\":?,\"type\":\"cyclic\"}");
	BT::send("? +EVENT:{\"index\":?,\"type\":\"release\"}");
	BT::send("? +EVENT:{\"index\":?,\"type\":\"error\"}");
	BT::send("? +EVENT:{\"index\":?,\"type\":\"errMeasure\"}");
	BT::send("? +EVENT:{\"index\":?,\"type\":\"pulse\",\"pulse\":[{\"width\":?,\"count\":?}<,...>]}");
	BT::send("? +EVENT:{\"index\":?,\"type\":\"measure\",\"measure\":{\"width\":?,\"idle\":?}}");
}*/

const BT::tListAT BT::listAT[] =
	{
		/* main commands */
		AT("SPEAK",			SPEAK)
		AT("PLAY",			PLAY)
		AT("PLAYSTOP",		PLAYSTOP)
		AT("LIST",			LIST)
		AT("GETMELODY",		GETMELODY)
		AT("SETMELODY",		SETMELODY)
		AT("DATAMELODY",	DATAMELODY)
		AT("ENDMELODY",		ENDMELODY)
		AT("DELETEMELODY",	DELETEMELODY)
		AT("CLEARMELODY",	CLEARMELODY)
		AT("GETEVENT",		GETEVENT)
		AT("SETEVENT",		SETEVENT)
		AT("DELETEEVENT",	DELETEEVENT)
		AT("CLEAREVENT",	CLEAREVENT)
		AT("SAVEFS",		SAVEFS)
		AT("FS",			FS)			/* auto event busy */
		AT("GETCONFIG",		GETCONFIG)
		/*AT("SETCONFIG",		SETCONFIG)*/
		AT("SETBUTTON",		SETBUTTON)
		AT("SETMEASURE",	SETMEASURE)
		AT("SETPRESSMAX",	SETPRESSMAX)
		AT("SETIDLEMAX",	SETIDLEMAX)
		AT("SETPULSESMAX",	SETPULSESMAX)
		AT("SETDEVIATION",	SETDEVIATION)
		AT("SETPLAYONCE",	SETPLAYONCE)
		AT("SETPLAYTOEND",	SETPLAYTOEND)
		AT("SETNOWIDTH",	SETNOWIDTH)
		AT("SETNOBOUNCEHIGH",	SETNOBOUNCEHIGH)
		AT("SETVOLUME",		SETVOLUME)
		AT("SETVOLUMEBEEP",	SETVOLUMEBEEP)
		AT("SETSLEEP",		SETSLEEP)
		AT("GETUART",		GETUART)
		AT("SETUART",		SETUART)
		AT("SETSTANDARD",   SETSTANDARD)
		AT("PINCODE",		PINCODE)
		AT("SAVECONFIG",	SAVECONFIG)
		AT("INFO",			INFO)
		AT("CHECK",			CHECK)

		/* BT answers */
		ATI("+READY",		BT::readyCmd)
		ATI("+VERSION",		BT::testAT)
		ATI("CONNECTED",	BT::connectedCmd)
		ATI("+DISC",		BT::disconnectedCmd)

		/* diagnostics */
		AT("AT",			EMPTY)
		AT("ATI",			ATI)
		AT("MEM",			MEM)
		AT("^BOOT",			BOOT)
		AT("^MAIN",			MAIN)
		/*AT("TEST",			TEST)
		AT("TEST2",			TEST2)*/

		/* events */
		AT("EVENT",			EMPTY)		/* auto event */

		/* BT answers no used */
		AT("+CONNECTING",	NULL)
		AT("+PAIRABLE",		NULL)
		AT("+ROLE",			NULL)
		AT("+PIN",			NULL)
		AT("+NAME",			NULL)
		AT("+BAUD",			NULL)
		AT("OK",			NULL)
		AT("+OK",			NULL)
		AT("ERROR",			NULL)
		AT("",				NULL)

		/* helps */
		/*AT("HELP",			HELP)
		AT("SPEAK?",		SPEAKq)
		AT("PLAY?",			PLAYq)
		AT("PLAYSTOP?",		PLAYSTOPq)
		AT("LIST?",			LISTq)
		AT("GETMELODY?",	GETMELODYq)
		AT("SETMELODY?",	SETMELODYq)
		AT("DATAMELODY?",	DATAMELODYq)
		AT("ENDMELODY?",	ENDMELODYq)
		AT("DELETEMELODY?",	DELETEMELODYq)
		AT("CLEARMELODY?",	CLEARMELODYq)
		AT("GETEVENT?",		GETEVENTq)
		AT("SETEVENT?",		SETEVENTq)
		AT("DELETEEVENT?",	DELETEEVENTq)
		AT("CLEAREVENT?",	CLEAREVENTq)
		AT("SAVEFS?",		SAVEFSq)
		AT("FS?",			FSq)
		AT("GETCONFIG?",	GETCONFIGq)
		AT("SETCONFIG?",	SETCONFIGq)
		AT("SETBUTTON?",	SETBUTTONq)
		AT("SETMEASURE?",	SETMEASUREq)
		AT("PINCODE?",		PINCODEq)
		AT("SAVECONFIG?",	SAVECONFIGq)
		AT("EVENT?",		EVENTq)*/

		AT_NULL
	};






/* no param */
ATP2(AT)
{
	ANSWER2("OK");
}

/* <index> */
ATP2(PLAY)
{
	if (at->argc == 1 && BT::testNumber((BT::tArgAT*)at->argv[0], 0, FS::MELODY_MAX - 1))
	{
		SIRENA::prePlay(at->argv[0]->uNumber);
		char* tmp = (char*)malloc(8);
		sprintf(tmp, "%u", (int)at->argv[0]->uNumber);
		ANSWER2(tmp);
		free(tmp);
	}
	else
	{
		ANSWER2("ERROR");
	}
}

/* no param */
ATP2(PLAYSTOP)
{
	if (at->argc == 0)
	{
		SIRENA::preStop();
		ANSWER2("OK");
	}
	else
	{
		ANSWER2("ERROR");
	}
}

/* no param */
ATP2(MELODY)
{
	FS::tList* list = FS::list();
	if (list == NULL)
	{
		ANSWER2("NULL");
	}
	else
	{
		char* tmp = (char*)malloc(4 * FS::MELODY_MAX + 32);
		int l = 0;
		uint8 i;
		bool f;
		//l = sprintf(tmp, "[");
		for (i = 0, f = false; i < list->melody.count; i++)
		{
			if (f)
			{
				tmp[l] = ',';
				l++;
				tmp[l] = '\0';
			}
			else
			{
				f = true;
			}
			l += sprintf(&tmp[l], "%u", list->melody.index[i]);
		}
		//l += sprintf(&tmp[l], "]");
		ANSWER2(tmp);
		free(tmp);
		free(list);
	}
}

const DBG::tListAT DBG::listAT[] =
	{
		AT2("AT",			AT)
		AT2("PLAY",			PLAY)
		AT2("PLAYSTOP",		PLAYSTOP)
		AT2("MELODY",		MELODY)
		AT_NULL
	};
