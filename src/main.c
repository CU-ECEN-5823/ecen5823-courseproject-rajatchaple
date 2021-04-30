/*********************************************************************************************
 *  @file main.c
 *	@brief a main file with initializations, setup and actions
 *
 *  @author : Dave Sluiter
 *            Rajat Chaple (GATT client code)
 *  		  Sundar Krishnakumar (GATT server code)
 *
 *  @date      April 29, 2020 (last update)
 **********************************************************************************************/

#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT
//#define INCLUDE_LOG_DEBUG 1
#include "log.h"
#include "main.h"
#include "proximity.h"

extern I2C_TransferReturn_TypeDef transfer_status;

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
	proximity_sensor_config();
//	test_proximity_sensor();	//Uncomment to Test proximity sensors functionality

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

	//Interrupts configuration for LETIMER0 (IF register)
	LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);
	NVIC_EnableIRQ(LETIMER0_IRQn);




	timerWaitUs(PROXIMITY_SENSOR_READ_INTERVAL);

	NVIC_EnableIRQ(GPIO_EVEN_IRQn);


	//enabling interrupt using interrupt control register
	blocking_write_i2c(0x89, 0b00000010);

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
			event = gecko_wait_event();

			handle_ble_event_client(event);

			if(BGLIB_MSG_ID(event->header) == gecko_evt_system_external_signal_id)
				event_handler_proximity_state(event);

		if((transfer_status == i2cTransferDone) && (BGLIB_MSG_ID(event->header) == gecko_evt_system_external_signal_id))
		{
			NVIC_DisableIRQ(I2C0_IRQn);
		}
	}



#if (LOWEST_ENERGY_MODE == EM1) || (LOWEST_ENERGY_MODE == EM2)
	SLEEP_SleepBlockEnd(sleep_mode_blocked);
#endif


} // appMain()

#else

#include "main.h"

// Sleep setting - 1
SLEEP_EnergyMode_t sleep_mode_blocked = SLEEP_MODE_BLOCKED;
runqueue evt;
struct gecko_cmd_packet *bl_evt;


int appMain(gecko_configuration_t *config)
{

	uint16_t v1 = 0; // cmp0

	uint32_t flags = 0;


	// Initialize stack
	gecko_init(config);

	// Make Tx power zero here
	gecko_cmd_system_set_tx_power(0); //  TX Power should be set to 0db

	#if INCLUDE_LOGGING
		// Init logging here
		logInit();
	#endif


	// Init GPIO configurations here
	gpioInit();

	// Init LCD display here
	displayInit();

	// Only if running as server init I2C.
	#if (DEVICE_IS_BLE_SERVER == 1)

		// Init IRC0 here
		I2C0_init();

	#endif

	// Init LETIMER0 clock tree here
	init_ULFRCO_LETIMER0(cmuClkDiv_1);
	value_to_load(cmuOsc_ULFRCO, cmuClkDiv_1, LETIMER_PERIOD_MS, &v1); // 3000ms



	// Config LETIMER0 here
	LETIMER_Init_TypeDef letimer_init_custom =
	{
		.enable         = false, // Do not enable LETIMER0 here
		.debugRun       = false,
		.comp0Top       = true,
		.bufTop         = false,
		.out0Pol        = 0,
		.out1Pol        = 0,
		.ufoa0          = letimerUFOANone,
		.ufoa1          = letimerUFOANone,
		.repMode        = letimerRepeatFree,
		.topValue		= 0
	};


	config_LETIMER0(&letimer_init_custom, v1, 0); // CMP1 not used. So pass zero.



	// Enable LETIMER0 interrupts here
	flags = LETIMER_IF_UF;
	config_INT_LETIMER0(flags);


	if (ENABLE_SLEEPING == 1)
	{

		// Prepare for sleeping
		SLEEP_Init_t sleepConfig = {0};
		SLEEP_InitEx(&sleepConfig);

		// Sleep setting - 2
		// First call so the End() call not required.
		// Need not call if EM3 mode needed.
		SLEEP_SleepBlockBegin(sleep_mode_blocked);


	}


	// Start LETIMER0 here for both server and client.
	// Timestamp needed for both.
	LETIMER_Enable(LETIMER0, true);


	/* Infinite loop */
	while (1)
	{

		if(!gecko_event_pending())
		{
			logFlush();
		}

		bl_evt = gecko_wait_event();


		handle_ble_event(bl_evt);

		// The event is pushed into the runqueue (state machine data structure) in the
		// handle_ble_event function's switch statement.
		// Retrieved using get_event() in FIFO order.
		// When the event is cleared by gecko_wait_event() then the same event is cleared
		// in the run-queue as well and sent to state machine for processing.
		evt = get_event();
		state_machine(evt);




	} // while(1)

	if (ENABLE_SLEEPING == 1)
	{
		SLEEP_SleepBlockEnd(sleep_mode_blocked);

	}

} // appMain()


#endif
