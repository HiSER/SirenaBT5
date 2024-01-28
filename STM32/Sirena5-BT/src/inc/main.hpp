/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 22 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef MAIN_HPP_
#define MAIN_HPP_ 1

#include <stm32f0xx.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_usart.h>
#include <stm32f0xx_flash.h>
#include <stm32f0xx_iwdg.h>
#include <stm32f0xx_tim.h>
#include <stm32f0xx_exti.h>
#include <stm32f0xx_spi.h>
#include <stm32f0xx_dma.h>
#include <macro.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#define NAME				"SirenaBT"
#define VERSION				5.0
#define SPEAK_ENCODE		WAVE7
#define IWDG_TIME_SECOND	15
#define BT_PIN_DEAFULT		"0000"

#define BUILD NAME "-" ADDQUOTES(VERSION) ", " ADDQUOTES(__BUILD_FULL__)

extern "C" size_t getFreeHeap(void);
extern "C" size_t getMinimumEverFreeHeap(void);

#define TIMER_PRIORITY		0
#define AMP_PRIORITY		1
#define CH_PRIORITY			1
#define SD_PRIORITY			2
#define DBG_PRIORITY		1
#define BT_PRIORITY			3
#define UART_PRIORITY		3

/*
 * POWER
 *  ON			- !PB9
 *
 * DBG
 *  RX			- PB10
 *
 * BT
 *  TX			- PB7
 *  RX			- PB6
 *  RESET		- !PA7
 *  INT			- PB8
 *
 * SD
 *  POWER		- !PB0
 *  CS			- !PB1
 *  CLK			- PB3
 *  DI			- PB5
 *  DO			- PB4
 *
 * AMP
 *  IN			- PA8
 *
 * CANNELS
 *  CH1			- PA9
 *  CH2			- PA10
 *  CH3			- PA11
 */

#endif
