/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 24 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <amp.hpp>

const uint8 AMP::volumeTable[] = {5, 10, 15, 20, 30, 50, 75, 100, 125, 150};

//extern "C" void TIM15_IRQHandler(void) __attribute__((__alias__("_ZN3AMP10__irq_mainEv")));
extern "C" void TIM16_IRQHandler(void) __attribute__((__alias__("_ZN3AMP9__irq_mldEv")));
extern "C" void TIM17_IRQHandler(void) __attribute__((__alias__("_ZN3AMP11__irq_speakEv")));

FIFO8* AMP::mainBuffer;
FIFO8* AMP::speakBuffer;

sint16 AMP::mainSample;
sint16 AMP::speakSample;
uint8 AMP::mainVolume;
uint8 AMP::_mainVolume;
bool AMP::speakIsWait;
uint16 AMP::speakWait;
bool AMP::outEnable;
bool AMP::outDisable;
uint8 AMP::disableCounter;
uint16 AMP::volumeWait;
uint8 AMP::cfgVolume;
uint8 AMP::cfgVolumeBeep;


/*void AMP::__irq_main(void)
{
	if (TIM_GetITStatus(TIM15, TIM_IT_Update) != RESET)
	{
		sint16 smp16 = mainSample / mainVolume + speakSample;
		uint8 smp8;
		if (smp16 < -128) smp16 = -128;
		if (smp16 > 127) smp16 = 127;
		smp8 = (uint8)smp16;
		smp8 += 0x80;
		TIM_SetCompare1(TIM1, smp8);
		TIM_ClearITPendingBit(TIM15, TIM_IT_Update);
	}
}*/

void AMP::__irq_mld(void)
{
	if (TIM_GetITStatus(TIM16, TIM_IT_Update) != RESET)
	{
		if (mainBuffer->empty())
		{
			if (mainSample > 0) mainSample--;
			if (mainSample < 0) mainSample++;
#if AMP_OFF == 1
			if (speakBuffer->empty())
			{
				if (outEnable)
				{
					outDisable = true;
				}
				else if (outDisable)
				{
					outDisable = false;
					GPIOA->MODER &= ~GPIO_MODER_MODER8;
					//DBGP("AMP: off");
				}
			}
#endif
		}
		else
		{
			uint8 smp = mainBuffer->pop();
			smp -= 0x80;
			mainSample = (sint8)smp;
		}

		if (volumeWait == 0)
		{
			_mainVolume = mainVolume;
		}
		else
		{
			volumeWait--;
		}

		sint16 smp16 = mainSample * _mainVolume / 100 + speakSample;
		uint8 smp8;
		if (smp16 < -128) smp16 = -128;
		if (smp16 > 127) smp16 = 127;
		smp8 = (uint8)smp16;
		smp8 += 0x80;
		TIM_SetCompare1(TIM1, smp8);

		TIM_ClearITPendingBit(TIM16, TIM_IT_Update);
	}
}

void AMP::__irq_speak(void)
{
	if (TIM_GetITStatus(TIM17, TIM_IT_Update) != RESET)
	{
		if (speakBuffer->empty())
		{
			if (speakSample > 0) speakSample--;
			if (speakSample < 0) speakSample++;
			if (speakIsWait)
			{
				//mainVolume = MAIN_VOLUME;
				speakWait = 0;
			}
			else
			{
				speakWait++;
				if (speakWait >= SPEAK_WAIT_END) speakIsWait = true;
			}
		}
		else
		{
			if (!speakIsWait)
			{
				uint8 smp = speakBuffer->pop();
				smp -= 0x80;
				speakSample = (sint8)smp;
				//mainVolume = MAIN_VOLUME_WITH_SPEAK;
				speakWait = 0;
			}
			else
			{
				speakWait++;
				if (speakWait >= SPEAK_WAIT_START) speakIsWait = false;
			}
		}
		TIM_ClearITPendingBit(TIM17, TIM_IT_Update);
	}
}

#if AMP_OFF == 1
void AMP::outOn()
{
	if (!outEnable)
	{
		outEnable = true;
		uint16 l = 128 * 8;
		//if (l > MAIN_BUFFER_SIZE) l = MAIN_BUFFER_SIZE;
		uint8* buff = (uint8*)malloc(l);
		uint16 i;
		for (i = 0; i < l; i++)
		{
			buff[i] = i >> 3;
		}
		mainBuffer->write(buff, l);
		free(buff);
		TIM_SetCompare1(TIM1, 0);
		GPIOA->MODER |= GPIO_MODER_MODER8_1;
		volumeWait = l;
		_mainVolume = 100;
		//DBGP("AMP: on");
	}
	outDisable = false;
}

void AMP::outOffThread(void* param __UNUSED)
{
	if (outDisable)
	{
		if (disableCounter > 5)
		{
			TIM_ITConfig(TIM16, TIM_IT_Update, DISABLE);
			if (outEnable)
			{
				outEnable = false;
				uint16 l = TIM1->CCR1 * 8;
				if (l > MAIN_BUFFER_SIZE) l = MAIN_BUFFER_SIZE;
				uint8* buff = (uint8*)malloc(l);
				uint16 i;
				for (i = 0; i < l; i++)
				{
					buff[i] = (l - i) >> 3;
				}
				mainBuffer->write(buff, l);
				free(buff);
				volumeWait = l;
				_mainVolume = 100;
				//DBGF("AMP: routine %u", l);
			}
			TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);
		}
		else
		{
			disableCounter++;
		}
	}
	else
	{
		disableCounter = 0;
	}
}
#endif

void AMP::Init(void)
{
	DBGP_NOTICE("AMP: init");

	mainBuffer = new FIFO8(MAIN_BUFFER_SIZE);
	speakBuffer = new FIFO8(SPEAK_BUFFER_SIZE);

	mainSample = 0;
	speakSample = 0;
	speakIsWait = true;
	speakWait = 0;
	outEnable = false;
	outDisable = false;
	disableCounter = 0;
	_mainVolume = 0;
	volumeWait = 0;
	//mainVolume = MAIN_VOLUME;
	cfgVolume = MAIN_VOLUME_STD;
	cfgVolumeBeep = MAIN_VOLUME_MAX;
	volume();

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);

	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);

	GPIO_InitTypeDef pinPWM;
	pinPWM.GPIO_Pin		= GPIO_Pin_8;
#if AMP_OFF == 1
	pinPWM.GPIO_Mode	= GPIO_Mode_IN;
#else
	pinPWM.GPIO_Mode    = GPIO_Mode_AF;
#endif
	pinPWM.GPIO_Speed	= GPIO_Speed_Level_3;
	pinPWM.GPIO_OType	= GPIO_OType_PP;
	pinPWM.GPIO_PuPd	= GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &pinPWM);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_2);

	TIM_TimeBaseInitTypeDef timPWM;
	TIM_TimeBaseStructInit(&timPWM);
	timPWM.TIM_Period			= 0xFF;
	TIM_TimeBaseInit(TIM1, &timPWM);

	TIM_OCInitTypeDef timPWMOC;
	TIM_OCStructInit(&timPWMOC);
	timPWMOC.TIM_OCMode			= TIM_OCMode_PWM1;
	timPWMOC.TIM_OutputState	= TIM_OutputState_Enable;
	timPWMOC.TIM_Pulse			= 0x80;
	TIM_OC1Init(TIM1, &timPWMOC);

	TIM_BDTRInitTypeDef timDT;
	TIM_BDTRStructInit(&timDT);
	TIM_BDTRConfig(TIM1, &timDT);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	TIM_Cmd(TIM1, ENABLE);

	//NVIC_EnableIRQ(TIM15_IRQn);
	NVIC_EnableIRQ(TIM16_IRQn);
	NVIC_EnableIRQ(TIM17_IRQn);
	//NVIC_SetPriority(TIM15_IRQn, AMP_PRIORITY);
	NVIC_SetPriority(TIM16_IRQn, AMP_PRIORITY);
	NVIC_SetPriority(TIM17_IRQn, AMP_PRIORITY);

	TIM_TimeBaseInitTypeDef timSMP;
	TIM_TimeBaseStructInit(&timSMP);

	/* output */
	//timSMP.TIM_Period = SystemCoreClock / SAMPLES_MAXIMUM - 1;
	//TIM_TimeBaseInit(TIM15, &timSMP);
	//TIM_SetCounter(TIM15, 0);

	/* speakBuffer */
	timSMP.TIM_Period = SystemCoreClock / SAMPLES_SPEAK - 1;
	TIM_TimeBaseInit(TIM17, &timSMP);
	TIM_SetCounter(TIM17, 0);

	/* mainBuffer */
	set();

	//TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);
	TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);
	TIM_ITConfig(TIM17, TIM_IT_Update, ENABLE);
	//TIM_Cmd(TIM15, ENABLE);
	TIM_Cmd(TIM16, ENABLE);
	TIM_Cmd(TIM17, ENABLE);

#if AMP_OFF == 1
	CALL::setInterval(outOffThread, 100);
#endif

	DBGP_NOTICE("AMP: ok");
}

void AMP::DeInit(void)
{
	//TIM_Cmd(TIM15, DISABLE);
	TIM_Cmd(TIM16, DISABLE);
	TIM_Cmd(TIM17, DISABLE);
	TIM_Cmd(TIM1, DISABLE);
	//NVIC_SetPriority(TIM15_IRQn, 0);
	NVIC_SetPriority(TIM16_IRQn, 0);
	NVIC_SetPriority(TIM17_IRQn, 0);
	//NVIC_DisableIRQ(TIM15_IRQn);
	NVIC_DisableIRQ(TIM16_IRQn);
	NVIC_DisableIRQ(TIM17_IRQn);

	GPIO_InitTypeDef pinPWM;
	GPIO_StructInit(&pinPWM);
	pinPWM.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &pinPWM);
}

void AMP::volume(uint8 value)
{
	if (value < MAIN_VOLUME_MIN) value = MAIN_VOLUME_MIN;
	if (value > MAIN_VOLUME_MAX) value = MAIN_VOLUME_MAX;
	mainVolume = volumeTable[value - MAIN_VOLUME_MIN];
}

void AMP::set(uint16 samples)
{
	TIM_TimeBaseInitTypeDef timSMP;
	TIM_TimeBaseStructInit(&timSMP);
	timSMP.TIM_Period = SystemCoreClock / samples - 1;
	TIM_TimeBaseInit(TIM16, &timSMP);
	TIM_SetCounter(TIM16, 0);
}

int AMP::write(const uint8* data, int length)
{
	int len;
	TIM_ITConfig(TIM16, TIM_IT_Update, DISABLE);
#if AMP_OFF == 1
	outOn();
#endif
	len = mainBuffer->write(data, length);
	TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);
	return len;
}

int AMP::speak(const uint8* data, int length)
{
	int len;
	TIM_ITConfig(TIM16, TIM_IT_Update, DISABLE);
#if AMP_OFF == 1
	outOn();
#endif
	TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);
	TIM_ITConfig(TIM17, TIM_IT_Update, DISABLE);
	len = speakBuffer->write(data, length);
	TIM_ITConfig(TIM17, TIM_IT_Update, ENABLE);
	return len;
}
