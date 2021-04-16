/*********************************************************************************************
 *  @file gpio.c
 *	@brief This file contains definitions for gpio initialization and configuration
 *
 *  @author : Dan Walkes
 *  @date Created on: Dec 12, 2018
 *
 *  Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.
 *  Updated by Rajat Chaple Jan 29, 2021. Changed GPIO Drive strength and added comments
 *  Updated by Rajat Chaple Feb 12, 2020. I2C sensor enable gpio config added
 **********************************************************************************************/
#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT
//#define INCLUDE_LOG_DEBUG 1
#include "log.h"
#include "gpio.h"

//#define DEBUG
/** -------------------------------------------------------------------------------------------
 * @brief This function sets up GPIO Modes and Drive properties
 *
 * @param None
 * @return None
 *-------------------------------------------------------------------------------------------- **/
void gpioInit()
{
	//GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);

	//GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);

	//configuring Sensor_enable pin
	GPIO_DriveStrengthSet(I2C_SENSOR_ENABLE_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(I2C_SENSOR_ENABLE_port, I2C_SENSOR_ENABLE_pin, gpioModePushPull, false);

	//configuring LCD_EXTCOMIN pin
	GPIO_DriveStrengthSet(DISP_EXTCOMIN_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(DISP_EXTCOMIN_port, DISP_EXTCOMIN_pin, gpioModePushPull, false);

	GPIO_PinModeSet(PB1_port, PB1_pin, gpioModeInputPullFilter, true);	//DOUT : true means pull-up
	GPIO_ExtIntConfig (PB1_port, PB1_pin, PB1_pin, false, true, true);	//enabled at falling edge
	NVIC_EnableIRQ(GPIO_ODD_IRQn);

	GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInputPullFilter, true);	//DOUT : true means pull-up
	GPIO_ExtIntConfig (PB0_port, PB0_pin, PB0_pin, true, true, true);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);

}// gpioInit()


/** -------------------------------------------------------------------------------------------
* This function Turns LED0 ON
*-------------------------------------------------------------------------------------------- **/
void gpioLed0SetOn()
{
	GPIO_PinOutSet(LED0_port,LED0_pin);
}// gpioLed0SetOn()




/** -------------------------------------------------------------------------------------------
* This function Turns LED0 OFF
*-------------------------------------------------------------------------------------------- **/
void gpioLed0SetOff()
{
	GPIO_PinOutClear(LED0_port,LED0_pin);
}// gpioLed0SetOff()


/** -------------------------------------------------------------------------------------------
* This function Turns LED1 ON
*-------------------------------------------------------------------------------------------- **/
void gpioLed1SetOn()
{
	GPIO_PinOutSet(LED1_port,LED1_pin);
}// gpioLed1SetOn()


/** -------------------------------------------------------------------------------------------
* This function Turns LED1 OFF
*-------------------------------------------------------------------------------------------- **/
void gpioLed1SetOff()
{
	GPIO_PinOutClear(LED1_port,LED1_pin);
}// gpioLed1SetOff()

/** -------------------------------------------------------------------------------------------
* This function powers I2C sensor and connects SCL, SDA lines
*-------------------------------------------------------------------------------------------- **/
void gpio_I2C_sensor_enable()
{
	GPIO_PinOutSet(I2C_SENSOR_ENABLE_port,I2C_SENSOR_ENABLE_pin);
}// I2C_sensor_enable()

/** -------------------------------------------------------------------------------------------
* This function disconnects power, SCL and SDA lines from I2C sensor
*-------------------------------------------------------------------------------------------- **/
void gpio_I2C_sensor_disable()
{
	GPIO_PinOutClear(I2C_SENSOR_ENABLE_port,I2C_SENSOR_ENABLE_pin);
}// I2C_sensor_disable()

/** -------------------------------------------------------------------------------------------
* This function powers I2C sensor and LCD
*-------------------------------------------------------------------------------------------- **/
void gpioI2CSensorEnSetOn()
{
	GPIO_PinOutSet(I2C_SENSOR_ENABLE_port,I2C_SENSOR_ENABLE_pin); // DISP_ENABLE set
}// gpioI2CSensorEnSetOn()

/** -------------------------------------------------------------------------------------------
* This function disconnects power, SCL and SDA lines from I2C sensor
*-------------------------------------------------------------------------------------------- **/
void gpioSetDisplayExtcomin(bool value)
{
	if(value)
		GPIO_PinOutSet(DISP_EXTCOMIN_port,DISP_EXTCOMIN_pin);
	else
		GPIO_PinOutClear(DISP_EXTCOMIN_port,DISP_EXTCOMIN_pin);

}// I2C_sensor_disable()

#else
#endif
