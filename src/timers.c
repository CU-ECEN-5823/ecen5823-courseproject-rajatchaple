/*********************************************************************************************
 *  @file timers.c
 *	@brief This file contains function definitions to handle timers initialization
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 4, 2021
 *
 *  Updated by Rajat Chaple Feb 20, 2020. changed timerWaitUs from olling based to Interrupt based
 **********************************************************************************************/

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT
#include "log.h"
#include "timers.h"
#include "proximity.h"

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
#else

#include "timers.h"



// Helper function to calculate COMPx AND CNT register values.
void value_to_load(CMU_Osc_TypeDef osc, CMU_ClkDiv_TypeDef div, uint32_t period, uint16_t *value)
{
	if (osc == cmuOsc_LFXO)
	{
		if (value != NULL)
		{
			*value = (period / 1000.0) * (32768 / div);

		}

	}
	else if (cmuOsc_ULFRCO)
	{
		if (value != NULL)
		{
			*value = (period / 1000.0) * (1000 / div);
		}

	}

}


void config_LETIMER0(LETIMER_Init_TypeDef *letimer_init, uint16_t cmp0_value, uint16_t cmp1_value)
{


	LETIMER_Init(LETIMER0, letimer_init);
	LETIMER_CompareSet(LETIMER0, 0, cmp0_value);
	LETIMER_CompareSet(LETIMER0, 1, cmp1_value);

}

void config_INT_LETIMER0(uint32_t interrupt_flags)
{

	LETIMER_IntEnable(LETIMER0, interrupt_flags);
	NVIC_EnableIRQ(LETIMER0_IRQn);

}

// Min=0uS
// Max=3000000uS
// Non polling interrupt based.
// Uses CMP1 interrupts.
inline void timerWaitUs(uint32_t us_wait)
{
	// Range checking
	// The parameter is unsigned integer.
	// So lower bound need not be checked.
	if (us_wait > 3000000)
	{
		us_wait = 3000000;
	}


	uint32_t chkpt = 0;

	// Convert microseconds to counter value;
	// LETIMER0 clock source settings previously done:
	// EM3 always
	// osc = cmuOsc_ULFRCO -> Freq = 1000Hz
	// prescaler div = 1
	uint32_t us_cnt = (us_wait / 1000000.0) * (1000.0 / 1);

	LETIMER_IntClear(LETIMER0, LETIMER_IFC_COMP1);


	// Get LETIMER0 CNT register value here
	uint32_t cnt_value = LETIMER_CounterGet(LETIMER0);


	// Counter roll-over managed here.
	if (cnt_value < us_cnt)
	{
		chkpt = abs(us_cnt - cnt_value); // e.g us_cnt = 3000 and cnt_value = 2000

		chkpt = LETIMER_CompareGet(LETIMER0, 0) - chkpt;
	}
	else if (cnt_value > us_cnt)
	{
		chkpt = cnt_value - us_cnt;
	}
	if (cnt_value < us_cnt)
	{
		chkpt = 0;
	}



	// Set COMP1 register value
	// Enable CMP1 interrupt.
	LETIMER_CompareSet(LETIMER0, 1, chkpt);
	// IMPORTANT step. Clear the COMP1 flag in IF register before enabling the COMP1 interrupt
	LETIMER_IntClear(LETIMER0, LETIMER_IFC_COMP1);
	LETIMER_IntEnable(LETIMER0, LETIMER_IF_COMP1);


}



#endif
