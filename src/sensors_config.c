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

uint8_t read_data;

/*****DEFINES*****/

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
	blocking_write_i2c(0x82, 0b00000001);
	blocking_write_i2c(0x83, 15);
//	blocking_write_i2c(0x80, 0b10001000);


	LOG_INFO("********************************");
	for(uint8_t reg = 0x80; reg< 0x90; reg++)
	{
		blocking_read_i2c(reg, &read_data);
		LOG_INFO("cmd register : %x value : %x", reg, read_data);
	}
//
	while(1)
	{
		uint8_t prox_dat_rdy_status;
		blocking_read_i2c(0x80, &read_data);
		prox_dat_rdy_status = (read_data & 0b00100000) >> 5;

		uint16_t prox_readings = 0;
		blocking_read_i2c(0x87, &read_data);
		prox_readings = read_data;
		prox_readings <<= 8;
		blocking_read_i2c(0x88, &read_data);
		prox_readings |= read_data;

		LOG_INFO("Prox_dat_rdy status : %x value : %d",
				prox_dat_rdy_status, 	//is prox reading ready
				prox_readings);


		i = 1000000;while(i--);
	}

}

#else
#endif
