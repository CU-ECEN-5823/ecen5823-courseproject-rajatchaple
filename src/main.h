/*********************************************************************************************
 *  @file main.h
 *	@brief includes, defines and function prototypes for main.c
 *		   configure energy mode using macro LOWEST_ENERGY_MODE
 *
 *  @author : Dave Sluiter
 *  @date Created on: Jan 26, 2021
 *
 *  Updated by Rajat Chaple Jan 29, 2020. Added GPIO Initialization and LED blinking.
 *  Updated by Rajat Chaple Feb 4, 2020. 	Removed delatApproxOneSecond() declaration
 *  										Added Energy Mode and LETIMER duty cycle configuration
 **********************************************************************************************/

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

#ifndef __myMAIN
#define __myMAIN


#include "gecko_configuration.h"
#include "native_gecko.h"
#include "em_cmu.h"
#include "timers.h"
#include "sleep.h"
#include "gpio.h"
#include "i2c.h"
#include "scheduler.h"
#include "ble.h"
#include "display.h"
#include "ble_device_type.h"


//defines for energy modes as Energy Mode is to be decided at pre-compile time
//values are taken from SLEEP_EnergyMode_t from sleep.h
//-----DO NOT Modify these values----
#define EM0 0
#define EM1 1
#define EM2 2
#define EM3 3
#define EM4 4
//-----------------------------------





//defines in which Energy mode a device is allowed to get into
//SELECT DESIRED ENERGY MODE
#define LOWEST_ENERGY_MODE EM1	//device is allowed to get into this mode...

//#define LETIMER_ON_TIME_MS  0//175 //
#define LETIMER_PERIOD_MS  (1000)//2250 //

//Oscillator frequency selection based on Energy Mode
#if (LOWEST_ENERGY_MODE >= EM0) && (LOWEST_ENERGY_MODE <= EM2)
#define OSCILLATOR_FREQ (32768)	//LFXO
#define PRESCALER_VALUE (4)
#elif (LOWEST_ENERGY_MODE == EM3)
#define OSCILLATOR_FREQ (1000)	//ULFRCO
#define PRESCALER_VALUE (1)
#endif

//Si7021 defines
#define WAIT_BOOT_TIME_SI7021 		  		(80000)	//wait for boot time of I2C module
#define TIME_FOR_TEMPERATURE_MEASUREMENT  	(11000)	//10.8 ms is maximum wait time for temperature to be ready


// function prototypes
int appMain(gecko_configuration_t *config);

#endif


#else

#ifndef __myMAIN
#define __myMAIN


#include "gecko_configuration.h"
#include "native_gecko.h"

#include "oscillators.h"
#include "timers.h"
#include "irq.h"
#include "sleep.h"
#include "log.h"
#include "scheduler.h"
#include "imu.h"
#include "ble.h"
#include "ble_device_type.h"

// MACROS definitions here
// BLE works in EM2
// I2C works in EM1
#define SLEEP_MODE_BLOCKED sleepEM3
#define ENABLE_SLEEPING 1
#define LETIMER_PERIOD_MS 5000// 5s
#define BOND_DISCONNECT 0

/*
#define SLEEP_MODE_BLOCKED sleepEM4
#define ENABLE_SLEEPING 1
#define LETIMER_PERIOD_MS 10000 // 10s
#define BOND_DISCONNECT 1
*/

// function prototypes
int appMain(gecko_configuration_t *config);



#endif


#endif
