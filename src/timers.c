/*********************************************************************************************
 *  @file timers.c
 *	@brief This file contains function definitions to handle timers initialization
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 4, 2021
 *
 *  Updated by Rajat Chaple Feb 20, 2020. changed timerWaitUs from olling based to Interrupt based
 **********************************************************************************************/
#include "log.h"
#include "timers.h"

//#define DEBUG

/** -------------------------------------------------------------------------------------------
 * @brief function to configure LETIMER0
 *
 * @param : None
 * @return : None
 *-------------------------------------------------------------------------------------------- **/
void init_LETIMER0()
{
	//LETIMER structure initialization
	LETIMER_Init_TypeDef init=
	{
	    false,             /* Disable timer when initialization completes. */
	    false,			   /* Counter shall keep running during debug halt. */
	    true,              /* Load COMP0 into CNT on underflow. */
	    false,             /* Do not load COMP1 into COMP0 when REP0 reaches 0. */
	    0,                 /* Idle value 0 for output 0. */
	    0,                 /* Idle value 0 for output 1. */
	    letimerUFOANone,   /* No action on underflow on output 0. */
	    letimerUFOANone,   /* No action on underflow on output 1. */
	    letimerRepeatFree, /* Count until stopped by SW. */
		0                  /* Use default top Value. */
	  };

	LETIMER_Init(LETIMER0, &init);

	//Setting up Counter, COMP and REP registers
	LETIMER_CounterSet(LETIMER0, VALUE_TO_LOAD_COUNTER);
	//LETIMER_CompareSet(LETIMER0, 1, VALUE_TO_LOAD_COMP1);
	LETIMER_CompareSet(LETIMER0, 0, VALUE_TO_LOAD_COUNTER);
	LETIMER_RepeatSet(LETIMER0, 0, 0);
	LETIMER_RepeatSet(LETIMER0, 1, 0);

	#ifdef DEBUG
	uint32_t ret_counter = LETIMER_CounterGet(LETIMER0);
	uint32_t ret_comp1 = LETIMER_CompareGet(LETIMER0, 1);
	uint32_t ret_comp0 = LETIMER_CompareGet(LETIMER0, 0);
	uint32_t ret_rep0 = LETIMER_RepeatGet(LETIMER0, 0);
	uint32_t ret_rep1 = LETIMER_RepeatGet(LETIMER0, 1);
#endif

}// init_LETIMER0()

/** -------------------------------------------------------------------------------------------
 * @brief wait for given microseconds time
 *
 * @param : None
 * @return : None
 *-------------------------------------------------------------------------------------------- **/
void timerWaitUs(uint32_t us_wait)
{
	uint32_t delay_in_counts = 0;
	uint32_t initial_counter_value = 0;
	uint32_t expected_counter_after_delay = 0;

	//Range check for input value
	if((us_wait/1000) > MAX_WAIT_TIME_ALLOWED_MS)	//checking if us_wait in milliseconds is lesser than allowed limit
	{
		//seting delay to default value which is 80000 i.e. 80 msec
		us_wait = 80000; //This default value is to be decided based on application
		LOG_WARN("Requested delay is more than allowed limit. Program may misbehave. Default delay of %d us set", us_wait);
	}

	delay_in_counts = (us_wait*ACTUAL_CLK_FREQ)/1000000;
	initial_counter_value = LETIMER_CounterGet(LETIMER0);

	//Handling rollover using VALUE_TO_LOAD_COUNTER
	expected_counter_after_delay = (VALUE_TO_LOAD_COUNTER + initial_counter_value - delay_in_counts) % VALUE_TO_LOAD_COUNTER;
	LETIMER_CompareSet(LETIMER0, 1, expected_counter_after_delay);

	LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);

}
