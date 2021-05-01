/*********************************************************************************************
 *  @file scheduler.h
 *	@brief This file contains defines, includes and function prototypes for scheduler.c
 *
 *  @authors : Rajat Chaple (GATT client code)
 *  		   Sundar Krishnakumar (GATT server code)
 *
 *  @date     April 29, 2020 (last update)
 **********************************************************************************************/

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

#ifndef SCHEDULER_H
#define SCHEDULER_H

//includes and defines
#include "em_core.h"
#include "gpio.h"
#include "timers.h"
#include "i2c.h"
#include "i2cspm.h"
#include "infrastructure.h"
#include "ble.h"
#include "display.h"

#define THRESHOLD_TIME_ACTIVE_TO_INACTIVE_S	(90)	//Seconds
#define INITIAL_TIME_UNTIL_TRIGGER_FOR_BAD_POSTURE_S	(30)	//Seconds


//Declarations
enum events{
	LETIMER_UF_INTERRUPT_EVENT = 0x00000001,
	LETIMER_COMP1_INTERRUPT_EVENT = 0x00000002,
	I2C_TRANSFER_COMPLETE = 0x00000004,
	I2C_TRANSFER_RETRY = 0x00000008,
	PB0_SWITCH_HIGH_TO_LOW = 0x00000010,
	PB0_SWITCH_LOW_TO_HIGH = 0x00000020,
	PB1_SWITCH_LOW_TO_HIGH = 0x00000040,
	PROXIMITY_DETECTED = 0x00000080,
	LAST_EVENT_IN_THE_LIST = PROXIMITY_DETECTED
};


//function prototypes
void scheduler_set_event_proximity_detected(void);
void scheduler_set_event_PB0_switch_high_to_low(void);
void scheduler_set_event_PB0_switch_low_to_high(void);
void scheduler_set_event_PB1_switch_low_to_high(void);
void scheduler_set_event_UF(void);
void scheduler_set_event_COMP1(void);
void scheduler_set_event_I2C_transfer_complete(void);
void scheduler_set_event_I2C_transfer_retry(void);
uint32_t get_event(void);
bool event_present(void);
void event_handler_proximity_state(struct gecko_cmd_packet* evt);
void state_machine_proximity_state(struct gecko_cmd_packet* evt);


#endif /* SCHEDULER_H */

#else

#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "stdlib.h"
#include "stdint.h"
#include "imu.h"
#include "em_core.h"
#include "sleep.h"
#include "native_gecko.h"
#include "ble_device_type.h"
#include "ble.h"
#include "main.h"

// Forward declarations
extern SLEEP_EnergyMode_t sleep_mode_blocked;

typedef uint8_t runqueue;

extern uint8_t current_state;

typedef enum
{
	STATE_ON_IMU,
	STATE_ACC_STANDBY_SIGNAL_SEND,
	STATE_ACC_STANDBY_SIGNAL_STOP,
	STATE_ACC_DATA_CFG_START,
	STATE_ACC_DATA_CFG_STOP,
	STATE_ACC_CTRL_REG1_START,
	STATE_ACC_CTRL_REG1_STOP,
	STATE_GYRO_STANDBY_SIGNAL_SEND,
	STATE_GYRO_STANDBY_SIGNAL_STOP,
	STATE_GYRO_RESET_SIGNAL_SEND,
	STATE_GYRO_RESET_SIGNAL_NACK, // reset signal gives a NACK.
	STATE_GYRO_RESET_SIGNAL_STOP,
	STATE_GYRO_CTRL_REG0_START,
	STATE_GYRO_CTRL_REG0_STOP,
	STATE_GYRO_CTRL_REG1_START,
	STATE_GYRO_CTRL_REG1_STOP,
	STATE_WAIT_FOR_65_MILLIS,
	STATE_ACC_MEASURE_START,
	STATE_ACC_MEASURE_STOP,
	STATE_GYRO_MEASURE_START,
	STATE_GYRO_MEASURE_STOP,
	STATE_SEND_IMU_INDICATION,
	TRANSFER_STATE_SIZE,

} transfer_states_t;

uint8_t events_present(void);

runqueue get_event(void);
void process_event(runqueue func);
void do_nothing(void);

void scheduler_set_event_UF(void);
void scheduler_set_event_STATE_ACC_STANDBY_SIGNAL_SEND(void);
void scheduler_set_event_STATE_ACC_STANDBY_SIGNAL_STOP(void);
void scheduler_set_event_STATE_ACC_DATA_CFG_STOP(void);
void scheduler_set_event_STATE_ACC_CTRL_REG1_STOP(void);
void scheduler_set_event_STATE_GYRO_STANDBY_SIGNAL_STOP(void);
void scheduler_set_event_STATE_GYRO_RESET_SIGNAL_NACK(void);
void scheduler_set_event_STATE_GYRO_CTRL_REG0_STOP(void);
void scheduler_set_event_STATE_WAIT_FOR_60_MILLIS(void);
void scheduler_set_event_STATE_GYRO_CTRL_REG1_STOP(void);
void scheduler_set_event_STATE_WAIT_FOR_65_MILLIS(void);
void scheduler_set_event_STATE_ACC_MEASURE_STOP(void);
void scheduler_set_event_STATE_GYRO_MEASURE_STOP(void);
void scheduler_set_event_STATE_SEND_IMU_INDICATION(void);
void state_machine(runqueue func);
void scheduler_set_event_common(uint32_t f_ptr);



#endif /* __SCHEDULER_H__*/
#endif
