/*
 * display.h
 *
 *  Created on: Jan 1, 2019
 *      Author: Dan Walkes
 *      Edited: Dave Sluiter, Dec 1 2020
 *				Rajat Chaple (GATT client code)
 *       		Sundar Krishnakumar (GATT server code)
 *
 * @date      April 29, 2020 (last update)
 */

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

#ifndef SRC_DISPLAY_H_
#define SRC_DISPLAY_H_

#define ECEN5823_INCLUDE_DISPLAY_SUPPORT 1
#define SCHEDULER_SUPPORTS_DISPLAY_UPDATE_EVENT 1
#define GPIO_DISPLAY_SUPPORT_IMPLEMENTED	1


#include "glib.h"

#include "native_gecko.h"

#include "graphics.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "glib.h"
#include "gpio.h"
//#include "log.h"
#include "hardware/kit/common/drivers/display.h"
#include "main.h"

// and for gpio
#include "gpio.h"

#include "ble_device_type.h"





/**
 * Display row definitions, used for writing specific content based on assignment requirements.
 * See assignment text for details.
 */
//enum display_row {
//	DISPLAY_ROW_NAME,
//	DISPLAY_ROW_BTADDR,
//	DISPLAY_ROW_BTADDR2,
//	DISPLAY_ROW_CLIENTADDR,
//	DISPLAY_ROW_CONNECTION,
//	DISPLAY_ROW_PASSKEY,
//	DISPLAY_ROW_ACTION,
//	DISPLAY_ROW_TEMPVALUE,
//	DISPLAY_ROW_MAX,
//};

enum display_row {
	DISPLAY_ROW_NAME,
	DISPLAY_ROW_BTADDR,
	DISPLAY_ROW_BTADDR2,
	DISPLAY_ROW_CLIENTADDR,
	DISPLAY_ROW_CONNECTION,
	DISPLAY_ROW_PASSKEY,
	DISPLAY_ROW_POSTURE,
	DISPLAY_ROW_INACTIVITY,
	DISPLAY_ROW_MAX,
};

// function prototypes
#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
void displayInit();
void displayUpdate();
void displayPrintf(enum display_row row, const char *format, ... );
#else
static inline void displayInit() { }
static inline void displayUpdate() { return true; }
static inline void displayPrintf(enum display_row row, const char *format, ... ) { row=row; format=format;}
#endif



#endif /* SRC_DISPLAY_H_ */

#else

#ifndef SRC_DISPLAY_H_
#define SRC_DISPLAY_H_

#define ECEN5823_INCLUDE_DISPLAY_SUPPORT 1
#define SCHEDULER_SUPPORTS_DISPLAY_UPDATE_EVENT 1
#define TIMER_SUPPORTS_1HZ_TIMER_EVENT	1

#define _1HZ_EVENT 1000 // 1000ms

// Copied from app_timer.h library as it was not discoverable in the project
// Timer Frequency used.
#define TIMER_CLK_FREQ ((uint32_t)32768)
// Convert msec to timer ticks.
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)

#include "glib.h"

#include "native_gecko.h"

#include "graphics.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "glib.h"
#include "gpio.h"
#include "log.h"
#include "hardware/kit/common/drivers/display.h"


// and for gpio
#include "gpio.h"

#include "ble_device_type.h"





/**
 * Display row definitions, used for writing specific content based on assignment requirements.
 * See assignment text for details.
 */
enum display_row {
	DISPLAY_ROW_NAME,
	DISPLAY_ROW_BTADDR,
	DISPLAY_ROW_BTADDR2,
	DISPLAY_ROW_CLIENTADDR,
	DISPLAY_ROW_CONNECTION,
	DISPLAY_ROW_PASSKEY,
	DISPLAY_ROW_TUT,
	DISPLAY_ROW_TEMPVALUE,
	DISPLAY_ROW_MAX,
};


// function prototypes
#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
void displayInit();
void displayUpdate();
void displayPrintf(enum display_row row, const char *format, ... );
#else
static inline void displayInit() { }
static inline void displayUpdate() { return true; }
static inline void displayPrintf(enum display_row row, const char *format, ... ) { row=row; format=format;}
#endif



#endif /* SRC_DISPLAY_H_ */
#endif
