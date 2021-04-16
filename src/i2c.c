/*********************************************************************************************
 *  @file i2c.c
 *	@brief This file contains i2c configuration and data transfer functions
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 11, 2021
 *
 *  Updated by Rajat Chaple Feb 20, 2020. Changed - Polling I2C transfer to Interrupt based
 **********************************************************************************************/
#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT
#include "log.h"
#include "i2c.h"



#define SENSOR_I2C_ADDRESS 0x13	//temperature sensor Si70210x40

I2C_TransferSeq_TypeDef seq;
I2C_TransferReturn_TypeDef transfer_status;
uint8_t write_data = (uint8_t)CMD_MEASURE_TEMPERATURE;
uint8_t read_data[2] = {0};

uint8_t read_data_i2c = 0;
/** -------------------------------------------------------------------------------------------
 * @brief i2c routine to set up pins, freq and channel
 *
 * @param true/false
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void i2c_init()
{

	I2CSPM_Init_TypeDef init =
  { I2C0,                       /* Use I2C instance 0 */                       \
    gpioPortC,                  /* SCL port */                                 \
    10,                         /* SCL pin */                                  \
    gpioPortC,                  /* SDA port */                                 \
    11,                         /* SDA pin */                                  \
    14,                         /* Location of SCL */                          \
    16,                         /* Location of SDA */                          \
    0,                          /* Use currently configured reference clock */ \
    I2C_FREQ_STANDARD_MAX,      /* Set to standard rate  */                    \
	i2cClockHLRStandard,        /* Set to use 4:4 low/high duty cycle */       \
  };


	//Setting up clock and pins
	I2CSPM_Init(&init);

}

/** -------------------------------------------------------------------------------------------
 * @brief i2c routine to connect or disconnect i2c sensor
 *
 * @param true/false
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void I2C0_enable(bool value)
{
	if(value == true)
		{
			gpio_I2C_sensor_enable();
		}
	else
		{
			gpio_I2C_sensor_disable();
		}
}

#ifndef USE_I2CSPM
/** -------------------------------------------------------------------------------------------
 * @brief i2c routine to write data to sensor over i2c
 *
 * @param true/false
 * @return None
 *-------------------------------------------------------------------------------------------- **/
I2C_TransferReturn_TypeDef i2c_write(uint8_t* data)
{
//	I2C_TransferSeq_TypeDef write_seq;
//	LOG_INFO("writing data");
	NVIC_DisableIRQ(I2C0_IRQn);
	//change this remove data
	seq.addr = SENSOR_I2C_ADDRESS << 1;
	seq.flags = I2C_FLAG_WRITE;
	seq.buf[0].data = data;
	seq.buf[0].len = 1;
	NVIC_EnableIRQ(I2C0_IRQn);
	transfer_status = I2C_TransferInit(I2C0, &seq);
	return transfer_status;
}

/** -------------------------------------------------------------------------------------------
 * @brief i2c routine to read data from sensor over i2c
 *
 * @param true/false
 * @return Temperature value
 *-------------------------------------------------------------------------------------------- **/
I2C_TransferReturn_TypeDef i2c_read(uint8_t* data_buffer)
{
//	LOG_INFO("reading data");
	NVIC_DisableIRQ(I2C0_IRQn);
	//I2C_TransferSeq_TypeDef read_seq;
	seq.addr = SENSOR_I2C_ADDRESS << 1;
	seq.flags = I2C_FLAG_READ;
	//seq.buf[0].data = read_data;
	seq.buf[0].data = data_buffer;
	seq.buf[0].len = 1;

	NVIC_EnableIRQ(I2C0_IRQn);
	transfer_status = I2C_TransferInit(I2C0, &seq);

	return transfer_status;
}

/** -------------------------------------------------------------------------------------------
 * @brief i2c routine to write data to sensor over i2c
 *
 * @param true/false
 * @return None
 *-------------------------------------------------------------------------------------------- **/
I2C_TransferReturn_TypeDef i2c_write_write(uint8_t* data)
{
//	I2C_TransferSeq_TypeDef write_seq;
	I2C_TransferReturn_TypeDef ret;

	NVIC_DisableIRQ(I2C0_IRQn);
	seq.addr = SENSOR_I2C_ADDRESS << 1;
	seq.flags = I2C_FLAG_WRITE_WRITE;
	seq.buf[0].data = data;
	seq.buf[0].len = 2;

	NVIC_EnableIRQ(I2C0_IRQn);
	transfer_status = I2C_TransferInit(I2C0, &seq);

	return transfer_status;
}

#else

/** -------------------------------------------------------------------------------------------
 * @brief i2c routine to write data to sensor over i2c
 *
 * @param true/false
 * @return None
 *-------------------------------------------------------------------------------------------- **/
I2C_TransferReturn_TypeDef i2c_write(uint8_t data*)
{
	I2C_TransferSeq_TypeDef write_seq;
	I2C_TransferReturn_TypeDef ret;

	write_seq.addr = SENSOR_I2C_ADDRESS << 1;
	write_seq.flags = I2C_FLAG_WRITE;
	write_seq.buf[0].data = data;
	write_seq.buf[0].len = 1;
	ret = I2CSPM_Transfer(I2C0, &write_seq);
	return ret;
}

/** -------------------------------------------------------------------------------------------
 * @brief i2c routine to write data to sensor over i2c
 *
 * @param true/false
 * @return None
 *-------------------------------------------------------------------------------------------- **/
I2C_TransferReturn_TypeDef i2c_write_write(uint8_t* data)
{
	I2C_TransferSeq_TypeDef write_seq;
	I2C_TransferReturn_TypeDef ret;

	write_seq.addr = SENSOR_I2C_ADDRESS << 1;
	write_seq.flags = I2C_FLAG_WRITE_WRITE;
	write_seq.buf[0].data = data;
	write_seq.buf[0].len = 2;
	ret = I2CSPM_Transfer(I2C0, &write_seq);
	return ret;
}

/** -------------------------------------------------------------------------------------------
 * @brief i2c routine to read data from sensor over i2c
 *
 * @param true/false
 * @return Temperature value
 *-------------------------------------------------------------------------------------------- **/
I2C_TransferReturn_TypeDef i2c_read(uint8_t* data_buffer)
{
//	uint8_t read_data[2] = {0};
//	uint16_t temperature_code = 0;
	I2C_TransferSeq_TypeDef seq;
	I2C_TransferReturn_TypeDef ret;


	seq.addr = SENSOR_I2C_ADDRESS << 1;
	seq.flags = I2C_FLAG_READ;
	seq.buf[0].data = data_buffer;
	seq.buf[0].len = 1;
	ret = I2CSPM_Transfer(I2C0, &seq);

	return ret;
}

#endif

#else
#endif
