/*********************************************************************************************
 *  @file gpio.h
 *	@brief includes, defines and function prototypes for gpio.c
 *
 *  @author : Dan Walkes
 *  @date Created on: Dec 12, 2018
 *
 *  Updated by  Dave Sluiter Sept 7, 2020. moved #defines from .c to .h file.
 *				Rajat Chaple (GATT client code)
 *       		Sundar Krishnakumar (GATT server code)
 *
 * @date        April 29, 2020 (last update)
 **********************************************************************************************/
#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT
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
#define PROX_INTRPT_port		(gpioPortD)
#define PROX_INTRPT_pin			(10)

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
#else

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_
#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>
#include <stdbool.h>
#include "em_core.h"
#include "native_gecko.h"

// Student TODO: define these, 0's are placeholder values.
// See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
// and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
// to determine the correct values for these.

#define	LED0_port  gpioPortF // change to correct ports and pins
#define LED0_pin   4
#define LED1_port  gpioPortF
#define LED1_pin   5

#define SENSOR_EN_port  gpioPortD
#define SENSOR_EN_pin   15

#define DISP_EXTCOMIN_port  gpioPortD
#define DISP_EXTCOMIN_pin   13

#define I2C0_SCL_port  gpioPortC
#define I2C0_SDA_port  gpioPortC
#define I2C0_SCL_pin   10
#define I2C0_SDA_pin   11


#define PB0_port  gpioPortF
#define PB0_pin   6
#define PB1_port  gpioPortF
#define PB1_pin   7

#define IMU_EN_port  gpioPortD
#define IMU_EN_pin   12


#define GPIO_SET_DISPLAY_EXT_COMIN_IMPLEMENTED 	1

#define GPIO_DISPLAY_SUPPORT_IMPLEMENTED		1


extern uint8_t button_value;

void gpioInit();
void gpioI2CSensorEnSetOn();
void gpioI2CSensorEnSetOff();
void gpioSetDisplayExtcomin(bool);

void gpio_set_event_PB0_press();


void gpioIMUSensorEnSetOn();
void gpioIMUSensorEnSetOff();

#endif /* SRC_GPIO_H_ */

#endif
