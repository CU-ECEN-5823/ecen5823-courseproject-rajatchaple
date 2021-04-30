/*********************************************************************************************
 *  @file timers.h
 *	@brief includes, defines and function prototypes for timers.c
 *
 *  @authors : Rajat Chaple (GATT client code)
 *  		   Sundar Krishnakumar (GATT server code)
 *
 *  @date      April 29, 2020 (last update)
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

#ifndef __TIMERS_H__
#define __TIMERS_H__


#include "gecko_configuration.h"
#include "native_gecko.h"
#include "em_letimer.h"
#include "em_cmu.h"
#include "stdlib.h"
#include "sleep.h"


// Forward declarations
void value_to_load(CMU_Osc_TypeDef osc, CMU_ClkDiv_TypeDef div, uint32_t period, uint16_t *value);
void config_LETIMER0(LETIMER_Init_TypeDef *letimer_init, uint16_t cmp0_value, uint16_t cmp1_value);
void config_INT_LETIMER0(uint32_t interrupt_flags);
void timerWaitUs(uint32_t us_wait);


#endif /* __TIMERS_H__ */
#endif
