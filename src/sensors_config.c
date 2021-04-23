/*********************************************************************************************
 *  @file sensors_config.c
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
#include "sensors_config.h"
#include "gpio.h"

uint8_t read_data;

/*****DEFINES*****/

#define TEST_COUNT_DATA_RANGE_VALIDITY	(2)
#define TEST_COUNT_GESTURE				(1)
//#define PROXIMITY_COMMAND_REG_ADDRESS	0x80
//#define PROXIMITY_PERIODIC_MEAS_MODE 	0b11100011


static uint32_t event_status;

#ifndef USE_I2CSPM

extern I2C_TransferReturn_TypeDef transfer_status;

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
#else

#include "em_i2c.h"

void blocking_read_i2c(uint8_t reg)
{
	I2C_TransferReturn_TypeDef ret = i2c_write(reg);

	if(ret != i2cTransferDone)
		LOG_INFO("i2c read failed 1");

	ret = i2c_read(reg);

	if(ret != i2cTransferDone)
		LOG_INFO("i2c read failed 2");
}

void blocking_write_i2c(uint8_t reg, uint8_t data)
{
	uint8_t data_2bytes[2];
	data_2bytes[0] = reg;
	data_2bytes[1] = data;
	I2C_TransferReturn_TypeDef ret = i2c_write_write(data_2bytes);

	if(ret != i2cTransferDone)
		LOG_INFO("i2c write failed 1");

//	ret = i2c_write(data);
//
//	if(ret != i2cTransferDone)
//		LOG_INFO("i2c write failed 2");
}

#endif


void proximity_sensor_config()
{
	int i;

	blocking_write_i2c(0x80, 0b10000011);
	blocking_write_i2c(0x82, 0b00000011);	//~4 readings per second
	blocking_write_i2c(0x83, 2);
#ifdef INTERRUPT_BASED_PROXIMITY_MEASUREMENT
	//setting up thresholds for interrupt generations
	//Low Threshold
	blocking_write_i2c(0x8A, 0);
	blocking_write_i2c(0x8B, 0);
	//HIGH Threshold
	blocking_write_i2c(0x8C, ((PROXIMITY_SENSOR_THRESHOLD_VALUE & 0xFF00) >> 8));
	blocking_write_i2c(0x8D, (PROXIMITY_SENSOR_THRESHOLD_VALUE & 0x00FF));
	//clearing interrrupt from interrupt status register of proximity
	blocking_write_i2c(0x8E,0x01);
#endif


	LOG_INFO("********************************");
	LOG_INFO("Reading all registers");
	for(uint8_t reg = 0x80; reg< 0x90; reg++)
	{
		blocking_read_i2c(reg, &read_data);
		LOG_INFO("cmd register : %x value : %x", reg, read_data);
	}

//	while(1)
//	{
//		uint8_t prox_dat_rdy_status;
//		blocking_read_i2c(0x80, &read_data);
//		prox_dat_rdy_status = (read_data & 0b00100000) >> 5;
//
//		uint16_t prox_readings = 0;
//		blocking_read_i2c(0x87, &read_data);
//		prox_readings = read_data;
//		prox_readings <<= 8;
//		blocking_read_i2c(0x88, &read_data);
//		prox_readings |= read_data;
//
//		LOG_INFO("Prox_dat_rdy status : %x value : %d",
//				prox_dat_rdy_status, 	//is prox reading ready
//				prox_readings);
//
//
//		i = 1000000;while(i--);
//	}

}


uint16_t proximity_sensor_read(proximity_sensor_read_t data_to_be_read)
{
	uint8_t prox_dat_rdy_status = 0;
	uint16_t prox_readings = 0;

	switch(data_to_be_read)
	{
		case PROXIMITY_PRODUCT_ID:
			blocking_read_i2c(0x81, &read_data);
			prox_readings = read_data;
			break;

		case IR_LED_CURRENT:
			blocking_read_i2c(0x81, &read_data);
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
			blocking_read_i2c(0x87, &read_data);
			prox_readings = read_data;
			prox_readings <<= 8;
			blocking_read_i2c(0x88, &read_data);
			prox_readings |= read_data;
			break;

		default:
			prox_readings = -1;
			break;

	}

//	while(prox_dat_rdy_status == 0)
//		{	gpioLed0SetOn();
//			blocking_read_i2c(0x80, &read_data);
//			prox_dat_rdy_status = (read_data & 0b00100000) >> 5;
//		}
//	gpioLed0SetOff();
//
//			blocking_read_i2c(0x87, &read_data);
//			prox_readings = read_data;
//			prox_readings <<= 8;
//			blocking_read_i2c(0x88, &read_data);
//			prox_readings |= read_data;

	return prox_readings;
}

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

//	while(test_count)
//	{
//		if(gesture_event == false)
//		{
//			{
//				gpioLed1SetOn();
//				gesture_event = true;
//			}
//		}
//		else
//		{
//			if(proximity_sensor_read() < PROXIMITY_SENSOR_THRESHOLD_VALUE)
//			{
//				gpioLed1SetOff();
//				gesture_event = false;
//				test_count--;
//			}
//		}
//
//	}
	LOG_INFO("Test passed for proximity sensor");

}

#else
#endif
