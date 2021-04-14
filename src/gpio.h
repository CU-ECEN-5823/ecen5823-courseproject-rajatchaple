/*********************************************************************************************
 *  @file gpio.h
 *	@brief includes, defines and function prototypes for gpio.c
 *
 *  @author : Dan Walkes
 *  @date Created on: Dec 12, 2018
 *
 *  Updated by Dave Sluiter Sept 7, 2020. moved #defines from .c to .h file.
 *  Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.
 *  Updated by Rjajat Chaple Feb 12, 2020. I2C sensor enable gpio config added
**********************************************************************************************/
#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_

#include <stdbool.h>
#include <string.h>
#include "em_gpio.h"
#include "main.h"


#define	LED0_port  				(gpioPortF)
#define LED0_pin   				(4)
#define LED1_port  				(gpioPortF)
#define LED1_pin   				(5)
#define I2C_SENSOR_ENABLE_port	(gpioPortD)
#define I2C_SENSOR_ENABLE_pin	(15)
#define DISP_EXTCOMIN_port		(gpioPortD)
#define DISP_EXTCOMIN_pin		(13)
#define PB0_port				(gpioPortF)
#define PB0_pin					(6)
#define PB1_port				(gpioPortF)
#define PB1_pin					(7)

#define GPIO_SET_DISPLAY_EXT_COMIN_IMPLEMENTED 1
#define TIMER_SUPPORTS_1HZ_TIMER_EVENT 1

//function prototypes
void gpioInit();
void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void gpioLed1SetOff();
void gpio_I2C_sensor_enable();
void gpio_I2C_sensor_disable();
void gpioI2CSensorEnSetOn(void);
void gpioSetDisplayExtcomin(bool state);

#endif /* SRC_GPIO_H_ */
