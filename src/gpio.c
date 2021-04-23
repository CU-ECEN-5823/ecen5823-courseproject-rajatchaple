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
	GPIO_PinModeSet(PROX_INTRPT_port, PROX_INTRPT_pin, gpioModeInputPullFilter, true);	//DOUT : true means pull up
	GPIO_ExtIntConfig (PROX_INTRPT_port, PROX_INTRPT_pin, PROX_INTRPT_pin, false, true, true);	//enable at falling edge



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

// Students see file: gpio.h for instructions
#include "gpio.h"

// Default state=1
// Reason: The pin is pulled high
// PB0 pressed=0
// PB0 released=1
uint8_t button_value = 1;


void gpioInit()
{
	// GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);

	// configuration for I2C sensor enable pin
	GPIO_DriveStrengthSet(SENSOR_EN_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(SENSOR_EN_port, SENSOR_EN_pin, gpioModePushPull, false);

	// Configuration for LCD charge build up prevention pin
	GPIO_DriveStrengthSet(DISP_EXTCOMIN_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(DISP_EXTCOMIN_port, DISP_EXTCOMIN_pin, gpioModePushPull, false);

	// Push button PB0 configuration

	// true =  pulled high.
	GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInputPullFilter, true);

	// Enable interrupt for GPIO.
	NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);

	// Configure PB0 interrupt on falling edge, i.e. when it is
	// pressed, and rising edge, i.e. when it is released.
	// Last argument = true. The pin is pulled high.
	GPIO_ExtIntConfig(PB0_port, PB0_pin, PB0_pin, true, true, true);

	// Push button PB1 configuration

	// true =  pulled high.
	GPIO_PinModeSet(PB1_port, PB1_pin, gpioModeInputPullFilter, true);

	// Enable interrupt for GPIO.
	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
	NVIC_EnableIRQ(GPIO_ODD_IRQn);

	// Configure PB1 interrupt on falling edge, i.e. when it is pressed.
	// Last argument = true. The pin is pulled high.
	GPIO_ExtIntConfig(PB1_port, PB1_pin, PB1_pin, false, true, true);


	// Configuration for IMU sensor enable pin
	GPIO_DriveStrengthSet(IMU_EN_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_PinModeSet(IMU_EN_port, IMU_EN_pin, gpioModePushPull, true);

}

void gpioLed0SetOn()
{
	GPIO_PinOutSet(LED0_port,LED0_pin);
}

void gpioLed0SetOff()
{
	GPIO_PinOutClear(LED0_port,LED0_pin);
}

void gpioLed1SetOn()
{
	GPIO_PinOutSet(LED1_port,LED1_pin);
}

void gpioLed1SetOff()
{
	GPIO_PinOutClear(LED1_port,LED1_pin);
}
void gpioI2CSensorEnSetOn()
{
	GPIO_PinOutSet(SENSOR_EN_port, SENSOR_EN_pin);
}
void gpioI2CSensorEnSetOff()
{
	GPIO_PinOutClear(SENSOR_EN_port, SENSOR_EN_pin);
}

// IMU sensor on/off control function calls
void gpioIMUSensorEnSetOn()
{
	GPIO_PinOutSet(IMU_EN_port, IMU_EN_pin);
}
void gpioIMUSensorEnSetOff()
{
	GPIO_PinOutClear(IMU_EN_port, IMU_EN_pin);
}

// Toggles the EXTCOMIN pin
void gpioSetDisplayExtcomin(bool value)
{
	if (value == 0)
	{
		GPIO_PinOutSet(DISP_EXTCOMIN_port, DISP_EXTCOMIN_pin);
	}
	else
	{
		GPIO_PinOutClear(DISP_EXTCOMIN_port, DISP_EXTCOMIN_pin);

	}
}


// Configured PB0 interrupt on falling edge, i.e. when it is
// pressed, and rising edge, i.e. when it is released.
// Last argument = true. The pin is pulled high.
// gpio_set_event functions are not part of the scheduler events.
// Hence defined here.
void gpio_set_event_PB0_press()
{

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section
	button_value = 0;
	gecko_external_signal(200);

	CORE_EXIT_CRITICAL();
}


void gpio_set_event_PB0_release()
{

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section
	button_value = 1;
	gecko_external_signal(210);

	CORE_EXIT_CRITICAL();

}

// Configured PB1 interrupt on falling edge, i.e. when it is pressed
// Last argument = true. The pin is pulled high.
void gpio_set_event_PB1_press()
{
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// critical section
	gecko_external_signal(300);

	CORE_EXIT_CRITICAL();

}

#endif
