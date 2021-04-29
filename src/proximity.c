/*********************************************************************************************
 *  @file proximity.c
 *	@brief This file contains sensors configuration for
 *			1) proximity sensor
 *
 *  @author : Rajat Chaple
 *  @date Created on: Apr 15, 2021
 **********************************************************************************************/

/*****INCLUDES*****/
#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

#include "i2c.h"
#include "scheduler.h"
#include "log.h"
#include "i2cspmhalconfig.h"
#include "proximity.h"
#include "gpio.h"

uint8_t read_data;

/*****DEFINES*****/

#define TEST_COUNT_DATA_RANGE_VALIDITY	(2)
#define TEST_COUNT_GESTURE				(2)


static uint32_t event_status;

/** -------------------------------------------------------------------------------------------
 * @brief i2c routine to read data from sensor over i2c (blocking read)
 *
 * @param register of proximity sensor, pointer to a data buffer
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void blocking_read_i2c(uint8_t reg, uint8_t* data_buffer)
{
	uint32_t timeout = I2CSPM_TRANSFER_TIMEOUT;//I2CSPM_TRANSFER_TIMEOUT;

	i2c_write(&reg);
	while(1)
	{
		if(event_present())
		{
			event_status = get_event();
			if(event_status & I2C_TRANSFER_COMPLETE)
				break;
			else if(event_status & I2C_TRANSFER_RETRY)
				i2c_write(&reg);
		}
		//timeout to retry in case there are no interrupts to continue I2C transfer
		if(!(timeout--))
		{
			i2c_write(&reg);
			timeout = I2CSPM_TRANSFER_TIMEOUT*10;
		}
	}

	i2c_read(data_buffer);
	while(1)
	{
		if(event_present())
		{
			event_status = get_event();
			if(event_status & I2C_TRANSFER_COMPLETE)
				break;
			else if(event_status & I2C_TRANSFER_RETRY)
				i2c_read(data_buffer);
		}
		if(!(timeout--))
		{
			i2c_read(data_buffer);
			timeout = I2CSPM_TRANSFER_TIMEOUT*10;
		}
	}


}

/** -------------------------------------------------------------------------------------------
 * @brief i2c routine to write data to sensor over i2c (blocking write)
 *
 * @param register of proximity sensor, pointer to a data buffer
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void blocking_write_i2c(uint8_t reg, uint8_t data)
{

	uint32_t timeout = I2CSPM_TRANSFER_TIMEOUT;//I2CSPM_TRANSFER_TIMEOUT;

	uint8_t data_2bytes[2];
	data_2bytes[0] = reg;
	data_2bytes[1] = data;


	i2c_write_write(data_2bytes);
	while(1)
	{
		if(event_present())
		{
			event_status = get_event();
			if(event_status & I2C_TRANSFER_COMPLETE)
				break;
			else if(event_status & I2C_TRANSFER_RETRY)
				i2c_write_write(data_2bytes);
		}

		if(!(timeout--))
		{
			i2c_write_write(data_2bytes);
			timeout = I2CSPM_TRANSFER_TIMEOUT*10;
		}

	}
}


/** -------------------------------------------------------------------------------------------
 * @brief configure proximity sensor
 *
 * @param None
 * @return None
 * resource: vcnl4010.pdf
 *-------------------------------------------------------------------------------------------- **/
void proximity_sensor_config()
{
	blocking_write_i2c(0x80, 0b10000011);	//Enable proximity measurement
	blocking_write_i2c(0x82, 0b00000011);	//~16 readings per second
	blocking_write_i2c(0x83, 15);			//Setting up IR LED current to 150mA
	//setting up thresholds for interrupt generations
	//Low Threshold
	blocking_write_i2c(0x8A, 0);
	blocking_write_i2c(0x8B, 0);
	//HIGH Threshold
	blocking_write_i2c(0x8C, ((PROXIMITY_SENSOR_THRESHOLD_VALUE & 0xFF00) >> 8));
	blocking_write_i2c(0x8D, (PROXIMITY_SENSOR_THRESHOLD_VALUE & 0x00FF));
	//clearing interrrupt from interrupt status register of proximity
	blocking_write_i2c(0x8E,0x01);


	LOG_DEBUG("********************************");
	LOG_DEBUG("Reading all registers");
	for(uint8_t reg = 0x80; reg< 0x90; reg++)
	{
		blocking_read_i2c(reg, &read_data);
		LOG_DEBUG("cmd register : %x value : %x", reg, read_data);
	}

}

/** -------------------------------------------------------------------------------------------
 * @brief wrapper for reading proximity sensor's register values
 *
 * @param data to be read
 * @return None
 * resource: vcnl4010.pdf
 *-------------------------------------------------------------------------------------------- **/
uint16_t proximity_sensor_read(proximity_sensor_read_t data_to_be_read)
{
	uint8_t prox_dat_rdy_status = 0;
	uint16_t prox_readings = 0;

	switch(data_to_be_read)
	{
		case PROXIMITY_PRODUCT_ID:
			blocking_read_i2c(0x81, &read_data);	//checking if product id = 0x21
			prox_readings = read_data;
			break;

		case IR_LED_CURRENT:
			blocking_read_i2c(0x83, &read_data);	//reading IR LED current register
			prox_readings = (read_data & 0b00111111) * 10;
			break;

		case PROXIMITY_SENSED_VALUE_RAW:
			while(prox_dat_rdy_status == 0)
			{
				gpioLed0SetOn();
				blocking_read_i2c(0x80, &read_data);
				prox_dat_rdy_status = (read_data & 0b00100000) >> 5;
			}
			gpioLed0SetOff();
			blocking_read_i2c(0x87, &read_data);	//Reading proximity result register
			prox_readings = read_data;
			prox_readings <<= 8;
			blocking_read_i2c(0x88, &read_data);	//Reading proximity result register
			prox_readings |= read_data;
			break;

		default:
			prox_readings = -1;
			break;

	}

	return prox_readings;
}

/** -------------------------------------------------------------------------------------------
 * @brief test sequence for proximity sensor.
 * 			checks if
 * 			-I2C communication with sensor is successful
 * 			-data is in valid range
 * 			-gesture for TEST_COUNT_GESTURE times (program is blocked until gestures are performed)
 *
 * @param none
 * @return None
 * resource: vcnl4010.pdf
 *-------------------------------------------------------------------------------------------- **/
void test_proximity_sensor()
{
	bool gesture_event = false;
	int read_data;
	uint8_t test_count = 0;
	uint8_t test;
	test = STATE_TEST_COMMUNICATION;

	while(test <= STATE_TEST_GESTURE)
	{
		switch(test)
		{
			case STATE_TEST_COMMUNICATION:
				if(proximity_sensor_read(PROXIMITY_PRODUCT_ID) == 0x21)
				{
					test++;	//advance to next state
					LOG_INFO("Test passed for: Sensor's communication");
				}
				break;

			case STATE_TEST_DATA_IN_VALID_RANGE:
				read_data = proximity_sensor_read(PROXIMITY_SENSED_VALUE_RAW);
				if(read_data >= 0 && read_data < 65535)
					test_count++;
				if(test_count >= TEST_COUNT_DATA_RANGE_VALIDITY)
				{
					test_count = 0;
					test++;	//advance to next state
					LOG_INFO("Test passed for: Data in valid Range");
				}
				break;

			case STATE_TEST_GESTURE:
				if(gesture_event == false)
				{
					if(proximity_sensor_read(PROXIMITY_SENSED_VALUE_RAW) > PROXIMITY_SENSOR_THRESHOLD_VALUE)
					{
						gpioLed1SetOn();
						gesture_event = true;
					}
				}
				else
				{
					if(proximity_sensor_read(PROXIMITY_SENSED_VALUE_RAW) < PROXIMITY_SENSOR_THRESHOLD_VALUE)
					{
						gpioLed1SetOff();
						gesture_event = false;
						test_count++;
					}
				}
				if(test_count >= TEST_COUNT_GESTURE)
				{
					test_count = 0;
					test++;	//advance to next state
					LOG_INFO("Test passed for: Gestures");
				}
				break;
			default:
				break;
		}
	}

	LOG_DEBUG("Test passed for proximity sensor");

}

#else
#endif
