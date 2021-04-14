/*********************************************************************************************
 *  @file i2c.h
 *	@brief This file contains  defines, includes and function prototypes for i2c.c
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 11, 2021
 **********************************************************************************************/
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
I2C_TransferReturn_TypeDef i2c_write(uint8_t);
I2C_TransferReturn_TypeDef i2c_read();

#endif /* SRC_I2C_H_ */
