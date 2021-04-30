/*********************************************************************************************
 *  @file oscillators.h
 *	@brief includes, defines and function prototypes for oscillators.c
 *		   clock frequency calculation based on Energy Mode requirement
 *
 *  @authors : Rajat Chaple (GATT client code)
 *  		   Sundar Krishnakumar (GATT server code)
 *
 *  @date     April 29, 2020 (last update)
 **********************************************************************************************/

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

#ifndef SRC_OSCILLATORS_H_
#define SRC_OSCILLATORS_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>
#include "em_cmu.h"

#define ACTUAL_CLK_FREQ (OSCILLATOR_FREQ/PRESCALER_VALUE)

//function prototypes
void configure_clock(void);

#endif /* SRC_OSCILLATORS_H_ */

#else

#ifndef __OSCILLATORS_H__
#define __OSCILLATORS_H__


#include "em_cmu.h"

void init_LFXO_LETIMER0(CMU_ClkDiv_TypeDef div);
void init_ULFRCO_LETIMER0(CMU_ClkDiv_TypeDef div);

#endif /* __OSCILLATORS_H__ */

#endif
