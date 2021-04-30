/*********************************************************************************************
 *  @file i2c.h
 *	@brief This file contains  defines, includes and function prototypes for i2c.c
 *
 *  @author : Rajat Chaple (GATT client code)
 *
 *
 *  @date      April 29, 2020 (last update)
 **********************************************************************************************/

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

//Includes
#include "em_i2c.h"
#include "i2cspm.h"
#include "gpio.h"

//defines
#define CMD_MEASURE_TEMPERATURE 	  		(0xF3)

//function prototypes
void i2c_init(void);
void I2C0_enable(bool);
I2C_TransferReturn_TypeDef i2c_write_write(uint8_t* data);
I2C_TransferReturn_TypeDef i2c_write(uint8_t*);
I2C_TransferReturn_TypeDef i2c_read(uint8_t* data);

#endif /* SRC_I2C_H_ */

#else
#endif
