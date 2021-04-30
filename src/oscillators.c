/*********************************************************************************************
 *  @file oscillators.c
 *	@brief This file contains definitions for clock configuration
 *
 *  @authors : Rajat Chaple (GATT client code)
 *  		   Sundar Krishnakumar (GATT server code)
 *
 *  @date      April 29, 2020 (last update)
 **********************************************************************************************/

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT
#include "oscillators.h"
//#define DEBUG

/** -------------------------------------------------------------------------------------------
 * @brief function to configure a clock
 *
 * @param : None
 * @return : None
 *-------------------------------------------------------------------------------------------- **/
void configure_clock()
{
//configuring clock as per ENergy Modes requirements
#if (LOWEST_ENERGY_MODE>=EM0) && (LOWEST_ENERGY_MODE<=EM2)
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
#elif (LOWEST_ENERGY_MODE == EM3)
	CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
#endif


#ifdef DEBUG
	CMU_Select_TypeDef ret_clock = CMU_ClockSelectGet(cmuClock_LFA);
#endif

	//Setting up prescalar for a clock
	CMU_ClockDivSet(cmuClock_LETIMER0, PRESCALER_VALUE);
#ifdef DEBUG
	CMU_ClkDiv_TypeDef ret_div  = CMU_ClockDivGet(cmuClock_LETIMER0);
#endif

#ifdef DEBUG
	//checking frequency for configured clock
	uint32_t ret_freq = CMU_ClockFreqGet(cmuClock_LFA);
	ret_freq = CMU_ClockFreqGet(cmuClock_LETIMER0);
#endif



}// configure_clock()

#else

#include "oscillators.h"


void init_LFXO_LETIMER0(CMU_ClkDiv_TypeDef div)
{

	// Enable the LFXO oscillator
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

	// Set the clock branch
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	/**
	 * Set the prescaler
	 * LFXO OSC - 32768Hz
	 * Prescaler - 4
	 * LETIMER0 CLK SRC = 32768 / 4 = 8192Hz
	 * This is the optimal setting to produce 7s of blink period using 16 bit down counter of LETIMER0
	 * Compare registers are also 16bit size.
	 */
	CMU_ClockDivSet(cmuClock_LETIMER0 , div);

	// Enable the clock - LFACLK for the peripheral - LETIMER0
	CMU_ClockEnable(cmuClock_LETIMER0, 1);



}

void init_ULFRCO_LETIMER0(CMU_ClkDiv_TypeDef div)
{

	// Enable the ULFRCO oscillator
	CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);

	// Set the clock branch
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);

	/**
	 * Set the prescaler
	 * ULFRCO OSC - 1000Hz
	 * Prescaler - 1 (no prescaling needed)
	 * LETIMER0 CLK SRC = 1000 / 1 = 1000Hz
	 * This is the optimal setting to produce 7s of blink period using 16 bit down counter of LETIMER0
	 * Compare registers are also 16bit size.
	 */
	CMU_ClockDivSet(cmuClock_LETIMER0 , div);

	// Enable the clock - LFACLK for the peripheral - LETIMER0
	CMU_ClockEnable(cmuClock_LETIMER0, 1);

}


#endif
