/*********************************************************************************************
 *  @file scheduler.h
 *	@brief This file contains defines, includes and function prototypes for scheduler.c
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 11, 2021
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



//Declarations
typedef enum temp_sensor_states{
	STATE0_TIMER_WAIT,
	STATE1_WAIT_FOR_POWER_UP,
	STATE2_WAIT_FOT_I2C_WRITE_COMPLETE,
	STATE3_WAIT_FOR_I2C_READ_START,
	STATE4_WAIT_FOR_I2C_READ_COMPLETE,
	MY_NUM_STATES
}temp_sensor_states_t;

enum events{
	LETIMER_UF_INTERRUPT_EVENT = 0x00000001,
	LETIMER_COMP1_INTERRUPT_EVENT = 0x00000002,
	I2C_TRANSFER_COMPLETE = 0x00000004,
	I2C_TRANSFER_RETRY = 0x00000008,
	PB0_SWITCH_HIGH_TO_LOW = 0x00000010,
	PB0_SWITCH_LOW_TO_HIGH = 0x00000020,
	PB1_SWITCH_LOW_TO_HIGH = 0x00000040,
	LAST_EVENT_IN_THE_LIST = PB1_SWITCH_LOW_TO_HIGH
};


//function prototypes
void scheduler_set_event_PB0_switch_high_to_low(void);
void scheduler_set_event_PB0_switch_low_to_high(void);
void scheduler_set_event_PB1_switch_low_to_high(void);
void scheduler_set_event_UF(void);
void scheduler_set_event_COMP1(void);
void scheduler_set_event_I2C_transfer_complete(void);
void scheduler_set_event_I2C_transfer_retry(void);
uint32_t get_event(void);
bool event_present(void);
void state_machine_measure_temperature(struct gecko_cmd_packet* evt);
void state_machine_proximity_state(struct gecko_cmd_packet* evt);
void indicate_temperature_over_ble(float);

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

// Forward declarations
extern SLEEP_EnergyMode_t sleep_mode_blocked;

typedef uint8_t runqueue;

extern uint8_t current_state;

typedef enum
{
	STATE_ON_IMU,
	STATE_WAIT_FOR_5_MILLIS,
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

typedef enum
{

	FIRST_TIME_PB1_PRESS,
	READ_CHARACTERISTIC_VALUE,
	GATT_CONNECTION_CLOSED,
	GATT_TRANSFER_STATE_SIZE,

} gatt_transfer_states_t;



uint8_t events_present(void);

runqueue get_event(void);
void process_event(runqueue func);
void do_nothing(void);

void scheduler_set_event_UF(void);
void scheduler_set_event_STATE_WAIT_FOR_5_MILLIS(void);
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
