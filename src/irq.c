/*********************************************************************************************
 *  @file irq.c
 *	@brief This file contains Interrupt handlers
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 4, 2020
 *
 *  Updated by Rajat Chaple Feb 12, 2020. Removed LED toggling from ISR
 *  									  Added - setting event on Timer0 Underflow
 *  Updated by Rajat Chaple Feb 20, 2020. Added - letimerMilliseconds since startup
 **********************************************************************************************/

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

#include "log.h"
#include "irq.h"


#define COMP1_MATCH_FLAG 0x00000002
#define COUNTER_UF_FLAG 0x00000004

static uint32_t time_since_startup = 0;
static uint32_t sys_ticks_ms = 0;
I2C_TransferReturn_TypeDef transfer_status;


/** -------------------------------------------------------------------------------------------
* Interrupt handler for LETIMER0
*-------------------------------------------------------------------------------------------- **/
void LETIMER0_IRQHandler()
{
	//Getting reason behind raised interrupt
	uint32_t reason = LETIMER_IntGetEnabled(LETIMER0);

	//Clearing an interrupt
	LETIMER_IntClear(LETIMER0,reason);

	if(reason & COUNTER_UF_FLAG) //Counter (CNT) underflow flag
	{
		CORE_DECLARE_IRQ_STATE;
		CORE_ENTER_CRITICAL();
		scheduler_set_event_UF();
		time_since_startup += LETIMER_PERIOD_MS;
		CORE_EXIT_CRITICAL();
	}
	if(reason & COMP1_MATCH_FLAG)
	{
		CORE_DECLARE_IRQ_STATE;
		CORE_ENTER_CRITICAL();
		scheduler_set_event_COMP1();
		CORE_EXIT_CRITICAL();
	}
} // LETIMER0_IRQHandler()

/** -------------------------------------------------------------------------------------------
* Interrupt handler for I2C0
*-------------------------------------------------------------------------------------------- **/
void I2C0_IRQHandler()
{

	transfer_status = I2C_Transfer(I2C0);
//	LOG_INFO("T S %d", transfer_status);
//	int i = 10000;while(i--);
	if(transfer_status == i2cTransferDone)
	{
		CORE_DECLARE_IRQ_STATE;
		CORE_ENTER_CRITICAL();
		scheduler_set_event_I2C_transfer_complete();
		CORE_EXIT_CRITICAL();
	}
	if(transfer_status == i2cTransferNack)
	{
		CORE_DECLARE_IRQ_STATE;
		CORE_ENTER_CRITICAL();
		scheduler_set_event_I2C_transfer_retry();
		CORE_EXIT_CRITICAL();
	}

} // I2C0_IRQHandler()

/** -------------------------------------------------------------------------------------------
* Interrupt handler for Even GPIOs
*-------------------------------------------------------------------------------------------- **/
void GPIO_EVEN_IRQHandler()
{
	//Getting reason behind raised interrupt
	uint32_t reason = GPIO_IntGetEnabled();

	//Clearing an interrupt
	GPIO_IntClear(reason);

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	if(GPIO_PinInGet(PB0_port, PB0_pin) == false)
		scheduler_set_event_PB0_switch_high_to_low();
	else if(GPIO_PinInGet(PB0_port, PB0_pin) == true)
		scheduler_set_event_PB0_switch_low_to_high();

	CORE_EXIT_CRITICAL();


} // GPIO_EVEN_IRQHandler()

/** -------------------------------------------------------------------------------------------
* Interrupt handler for Odd GPIOs
*-------------------------------------------------------------------------------------------- **/
void GPIO_ODD_IRQHandler()
{
	//Getting reason behind raised interrupt
	uint32_t reason = GPIO_IntGetEnabled();

	//Clearing an interrupt
	GPIO_IntClear(reason);

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	scheduler_set_event_PB1_switch_low_to_high();	//set PB1 button pressed event
	CORE_EXIT_CRITICAL();


} // GPIO_ODD_IRQHandler()


/** -------------------------------------------------------------------------------------------
 * @brief returns milliseconds expired since startup
 *
 * @param : None
 * @return : None
 *-------------------------------------------------------------------------------------------- **/
uint32_t letimerMilliseconds()
{
	uint32_t counter_in_ms = ((LETIMER_CounterGet(LETIMER0) * MILLISECONDS_IN_SECOND)/ACTUAL_CLK_FREQ);
	uint32_t time_since_timer0_underflow = (LETIMER_PERIOD_MS - counter_in_ms);
	return time_since_startup + time_since_timer0_underflow;
}

/** -------------------------------------------------------------------------------------------
 * Interrupt handler for sys ticks from CMSIS
 *-------------------------------------------------------------------------------------------- **/

void SysTick_Handler ()
{
	sys_ticks_ms++;                                     // increment ticks
}

/** -------------------------------------------------------------------------------------------
 * @brief returns milliseconds expired since startup
 *
 * @param : None
 * @return : None
 *-------------------------------------------------------------------------------------------- **/
uint32_t getSysTicks()
{
	return sys_ticks_ms;
}
#else
#endif
