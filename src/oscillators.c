/*********************************************************************************************
 *  @file oscillators.c
 *	@brief This file contains definitions for clock configuration
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 4, 2021
 **********************************************************************************************/

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
