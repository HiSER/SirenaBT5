/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 7 мар. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <ch.hpp>

#if CH_IRQ_THREAD != 0
extern "C" void TIM15_IRQHandler(void) __attribute__((__alias__("_ZN2CH12__irq_threadEv")));
#endif

PIN* CH::ch[CHANNELS];

void CH::Init(void)
{
	DBGP_NOTICE("CH: init")

	ch[0] = new PIN(1, GPIO_Pin_9, false, true);
	ch[1] = new PIN(2, GPIO_Pin_10, true);
	ch[2] = new PIN(3, GPIO_Pin_11, true);

	if (CHANNELS > 3)
	{
		DBGP_FATAL("CH: fatal count channels");
		LOOP {}
	}
	else if (CHANNELS < 3)
	{
		DBGP_WARNING("CH: error count channels");
	}
	if (ch[0] == NULL || ch[1] == NULL || ch[2] == NULL)
	{
		DBGP_FATAL("CH: fatal allocate memory for channels");
		LOOP {}
	}

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef pin;
	pin.GPIO_Pin	= GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	pin.GPIO_Mode	= GPIO_Mode_IN;
	pin.GPIO_Speed	= GPIO_Speed_Level_1;
	pin.GPIO_OType	= GPIO_OType_PP;
	pin.GPIO_PuPd	= GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &pin);

#if CH_IRQ_THREAD == 0
	CALL::setImmediate(thread);
#else
	TIM_TimeBaseInitTypeDef tim;
	TIM_TimeBaseStructInit(&tim);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);
	NVIC_EnableIRQ(TIM15_IRQn);
	NVIC_SetPriority(TIM15_IRQn, CH_PRIORITY);
	tim.TIM_Period = SystemCoreClock / CH_IRQ_PER_SECOND - 1;
	TIM_TimeBaseInit(TIM15, &tim);
	TIM_SetCounter(TIM15, 0);
#endif

	DBGP_NOTICE("CH: ok");
}

void CH::Start(void)
{
	__DMB();
#if CH_IRQ_THREAD != 0
	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM15, ENABLE);
#endif
}

void CH::DeInit(void)
{
#if CH_IRQ_THREAD != 0
	TIM_Cmd(TIM15, DISABLE);
	TIM_ITConfig(TIM15, TIM_IT_Update, DISABLE);
	NVIC_SetPriority(TIM15_IRQn, 0);
	NVIC_DisableIRQ(TIM15_IRQn);
#endif
}

void CH::__irq_thread(void)
{
	if (TIM_GetITStatus(TIM15, TIM_IT_Update) != RESET)
	{
		thread(NULL);
		TIM_ClearITPendingBit(TIM15, TIM_IT_Update);
	}
}

void CH::thread(__UNUSED void* param)
{
	uint8 i;
	for (i = 0; i < CHANNELS; i++)
	{
		ch[i]->thread();
	}
#if CH_IRQ_THREAD == 0
	CALL::setImmediate(thread);
#endif
}

bool CH::active(void)
{
	bool result;
#if CH_IRQ_THREAD != 0
	TIM_ITConfig(TIM15, TIM_IT_Update, DISABLE);
#endif
	result = ch[0]->_get() || ch[1]->_get() || ch[2]->_get();
#if CH_IRQ_THREAD != 0
	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);
#endif
	return result;
}

bool CH::event(tEvent* event)
{
	uint8 i;
	bool result = false;
#if CH_IRQ_THREAD != 0
	TIM_ITConfig(TIM15, TIM_IT_Update, DISABLE);
#endif
	for (i = 0; i < CHANNELS; i++)
	{
		if (ch[i]->active())
		{
			event->index = ch[i]->index();
			memcpy(event->pulseType, ch[i]->pulseType, sizeof(PIN::tPulses) * PIN::PULSES_TYPE_MAXIMUM);
			event->lengthType = ch[i]->lengthType;
			event->playOnce = ch[i]->cfg.playOnce;
			event->playToEnd = ch[i]->cfg.playToEnd;
			ch[i]->reset();
			result = true;
			break;
		}
	}
#if CH_IRQ_THREAD != 0
	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);
#endif
	return result;
}

void CH::set(uint8 index, const PIN::tCfg* config)
{
#if CH_IRQ_THREAD != 0
	TIM_ITConfig(TIM15, TIM_IT_Update, DISABLE);
#endif
	if (index < CHANNELS && config != NULL)
	{
		memcpy(&ch[index]->cfg, config, sizeof(PIN::tCfg));
	}
#if CH_IRQ_THREAD != 0
	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);
#endif
}

PIN::tCfg* CH::get(uint8 index)
{
	PIN::tCfg* config = (PIN::tCfg*)malloc(sizeof(PIN::tCfg));
#if CH_IRQ_THREAD != 0
	TIM_ITConfig(TIM15, TIM_IT_Update, DISABLE);
#endif
	if (index < CHANNELS)
	{
		memcpy(config, &ch[index]->cfg, sizeof(PIN::tCfg));
	}
	else
	{
		memset(config, 0, sizeof(PIN::tCfg));
	}
#if CH_IRQ_THREAD != 0
	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);
#endif
	return config;
}

uint8 CH::deviation(uint8 index)
{
	uint8 d;
#if CH_IRQ_THREAD != 0
	TIM_ITConfig(TIM15, TIM_IT_Update, DISABLE);
#endif
	d = ((index < CHANNELS) ? ch[index]->cfg.deviation : PIN::PULSES_DEVIATION_DEF);
#if CH_IRQ_THREAD != 0
	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);
#endif
	return d;
}

bool CH::standard(uint8 index)
{
    uint8 r;
#if CH_IRQ_THREAD != 0
    TIM_ITConfig(TIM15, TIM_IT_Update, DISABLE);
#endif
    r = ((index < CHANNELS) ? ch[index]->cfg.standard : false);
#if CH_IRQ_THREAD != 0
    TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);
#endif
    return r;
}

PIN::PIN(uint8 index, uint16 pin, bool button, bool standard)
{
	_index = index;
	_pin = pin;
	memset(&cfg, 0, sizeof(tCfg));
	cfg.pressMax	= PRESS_DEF;
	cfg.idleMax		= IDLE_DEF;
	cfg.pulsesMax	= PULSES_DEF;
	cfg.deviation	= PULSES_DEVIATION_DEF;
	cfg.button		= button;
	/*cfg.measure		= false;
	cfg.playOnce	= false;
	cfg.playToEnd	= false;
	cfg.noWidth		= false;*/
	cfg.noBounceHigh	= true;
	cfg.standard    = standard;
	bounceTime		= new TIMER();
	pressTime		= new TIMER();
	idleTime		= new TIMER();
	pulseWidth		= new TIMER();
	stateBounce		= false;
	state			= false;
	pressTrigged	= false;
	cyclicPulses	= false;
	idleTrigged		= true;
	firstTime		= FIRST_TIME_COMPENSATION;
	pulses			= 0;
	reset();
}

bool PIN::_get(void)
{
	return ((GPIOA->IDR & _pin) != 0);
}


bool PIN::active(void)
{
	return _active;
}

uint8 PIN::index(void)
{
	return _index;
}

void PIN::reset(void)
{
	lengthType = eptNull;
	multiPulses = false;
	_active = false;
}

void PIN::set(ePulseType type)
{
	if (_active)
	{
		DBGF_WARNING("CH%u event collision", _index);
	}
	else
	{
		_active = true;
		if (type != eptNull) lengthType = type;
	}
}

void PIN::thread(void)
{
	if (stateBounce ^ _get())
	{
		stateBounce = !stateBounce;
		if (cfg.button)
		{
			bounceTime->set((stateBounce) ? ANTI_BOUNCE_BUTTON_HIGH : ANTI_BOUNCE_BUTTON_LOW);
		}
		else
		{
			bounceTime->set((stateBounce) ? ((cfg.noBounceHigh) ? 0 : ANTI_BOUNCE_HIGH) : ANTI_BOUNCE_LOW);
		}
	}
	if ((state ^ stateBounce) && bounceTime->finish())
	{
		state = !state;
		//DBGF("CH pin %u state %s", _index, STRBOOL(state));
		if (cfg.measure)
		{
			sint16 width;
			if (state)
			{
				if (!idleTrigged)
				{
					width = idleTime->get();
					if (multiPulses)
					{
						if (width > pulseType[0].count) pulseType[0].count = width;
					}
				}
				idleTrigged = false;
				pulseWidth->set();
				idleTime->set();
				pressTime->set(cfg.idleMax);
			}
			else
			{
				if (!idleTrigged)
				{
					width = pulseWidth->get();
					if (!multiPulses)
					{
						multiPulses = true;
						pulseType[0].width = width;
						pulseType[1].width = width;
						pulseType[0].count = width;
					}
					else
					{
						if
						(
							   pulseType[1].width < (width - cfg.deviation)
							|| pulseType[1].width > (width + cfg.deviation)
						)
						{
							pulseType[1].width = width;
							if (width > pulseType[0].count) pulseType[0].count = width;
						}
					}
				}
			}
		}
		else
		{
			if (cfg.button)
			{
				set((state) ? eptPress : eptRelease);
			}
			else
			{
				if (state)
				{
					if (idleTrigged) pulses = 0;
					if (!cyclicPulses)
					{
						if (pulses < cfg.pulsesMax)
						{
							pulses++;
						}
						else
						{
							cyclicPulses = true;
							set(eptCyclic);
						}
					}
					//pressTrigged = (pulses > 1) | multiPulses | cyclicPulses;
					pressTrigged = false;
					idleTrigged = false;
					//pressTime->set(cfg.pressMax);
					//DBGF("CH%u pressMax %u", _index, cfg.pressMax);
					pressTime->set(((pulses > 1) | multiPulses | cyclicPulses) ? PIN::TIME_MAXIMUM : cfg.pressMax);
					idleTime->set(cfg.idleMax);
					pulseWidth->set();
				}
				else
				{
					if (pressTrigged && idleTrigged)
					{
						//DBGF("CH%u release", _index);
						set(eptRelease);
					}
					else if (!_active && lengthType < PULSES_TYPE_MAXIMUM)
					{
						sint16 width = ((cfg.noWidth) ? 100 : pulseWidth->get() + firstTime - ANTI_BOUNCE_LOW + ((cfg.noBounceHigh) ? 0 : ANTI_BOUNCE_HIGH));
						firstTime = 0;
						if (pulses == 1)
						{
							pulseType[lengthType].width = width;
							pulseType[lengthType].count = 1;
						}
						else
						{
							if
							(
								   pulseType[lengthType].width >= (width - cfg.deviation)
								&& pulseType[lengthType].width <= (width + cfg.deviation)
							)
							{
								pulseType[lengthType].width = ((uint32)pulseType[lengthType].width + width) / 2;
								pulseType[lengthType].count = pulses;
							}
							else
							{
								lengthType = (ePulseType)(lengthType + 1);
								if (lengthType < PULSES_TYPE_MAXIMUM)
								{
									pulseType[lengthType].width = width;
									pulseType[lengthType].count = 1;
								}
								multiPulses = true;
								pulses = 1;
							}
						}
					}
				}
			}
		}
	}
	if (cfg.measure)
	{
		if (!idleTrigged && pressTime->finish())
		{
			idleTrigged = true;
			if (!state)
			{
				set(eptMeasure);
			}
			else
			{
				//DBGF("CH%u press no measure", _index);
				set(eptErrMeasure);
			}
			multiPulses = false;
		}
	}
	else if (!cfg.button)
	{
		if (state)
		{
			if (!pressTrigged && pressTime->finish())
			{
				pressTrigged = true;
				idleTrigged = true;
				firstTime = 0;
				//DBGF("CH%u press", _index);
				set(eptPress);
			}
		}
		else
		{
			if (!idleTrigged && idleTime->finish())
			{
				pressTrigged = false;
				idleTrigged = true;
				//DBGF("CH%u idle pulses %u", _index, pulses);
				if (cyclicPulses)
				{
					set(eptRelease);
				}
				else
				{
					set((lengthType < PULSES_TYPE_MAXIMUM) ? (ePulseType)(lengthType + 1) : eptNull);
				}
				cyclicPulses = false;
			}
		}
	}
	/*if (_active)
	{
		uint8 i;
		switch (lengthType)
		{
		case eptErrMeasure:
			DBGF("CH%u: press no measure", _index);
			break;
		case eptMeasure:
			DBGF("CH%u: measure %u/%u", _index, pulseType[0].width, pulseType[0].count);
			break;
		case eptCyclic:
			DBGF("CH%u: cyclic", _index);
			break;
		case eptRelease:
			DBGF("CH%u: release", _index);
			break;
		case eptPress:
			DBGF("CH%u: press", _index);
			break;
		default:
			if (lengthType == eptNull)
			{
				DBGF("CH%u: error pulses", _index);
			}
			else
			{
				for (i = 0; i < lengthType; i++)
				{
					DBGF("CH%u: pulses %u - %u/%u", _index, i, pulseType[i].width, pulseType[i].count);
				}
			}
			break;
		}
		reset();
	}*/
}
