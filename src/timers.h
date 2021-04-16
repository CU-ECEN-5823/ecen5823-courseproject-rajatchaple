/*********************************************************************************************
 *  @file timers.h
 *	@brief includes, defines and function prototypes for timers.c
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 4, 2021
 **********************************************************************************************/
#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

#ifndef SRC_TIMERS_H_
#define SRC_TIMERS_H_

#include "main.h"
#include "em_letimer.h"
#include "oscillators.h"

//Calculating Counter and COMP1 value a precompile time
#define VALUE_TO_LOAD_COUNTER ((LETIMER_PERIOD_MS*ACTUAL_CLK_FREQ)/1000)
//#define VALUE_TO_LOAD_COMP1 (((LETIMER_PERIOD_MS-LETIMER_ON_TIME_MS)*ACTUAL_CLK_FREQ)/1000)
#define VALUE_TO_LOAD_COMP1 0
#define MILLISECONDS_IN_SECOND (1000)
#define MAX_WAIT_TIME_ALLOWED_MS ((65536*MILLISECONDS_IN_SECOND)/ACTUAL_CLK_FREQ)	//pow(2,16) = 65536

//function prototypes
void init_LETIMER0();
void timerWaitUs(uint32_t);

#endif /* SRC_TIMERS_H_ */

#else
#endif
