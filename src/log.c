/*********************************************************************************************
 *  @file log.c
 *	@brief logging purpose
 *
 *  @author : Dan Walkes
 *  @date Created on: Dec 18, 2018
 *
*  Updated by Dave Sluiter Jan 5, 2021.	    updates to loggerGetTimestamp()
 **********************************************************************************************/

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

#include "retargetserial.h"
#include "log.h"
#include <stdbool.h>

//#define MY_USE_SYSTICKS


#if INCLUDE_LOGGING
/**
 * @return a timestamp value for the logger, typically based on a free running timer.
 * This will be printed at the beginning of each log message.
 */
uint32_t loggerGetTimestamp(void)
{
    #ifdef MY_USE_SYSTICKS
    
       // Students: Look in the CMSIS library for systick routines. For debugging
       //           purposes this can provide greater resolution than a timestamp based on
       //           LETIMER0. Do not turn in any code that executes systick routines
       //           as this may effect your energy measurements and subsequently your grade.
       
       // Develop this function if you so desire for debugging purposes only
	   return getSysTicks();
	   
    #else
    
       return letimerMilliseconds();
	   //return (0);
	   
    #endif
}


/**
 * Initialize logging for Blue Gecko.
 * See https://www.silabs.com/community/wireless/bluetooth/forum.topic.html/how_to_do_uart_loggi-ByI
 */
void logInit(void)
{
	RETARGET_SerialInit();
	/**
	 * See https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__RetargetIo.html#ga9e36c68713259dd181ef349430ba0096
	 * RETARGET_SerialCrLf() ensures each linefeed also includes carriage return.  Without it, the first character is shifted in TeraTerm
	 */
	RETARGET_SerialCrLf(true);

#ifdef MY_USE_SYSTICKS
	SysTick_Config(SystemCoreClock / 1000);      /* Configure SysTick to generate an interrupt every millisecond */
#endif
	LOG_INFO("Initialized Logging");
}

/**
 * Block for chars to be flushed out of the serial port.  Important to do this before entering SLEEP() or you may see garbage chars output.
 */
void logFlush(void)
{
	RETARGET_SerialFlush();
}
#endif

#else
#endif
