/*********************************************************************************************
 *  @file scheduler.c
 *	@brief This file contains Event management (scheduling)
 *
 *  @authors : Rajat Chaple (GATT client code)
 *  		   Sundar Krishnakumar (GATT server code)
 *
 *  @date      April 29, 2020 (last update)
 **********************************************************************************************/

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

#define INCLUDE_LOG_DEBUG 0
#include "log.h"
#include "scheduler.h"
#include "proximity.h"
#include "main.h"
#include "timers.h"
#include "gpio.h"
#include "i2c.h"



uint16_t pobp_tut_timer_seconds_initial_value = INITIAL_TIME_UNTIL_TRIGGER_FOR_BAD_POSTURE_S;
uint32_t inactive_timer_seconds = THRESHOLD_TIME_ACTIVE_TO_INACTIVE_S;

uint16_t pobp_tut_timer_seconds = INITIAL_TIME_UNTIL_TRIGGER_FOR_BAD_POSTURE_S;

static uint32_t event_status  = 0x00000000;
extern bool is_bad_posture;
float temperature = 0;
char temperature_str[13];
const uint8_t command_for_clearing_prox_interrupt[2] = {0x8E,0x01};	//register 8E and data 00


/** ---------------------------------------------------------------------------------------------------------
 * @brief state machine for measuring temperature over i2c in event driven mode using followingstate machines
 *
 * @param None
 * @return None
 *--------------------------------------------------------------------------------------------------------- **/
void event_handler_proximity_state(struct gecko_cmd_packet* evt)
{

	uint32_t event = evt->data.evt_system_external_signal.extsignals;

	switch(event)
	{
	case LETIMER_UF_INTERRUPT_EVENT:

			if(inactive_timer_seconds > 0)
				{
					inactive_timer_seconds -= (LETIMER_PERIOD_MS / MILLISECONDS_IN_SECOND);
					displayPrintf(DISPLAY_ROW_INACTIVITY, "ACTIVE (%d)", (inactive_timer_seconds + 1));
					// turn LED1 off
					gpioLed1SetOff();
				}
			else
				{
					displayPrintf(DISPLAY_ROW_INACTIVITY, "INACTIVE");
					// turn on LED1 here
					gpioLed1SetOn();
				}

			if(pobp_tut_timer_seconds > 0)
				{
					if(is_bad_posture == true)
					{
						pobp_tut_timer_seconds -= (LETIMER_PERIOD_MS / MILLISECONDS_IN_SECOND);
						displayPrintf(DISPLAY_ROW_POSTURE, "Bad Pos TO: %d", (pobp_tut_timer_seconds + 1));
					}
					else
					{
						displayPrintf(DISPLAY_ROW_POSTURE, "GOOD POSTURE", (pobp_tut_timer_seconds + 1));
						pobp_tut_timer_seconds = pobp_tut_timer_seconds_initial_value;
						// turn off led0
						gpioLed0SetOff();
					}
				}
			else
				{
					displayPrintf(DISPLAY_ROW_POSTURE, "BAD POSTURE");
					// turn on led0
					gpioLed0SetOn();

				}

			LOG_DEBUG("---------------------------------Time elapsed (inactive since) : %d", inactive_timer_seconds);
			LOG_DEBUG("---------------------------------Time elapsed (POBP TUT timer ) : %d", pobp_tut_timer_seconds);

		break;

	case PROXIMITY_DETECTED:

//		blocking_write_i2c(0x8E, 0x00);
		gpioLed1SetOn();
		LOG_DEBUG("Proximity detected");
		inactive_timer_seconds = THRESHOLD_TIME_ACTIVE_TO_INACTIVE_S;
		i2c_write_write((uint8_t*)&command_for_clearing_prox_interrupt);
		gpioLed1SetOff();
		break;

	case I2C_TRANSFER_COMPLETE:
		LOG_DEBUG("Proximity interrupt cleared");
		break;

	case I2C_TRANSFER_RETRY:	//when NACK is received for transfer
		i2c_write_write((uint8_t*)&command_for_clearing_prox_interrupt);
		break;

	default:
		break;
	}

}

/** -------------------------------------------------------------------------------------------
 * @brief schedule routine to set a scheduler event for PB0 Low to High (called from Critical section)
 *
 * @param None
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void scheduler_set_event_proximity_detected()
{
	gecko_external_signal(PROXIMITY_DETECTED);
} // scheduler_set_event_PB0_switch_high_to_low()


/** -------------------------------------------------------------------------------------------
 * @brief schedule routine to set a scheduler event for PB0 Low to High (called from Critical section)
 *
 * @param None
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void scheduler_set_event_PB0_switch_high_to_low()
{
	gecko_external_signal(PB0_SWITCH_HIGH_TO_LOW);
} // scheduler_set_event_PB0_switch_high_to_low()


/** -------------------------------------------------------------------------------------------
 * @brief schedule routine to set a scheduler event for PB1 Low to High (called from Critical section)
 *
 * @param None
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void scheduler_set_event_PB1_switch_low_to_high()
{
	gecko_external_signal(PB1_SWITCH_LOW_TO_HIGH);
} // scheduler_set_event_PB1_switch_high_to_low()

/** -------------------------------------------------------------------------------------------
 * @brief schedule routine to set a scheduler event for PB0 Low to High (called from Critical section)
 *
 * @param None
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void scheduler_set_event_PB0_switch_low_to_high()
{
	gecko_external_signal(PB0_SWITCH_LOW_TO_HIGH);
} // scheduler_set_event_PB0_switch_low_to_high()

/** -------------------------------------------------------------------------------------------
 * @brief schedule routine to set a scheduler event for Timer0 underflow interrupt (called from Critical section)
 *
 * @param None
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void scheduler_set_event_UF()
{
	//event_status |= LETIMER_UF_INTERRUPT_EVENT;
	gecko_external_signal(LETIMER_UF_INTERRUPT_EVENT);
} // schedulerSetEventUF()

/** -------------------------------------------------------------------------------------------
 * @brief schedule routine to set a scheduler event for COMP1 register match (called from Critical section)
 *
 * @param None
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void scheduler_set_event_COMP1()
{
	//event_status |= LETIMER_COMP1_INTERRUPT_EVENT;
	gecko_external_signal(LETIMER_COMP1_INTERRUPT_EVENT);
} // schedulerSetEventCOMP1()

/** -------------------------------------------------------------------------------------------
 * @brief schedule routine to set a scheduler event when I2C transfer completes
 *
 * @param None
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void scheduler_set_event_I2C_transfer_complete()
{
	event_status |= I2C_TRANSFER_COMPLETE;
	gecko_external_signal(I2C_TRANSFER_COMPLETE);
} // scheduler_set_event_I2C_transfer_complete()


/** -------------------------------------------------------------------------------------------
 * @brief schedule routine to set a scheduler event when I2C transfer received NACK
 *
 * @param None
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void scheduler_set_event_I2C_transfer_retry()
{
	event_status |= I2C_TRANSFER_RETRY;
	gecko_external_signal(I2C_TRANSFER_RETRY);
} // scheduler_set_event_I2C_transfer_retry()


/** -------------------------------------------------------------------------------------------
 * @brief scheduler routine to return 1 event to main()
 *
 * @param None
 * @return theEvent : returns status of events
 *-------------------------------------------------------------------------------------------- **/
uint32_t get_event()
{
	uint32_t theEvent;
	// determine 1 event to return to main()
	for(uint32_t i=1; i<=LAST_EVENT_IN_THE_LIST; i<<=1)
	{
		if(i & event_status)
		{
			theEvent = i;
			break;
		}
	}

	// enter critical section
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	// clear the event
	event_status &= ~theEvent;//~(LETIMER_UF_INTERRUPT_EVENT);
	// exit critical section
	CORE_EXIT_CRITICAL();
	return (theEvent);
} // get_event()

/** -------------------------------------------------------------------------------------------
 * @brief returns true if any event is pending or returns false
 *
 * @param None
 * @return ret :true if any event is pending
 *-------------------------------------------------------------------------------------------- **/
bool event_present()
{
	bool ret = false;
	if(event_status)
	{
		ret = true;
	}

	return ret;
} // events_present()


#else

#include "scheduler.h"



runqueue rq_array[30];
uint8_t rq_ind = 0;
uint8_t read = 0;
uint8_t current_state;
runqueue tmp = 0;

uint8_t events_present(void)
{
	if (read == rq_ind)
	{
		return 0;
	}
	else
	{
		return 1;
	}

}

void scheduler_set_event_UF(void)
{
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(10);

	CORE_EXIT_CRITICAL();

}



void scheduler_set_event_STATE_ACC_STANDBY_SIGNAL_SEND(void)
{

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(20);

	CORE_EXIT_CRITICAL();

}


void scheduler_set_event_STATE_ACC_STANDBY_SIGNAL_STOP(void)
{

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(30);

	CORE_EXIT_CRITICAL();

}


void scheduler_set_event_STATE_ACC_DATA_CFG_STOP(void)
{
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(40);

	CORE_EXIT_CRITICAL();

}


void scheduler_set_event_STATE_ACC_CTRL_REG1_STOP(void)
{

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(50);

	CORE_EXIT_CRITICAL();

}

void scheduler_set_event_STATE_GYRO_STANDBY_SIGNAL_STOP(void)
{

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(60);

	CORE_EXIT_CRITICAL();

}

void scheduler_set_event_STATE_GYRO_RESET_SIGNAL_NACK(void)
{

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(70);

	CORE_EXIT_CRITICAL();


}

void scheduler_set_event_STATE_GYRO_CTRL_REG0_STOP(void)
{

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(80);

	CORE_EXIT_CRITICAL();

}

void scheduler_set_event_STATE_GYRO_CTRL_REG1_STOP(void)
{

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(90);

	CORE_EXIT_CRITICAL();


}




void scheduler_set_event_STATE_WAIT_FOR_65_MILLIS(void)
{
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(100);

	CORE_EXIT_CRITICAL();

}




void scheduler_set_event_STATE_ACC_MEASURE_STOP(void)
{
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(110);

	CORE_EXIT_CRITICAL();

}


void scheduler_set_event_STATE_GYRO_MEASURE_STOP(void)
{

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(120);

	CORE_EXIT_CRITICAL();

}


void scheduler_set_event_STATE_SEND_IMU_INDICATION(void)
{

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	gecko_external_signal(130);

	CORE_EXIT_CRITICAL();

}


void scheduler_set_event_common(uint32_t signal)
{
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section

	if (((rq_ind + 1) % 30) != read)
	{
		rq_ind = (rq_ind + 1) % 30;
		rq_array[rq_ind] = (runqueue)signal; // starts from index=0
	}


	CORE_EXIT_CRITICAL();

}



runqueue get_event(void)
{
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section
	if (read == rq_ind) // runqueue empty condition
	{
		CORE_EXIT_CRITICAL();
		return 0;
	}
	read = (read + 1) % 30; // reads from index=0
	tmp = rq_array[read];
	rq_array[read] = 0;

	CORE_EXIT_CRITICAL();

	return tmp;


}

// state machine
void state_machine(runqueue func)
{



	static uint8_t next_state  = STATE_ON_IMU;
	current_state = next_state;

	switch (current_state)
	{


		case STATE_ON_IMU:
			if (func == 10)
			{

				SLEEP_SleepBlockEnd(sleep_mode_blocked);
				SLEEP_SleepBlockBegin(sleepEM2);


				turn_on_IMU();
				next_state = STATE_ACC_STANDBY_SIGNAL_SEND;
			}
			break;

		case STATE_ACC_STANDBY_SIGNAL_SEND:
			if (func == 20)
			{

				FXOS_send_standby_signal();
				next_state = STATE_ACC_DATA_CFG_START;
			}
			break;

		case STATE_ACC_DATA_CFG_START:
			if (func == 30)
			{

				FXOS_DATA_CFG_start();
				next_state = STATE_ACC_CTRL_REG1_START;
			}
			break;

		case STATE_ACC_CTRL_REG1_START:
			if (func == 40)
			{

				FXOS_CTRL_REG1_signal_start();
				next_state = STATE_GYRO_STANDBY_SIGNAL_SEND;
			}
			break;

		case STATE_GYRO_STANDBY_SIGNAL_SEND:
			if (func == 50)
			{

				FXAS_standby_signal_send();
				next_state = STATE_GYRO_RESET_SIGNAL_SEND;
			}
			break;

		case STATE_GYRO_RESET_SIGNAL_SEND:
			if (func == 60)
			{

				FXAS_reset_signal_send();
				next_state = STATE_GYRO_CTRL_REG0_START;
			}
			break;

		case STATE_GYRO_CTRL_REG0_START:
			if (func == 70)
			{

				FXAS_CTRL_REG0_signal_start();			
				next_state = STATE_GYRO_CTRL_REG1_START;
				
			}
			break;



		case STATE_GYRO_CTRL_REG1_START:
			if (func == 80)
			{

				FXAS_CTRL_REG1_signal_start();
				next_state = STATE_WAIT_FOR_65_MILLIS;
			}
			break;

		// 60 ms + 1/ODR is needed for transition from standby to active mode
		case STATE_WAIT_FOR_65_MILLIS:
			if (func == 90)
			{

				wait_for_65_millis();
				next_state = STATE_ACC_MEASURE_START;

			}
			break;

		case STATE_ACC_MEASURE_START:
			if (func == 100)
			{

				FXOS_measure_start();
				next_state = STATE_GYRO_MEASURE_START;
			}
			break;


		case STATE_GYRO_MEASURE_START:
			if (func == 110)
			{

				FXAS_measure_start();
				next_state = STATE_GYRO_MEASURE_STOP;
			}
			break;


		case STATE_GYRO_MEASURE_STOP:
			if (func == 120)
			{

				#if BOND_DISCONNECT

				#else

					SLEEP_SleepBlockEnd(sleepEM2);
					SLEEP_SleepBlockBegin(sleep_mode_blocked);

				#endif
				FXAS_measure_stop_off_read();

				next_state = STATE_ON_IMU;
			}
			break;

	}

}

#endif
