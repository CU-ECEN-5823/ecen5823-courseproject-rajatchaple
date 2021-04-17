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

#include "irq.h"


transfer_states_t state = 0;


void LETIMER0_IRQHandler(void)
{

	// Determine interrupt source
	if (LETIMER_IntGetEnabled(LETIMER0) == LETIMER_IF_UF)
	{
		// Clear the interrupt flags that were set
		LETIMER_IntClear(LETIMER0, LETIMER_IFC_UF);

		// Interrupt handler logic here

		#if INCLUDE_LOGGING
			CORE_DECLARE_IRQ_STATE;
			CORE_ENTER_CRITICAL();

			// Increment millis_cnt here. Timestamp is based on UF interrupt (3000ms) + current counter value.
			// Calculation taken from lecture slides.
			millis_cnt = millis_cnt + 3000 + (3000 - LETIMER_CounterGet(LETIMER0));

			CORE_EXIT_CRITICAL();
		#endif


		// Start the I2C read cycle.
		#if (DEVICE_IS_BLE_SERVER == 1)

			scheduler_set_event_UF();
			state++;


		#endif


	}

	// Determine interrupt source
	if (LETIMER_IntGetEnabled(LETIMER0) == LETIMER_IF_COMP1)
	{
		// Clear the interrupt flags that were set
		LETIMER_IntClear(LETIMER0, LETIMER_IFC_COMP1);

		// After wake-up disable interrupt.
		LETIMER_IntDisable(LETIMER0, LETIMER_IF_COMP1);


		if (state == STATE_WAIT_FOR_5_MILLIS)
		{
			state++;
			state++;
			scheduler_set_event_STATE_WAIT_FOR_5_MILLIS();
			return;
		}


		if (state == STATE_WAIT_FOR_65_MILLIS)
		{
			state++;
			state++;
			scheduler_set_event_STATE_WAIT_FOR_65_MILLIS();
			return;
		}


		if (state == STATE_SEND_IMU_INDICATION)
		{
			state = 0; // This is the last state. Reset to 0 here.
			scheduler_set_event_STATE_SEND_IMU_INDICATION();
			return;
		}


	}

}


// Configured PB0 interrupt on falling edge, i.e. when it is
// pressed, and rising edge, i.e. when it is released.
// Last argument = true. The pin is pulled high.
void GPIO_EVEN_IRQHandler(void)
{

	if (GPIO_IntGetEnabled()== (1 << PB0_pin))
	{

		GPIO_IntClear(1 << PB0_pin);

		// Do something here.
		uint8_t inp_state = GPIO_PinInGet(PB0_port, PB0_pin);

		// When pressed
		if (inp_state == 0)
		{

			gpio_set_event_PB0_press();

		}

		// When released
		if (inp_state == 1)
		{

			gpio_set_event_PB0_release();

		}

	}

}



// Configured PB1 interrupt on falling edge, i.e. when it is pressed
// Last argument = true. The pin is pulled high.
void GPIO_ODD_IRQHandler(void)
{

	if (GPIO_IntGetEnabled()== (1 << PB1_pin))
	{

		GPIO_IntClear(1 << PB1_pin);

		// Do something here.
		uint8_t inp_state = GPIO_PinInGet(PB1_port, PB1_pin);

		// When pressed
		if (inp_state == 0)
		{
			gpio_set_event_PB1_press();

		}

	}

}


void I2C0_IRQHandler(void)
{

	// Use I2C_Transfer() inside ISR
	if (I2C_IntGetEnabled(I2C0) == I2C_IF_START)
	{
		I2C_TransferReturn_TypeDef ret;
		ret = I2C_Transfer(I2C0);
		if (ret < 0)
		{

		}

	}


	if (I2C_IntGetEnabled(I2C0) == I2C_IF_ACK)
	{
		I2C_TransferReturn_TypeDef ret;
		ret = I2C_Transfer(I2C0);
		if (ret < 0)
		{

		}

	}




	if (I2C_IntGetEnabled(I2C0) == I2C_IF_NACK)
	{
		I2C_TransferReturn_TypeDef ret;
		ret = I2C_Transfer(I2C0);

		// NACK on FXAS reset.
		if (state == STATE_GYRO_RESET_SIGNAL_NACK)
		{


			if (ret < 0)
			{

			}
			else if (ret == 1)
			{
				state++;
				state++;
				state++;
				// I2C0->CMD = I2C_CMD_STOP; // send stop signal. This brings gyro sensor to known state.
				scheduler_set_event_STATE_GYRO_RESET_SIGNAL_NACK();
				return;

			}

		}

	}


	if (I2C_IntGetEnabled(I2C0) == I2C_IF_RXDATAV)
	{
		I2C_TransferReturn_TypeDef ret;
		ret = I2C_Transfer(I2C0);

		if (ret < 0)
		{

		}

	}


	if (I2C_IntGetEnabled(I2C0) == I2C_IF_MSTOP)
	{
		I2C_TransferReturn_TypeDef ret;
		ret = I2C_Transfer(I2C0);

		if (state == STATE_ACC_STANDBY_SIGNAL_STOP)
		{

			if (ret < 0)
			{

			}
			else if (ret == 0)
			{
				state++;
				state++;
				scheduler_set_event_STATE_ACC_STANDBY_SIGNAL_STOP();
				return;

			}


		}

		if (state == STATE_ACC_DATA_CFG_STOP)
		{

			if (ret < 0)
			{

			}
			else if (ret == 0)
			{
				state++;
				state++;
				scheduler_set_event_STATE_ACC_DATA_CFG_STOP();
				return;

			}

		}


		if (state == STATE_ACC_CTRL_REG1_STOP)
		{

			if (ret < 0)
			{

			}
			else if (ret == 0)
			{
				state++;
				state++;
				scheduler_set_event_STATE_ACC_CTRL_REG1_STOP();
				return;

			}

		}

		if (state == STATE_GYRO_STANDBY_SIGNAL_STOP)
		{

			if (ret < 0)
			{

			}
			else if (ret == 0)
			{
				state++;
				state++;
				scheduler_set_event_STATE_GYRO_STANDBY_SIGNAL_STOP();
				return;

			}

		}



		if (state == STATE_GYRO_CTRL_REG0_STOP)
		{

			if (ret < 0)
			{

			}
			else if (ret == 0)
			{
				state++;
				state++;
				scheduler_set_event_STATE_GYRO_CTRL_REG0_STOP();
				return;

			}

		}


		if (state == STATE_GYRO_CTRL_REG1_STOP)
		{

			if (ret < 0)
			{

			}
			else if (ret == 0)
			{
				state++;
				scheduler_set_event_STATE_GYRO_CTRL_REG1_STOP();
				return;

			}

		}


		if (state == STATE_ACC_MEASURE_STOP)
			{

				if (ret < 0)
				{

				}
				else if (ret == 0)
				{
					state++;
					state++;
					scheduler_set_event_STATE_ACC_MEASURE_STOP();
					return;

				}

			}


		if (state == STATE_GYRO_MEASURE_STOP)
			{

				if (ret < 0)
				{

				}
				else if (ret == 0)
				{
					state++;
					scheduler_set_event_STATE_GYRO_MEASURE_STOP();
					return;

				}

			}

	}
}


#endif
