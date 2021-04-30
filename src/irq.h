/*********************************************************************************************
 *  @file irq.h
 *	@brief includes, defines and function prototypes for irq.c
 *
 *  @authors : Rajat Chaple (GATT client code)
 *  		   Sundar Krishnakumar (GATT server code)
 *
 *  @date      April 29, 2020 (last update)
 **********************************************************************************************/

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT
#ifndef SRC_IRQ_H_
#define SRC_IRQ_H_


#include "em_letimer.h"
#include "gpio.h"
#include "em_gpio.h"
#include "sleep.h"
#include "em_core.h"
#include "scheduler.h"
#include "timers.h"

uint32_t letimerMilliseconds(void);
uint32_t getSysTicks(void);

#endif /* SRC_IRQ_H_ */

#else


#ifndef __IRQ_H__
#define __IRQ_H__


#include "em_letimer.h"
#include "scheduler.h"
#include "log.h" // Only used to increment millis_count. No calls to LOG() inside ISR.
#include "ble.h"
#include "gpio.h"



#endif /* __IRQ_H__ */


#endif
