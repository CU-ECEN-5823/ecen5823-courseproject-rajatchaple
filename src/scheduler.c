/*********************************************************************************************
 *  @file scheduler.c
 *	@brief This file contains Event management (scheduling)
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 11, 2021
 *
 *  Updated by Rajat Chaple Feb 20, 2020. State machine for I2C temp read added
 *  Updated by Rajat Chaple Feb 26, 2021. Temp sent as an indication
 **********************************************************************************************/

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

//#define INCLUDE_LOG_DEBUG 1
#include "log.h"
#include "scheduler.h"

static uint32_t event_status  = 0x00000000;
// extern ble_status_t ble_status;

float temperature = 0;
char temperature_str[13];
// extern uint8_t read_data[2];



/** ---------------------------------------------------------------------------------------------------------
 * @brief state machine for measuring temperature over i2c in event driven mode using followingstate machines
 *	enum temp_sensor_states{
 *		STATE0_TIMER_WAIT,
 *		STATE1_WAIT_FOR_POWER_UP,
 *		STATRE2_WAIT_FOT_I2C_WRITE_COMPLETE,
 *		STATE3_WAIT_FOR_I2C_READ_START,
 *		STATE4_WAIT_FOR_I2C_READ_COMPLETE,
 *		MY_NUM_STATES
 *	};
 *
 * @param None
 * @return None
 *--------------------------------------------------------------------------------------------------------- **/
#if 0
void state_machine_proximity_state(struct gecko_cmd_packet* evt)
{
	uint16_t temperature_code = 0;

	uint32_t event = evt->data.evt_system_external_signal.extsignals;


	temp_sensor_states_t current_state;
	static temp_sensor_states_t next_state = STATE0_TIMER_WAIT;

	current_state = next_state;

	switch(current_state)
				{
				case STATE0_TIMER_WAIT:
					if(event == LETIMER_UF_INTERRUPT_EVENT)
					{
						LOG_DEBUG("LETIMER0 Underflow occurred. Initiating Temperature measurement");
						//I2C0_enable(true);	//Power on  and connect Si7021
						timerWaitUs(WAIT_BOOT_TIME_SI7021); //wait for boot time of I2C module
						next_state = STATE1_WAIT_FOR_POWER_UP;
					}

					break;

				case STATE1_WAIT_FOR_POWER_UP:
					if(event == LETIMER_COMP1_INTERRUPT_EVENT)
					{
						LOG_DEBUG("Si7021 ON and ready for communication. Sending wrie command for temperature measurement");
						LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);	// Disable COMP1 Interrupt
						i2c_write(CMD_MEASURE_TEMPERATURE);
						next_state = STATE2_WAIT_FOT_I2C_WRITE_COMPLETE;
						SLEEP_SleepBlockBegin(sleepEM2);	//Allow energy mode to EM1 as I2C requires minimum EM1 to be operational
					}
					break;

				case STATE2_WAIT_FOT_I2C_WRITE_COMPLETE:
					if(event == I2C_TRANSFER_COMPLETE)
					{
						LOG_DEBUG("Write command sent successfully");
						SLEEP_SleepBlockEnd(sleepEM2);		//As i2c transfer is finished, allow low energy mode of EM3
						timerWaitUs(TIME_FOR_TEMPERATURE_MEASUREMENT);	//10.8 ms is maximum wait time for temperature to be ready
						next_state = STATE3_WAIT_FOR_I2C_READ_START;
					}
					if(event == I2C_TRANSFER_RETRY)
					{
						LOG_DEBUG("Received NACK. Re-attempting I2C write");
						i2c_write(CMD_MEASURE_TEMPERATURE);
					}
					break;

				case STATE3_WAIT_FOR_I2C_READ_START:
					if(event == LETIMER_COMP1_INTERRUPT_EVENT)
					{
						LOG_DEBUG("Temperature measurement is expected to be ready. Send read command for temperature value");
						LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);	// Disable COMP1 Interrupt
						i2c_read();
						next_state = STATE4_WAIT_FOR_I2C_READ_COMPLETE;
						SLEEP_SleepBlockBegin(sleepEM2);	//Allow energy mode to EM1 as I2C requires minimum EM1 to be operational
					}
					break;

				case STATE4_WAIT_FOR_I2C_READ_COMPLETE:
					if(event == I2C_TRANSFER_COMPLETE)
					{
						LOG_DEBUG("Temperature read successful. Send converted value over bluetooth if Indication is ON");
						SLEEP_SleepBlockEnd(sleepEM2);		//As i2c transfer is finished, allow low energy mode of EM3
						//I2C0_enable(false);		//Power off  and disconnect Si7021

						//converting two uint8 data to uint16
						temperature_code = read_data[0];
						temperature_code = temperature_code<<8;
						temperature_code |= read_data[1];

						temperature = ((175.72*temperature_code)/65536) - 46.85;
						LOG_INFO("T : %.2f deg celcius", temperature); //printing integer converted value of temperature

						//displaying
						sprintf(temperature_str,"temp = %.2f",temperature);
						displayPrintf(DISPLAY_ROW_TEMPVALUE, temperature_str);

						if(ble_status.htp_indication_status == true)
						  {
							indicate_temperature_over_ble(temperature);
						  }

						next_state = STATE0_TIMER_WAIT;
					}
					if(event == I2C_TRANSFER_RETRY)
					{
						i2c_read();
						LOG_DEBUG("Received NACK. Re-attempting I2C read");
					}
					break;

				default:
					break;
				}
}



/** ---------------------------------------------------------------------------------------------------------
 * @brief state machine for measuring temperature over i2c in event driven mode using followingstate machines
 *	enum temp_sensor_states{
 *		STATE0_TIMER_WAIT,
 *		STATE1_WAIT_FOR_POWER_UP,
 *		STATRE2_WAIT_FOT_I2C_WRITE_COMPLETE,
 *		STATE3_WAIT_FOR_I2C_READ_START,
 *		STATE4_WAIT_FOR_I2C_READ_COMPLETE,
 *		MY_NUM_STATES
 *	};
 *
 * @param None
 * @return None
 *--------------------------------------------------------------------------------------------------------- **/

void state_machine_measure_temperature(struct gecko_cmd_packet* evt)
{
	uint16_t temperature_code = 0;

	uint32_t event = evt->data.evt_system_external_signal.extsignals;


	temp_sensor_states_t current_state;
	static temp_sensor_states_t next_state = STATE0_TIMER_WAIT;

	current_state = next_state;

	switch(current_state)
				{
				case STATE0_TIMER_WAIT:
					if(event == LETIMER_UF_INTERRUPT_EVENT)
					{
						LOG_DEBUG("LETIMER0 Underflow occurred. Initiating proximity sense");
						//I2C0_enable(true);	//Power on  and connect Si7021
						timerWaitUs(WAIT_BOOT_TIME_SI7021); //wait for boot time of I2C module
						next_state = STATE1_WAIT_FOR_POWER_UP;
					}

					break;

				case STATE1_WAIT_FOR_POWER_UP:
					if(event == LETIMER_COMP1_INTERRUPT_EVENT)
					{
						LOG_DEBUG("Si7021 ON and ready for communication. Sending wrie command for temperature measurement");
						LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);	// Disable COMP1 Interrupt
						i2c_write(CMD_MEASURE_TEMPERATURE);
						next_state = STATE2_WAIT_FOT_I2C_WRITE_COMPLETE;
						SLEEP_SleepBlockBegin(sleepEM2);	//Allow energy mode to EM1 as I2C requires minimum EM1 to be operational
					}
					break;

				case STATE2_WAIT_FOT_I2C_WRITE_COMPLETE:
					if(event == I2C_TRANSFER_COMPLETE)
					{
						LOG_DEBUG("Write command sent successfully");
						SLEEP_SleepBlockEnd(sleepEM2);		//As i2c transfer is finished, allow low energy mode of EM3
						timerWaitUs(TIME_FOR_TEMPERATURE_MEASUREMENT);	//10.8 ms is maximum wait time for temperature to be ready
						next_state = STATE3_WAIT_FOR_I2C_READ_START;
					}
					if(event == I2C_TRANSFER_RETRY)
					{
						LOG_DEBUG("Received NACK. Re-attempting I2C write");
						i2c_write(CMD_MEASURE_TEMPERATURE);
					}
					break;

				case STATE3_WAIT_FOR_I2C_READ_START:
					if(event == LETIMER_COMP1_INTERRUPT_EVENT)
					{
						LOG_DEBUG("Temperature measurement is expected to be ready. Send read command for temperature value");
						LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);	// Disable COMP1 Interrupt
						i2c_read();
						next_state = STATE4_WAIT_FOR_I2C_READ_COMPLETE;
						SLEEP_SleepBlockBegin(sleepEM2);	//Allow energy mode to EM1 as I2C requires minimum EM1 to be operational
					}
					break;

				case STATE4_WAIT_FOR_I2C_READ_COMPLETE:
					if(event == I2C_TRANSFER_COMPLETE)
					{
						LOG_DEBUG("Temperature read successful. Send converted value over bluetooth if Indication is ON");
						SLEEP_SleepBlockEnd(sleepEM2);		//As i2c transfer is finished, allow low energy mode of EM3
						//I2C0_enable(false);		//Power off  and disconnect Si7021

						//converting two uint8 data to uint16
						temperature_code = read_data[0];
						temperature_code = temperature_code<<8;
						temperature_code |= read_data[1];

						temperature = ((175.72*temperature_code)/65536) - 46.85;
						LOG_INFO("T : %.2f deg celcius", temperature); //printing integer converted value of temperature

						//displaying
						sprintf(temperature_str,"temp = %.2f",temperature);
						displayPrintf(DISPLAY_ROW_TEMPVALUE, temperature_str);

						if(ble_status.htp_indication_status == true)
						  {
							//indicate_temperature_over_ble(temperature);
						  }

						next_state = STATE0_TIMER_WAIT;
					}
					if(event == I2C_TRANSFER_RETRY)
					{
						i2c_read();
						LOG_DEBUG("Received NACK. Re-attempting I2C read");
					}
					break;

				default:
					break;
				}
}

#endif

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
//	gecko_external_signal(I2C_TRANSFER_COMPLETE);
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
//	gecko_external_signal(I2C_TRANSFER_RETRY);
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



void scheduler_set_event_STATE_WAIT_FOR_5_MILLIS(void)
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
				turn_on_IMU();
				next_state = STATE_WAIT_FOR_5_MILLIS;
			}
			break;

		case STATE_WAIT_FOR_5_MILLIS:
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

				FXAS_measure_stop_off_read();

				next_state = STATE_ON_IMU;
			}
			break;

	}

}

#endif
