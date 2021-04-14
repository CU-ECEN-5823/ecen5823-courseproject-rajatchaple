/*********************************************************************************************
 *  @file main.c
 *	@brief a main file with initializations, setup and actions
 *
 *  @author : Dave Sluiter
 *  @date Created on: Jan 26, 2021
 *
 *  Updated by Rajat Chaple Jan 29, 2020. Added GPIO Initialization and LED blinking.
 *  Updated by Rajat Chaple Feb  4, 2020. 	Removed delayApproxOneSecond().
 *  										Added clock,timer sleep and interrupt configuration.
 *	Updated by Rajat Chaple Feb 12, 2020.	Added I2C configuration and Temperature read over
 *											I2C in blocking mode
 *	Updated by Rajat Chaple Feb 20, 2020.	Changed I2C configuration and Temperature read over
 *											I2C to nonblocking mode
 *	Updated by Rajat Chaple Feb 26, 2020.	Added sending temperature over BLE
 *	Updated by Rajat Chaple Mar 13, 2020.	Client functionality added
 **********************************************************************************************/


//#define INCLUDE_LOG_DEBUG 1
#include "log.h"
#include "main.h"


#if BUILD_INCLUDES_BLE_SERVER
extern I2C_TransferReturn_TypeDef transfer_status;
extern ble_status_t ble_status;
#endif
/** -------------------------------------------------------------------------------------------
 * @brief entry function for sequential execution which contains initializations and configurations
 * 		  of different peripherals
 *
 * @param gecko_configuration_t Gecko configuration
 * @return int : returns 0 on success, 1 on failure
 *-------------------------------------------------------------------------------------------- **/
int appMain(gecko_configuration_t *config)
{
	struct gecko_cmd_packet* event;

	//Initialize logging
	logInit();

	//Initializing the bluetooth stack
	gecko_init(config);

	//clock configuration for LETIMER0
	configure_clock();
	CMU_ClockEnable(cmuClock_LETIMER0, true);

	//GPIO initialization for LED0
	gpioInit();

	//i2c0 configuration for clock and pins
	i2c_init();

	//Sleep configuration
	SLEEP_Init_t sleepConfig = {0};
	SLEEP_InitEx(&sleepConfig);

#if (LOWEST_ENERGY_MODE == EM1) || (LOWEST_ENERGY_MODE == EM2)
	const SLEEP_EnergyMode_t sleep_mode_blocked = LOWEST_ENERGY_MODE + 1; //same as sleepEMn
	SLEEP_SleepBlockBegin(sleep_mode_blocked); // Enter into lowest energy mode
#endif

	//LETIMER0 initialization
	init_LETIMER0();
	LETIMER_Enable(LETIMER0, true);

	//initializing display on Gecko board
	displayInit();	//same enable line also powers up I2C0 hence commented I2C0enable below
	//I2C0_enable(true);	//Power on  and connect Si7021



	//Interrupts configuration for LETIMER0 (IF register)
	NVIC_EnableIRQ(LETIMER0_IRQn);


#if DEVICE_IS_BLE_SERVER
	//Displaying text "Server" on LCD
	displayPrintf(DISPLAY_ROW_NAME, "Server");
#else
	//Displaying text "Server" on LCD
	displayPrintf(DISPLAY_ROW_NAME, "Client");
#endif


	while(1){
		if(!gecko_event_pending())
		{
			logFlush();
		}

		//Waiting for Bluetooth event. Allowed to enter sleep mode EM2
		event = gecko_wait_event();

#if DEVICE_IS_BLE_SERVER
		//Perform actions based on event occurred
		handle_ble_event_server(event);

		//if Bluetooth is disconnected, disable temperature measurement over I2C
		if(BGLIB_MSG_ID(event->header) == gecko_evt_le_connection_closed_id)
		{
			NVIC_DisableIRQ(I2C0_IRQn); //halting in progress transfers(if any)
			LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1 | LETIMER_IEN_UF); //Disabling periodic interrupts used for temp measurement
			//Forcing state machine to exit as Bluetooth is disconnected. This also ensures it to be ready to resume at connection
			event->data.evt_system_external_signal.extsignals = STATE4_WAIT_FOR_I2C_READ_COMPLETE;
			state_machine_measure_temperature(event);
		}

		//if Bluetooth is connected, enable temperature measurement over I2C
		if(BGLIB_MSG_ID(event->header) == gecko_evt_le_connection_opened_id)
		{
			LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF); //Enabling periodic interrupts used for temp measurement
			//chances are first UF interrupt  < specified time. Because, counter value can be somewhere in-between. LETIMER0_INIT would be required to handle this
		}


		//Performing Temperature measurement only when client is connected over Bluetooth
		if((ble_status.connection_status != DISCONNECTED) && (BGLIB_MSG_ID(event->header) == gecko_evt_system_external_signal_id)) //process state machine only in case of external event id
		{
			state_machine_measure_temperature(event);
		}

		//Disabling I2C interrupts. re-enalbed in i2c_write() or i2c_read()
		if(transfer_status == i2cTransferDone)
		{
			NVIC_DisableIRQ(I2C0_IRQn);
		}

#else
		//Perform actions based on event occurred
		handle_ble_event_client(event);
#endif


		}// while(1)

#if (LOWEST_ENERGY_MODE == EM1) || (LOWEST_ENERGY_MODE == EM2)
	SLEEP_SleepBlockEnd(sleep_mode_blocked);
#endif


} // appMain()
