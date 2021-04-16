/*********************************************************************************************
 *  @file ble.c
 *	@brief This file contains event handler for ble events. THis handler sets advertising interval,
 *			connection interval, slave latency and Timeout.
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 25, 2021
 *  @Resource https://docs.silabs.com/resources/bluetooth/codeexamples/
 	 applicaCons/thermometer-example-with-efr32-internal-temperature-sensor/source/app.c
 **********************************************************************************************/

//Includes
//#define INCLUDE_LOG_DEBUG 1
#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT
#include "log.h"
#include "ble.h"

//defines

//variables
const uint8_t htp_service[2] = { 0x09, 0x18 };	//UUID for htp service
const uint8_t htp_characteristic[2] = { 0x1c, 0x2a };	//UUID for htp characteristic

const uint8_t server_button_service[16] = {0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x01, 0x00, 0x00, 0x00}; //{00000001-38c8-433e-87ec-652a2d136289}
const uint8_t server_button_characteristic[16] = {0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x02, 0x00, 0x00, 0x00}; //{00000002-38c8-433e-87ec-652a2d136289}

tx_power_based_on_rssi_t tx_power_based_on_rssi_table[] = {
		{ RSSI_MAX, -35, TX_POWER_MIN },
		{ -35, -45, -20 },
		{ -45, -55, -15 },
		{ -55, -65, -5 },
		{ -65, -75, 0 },
		{ -75, -85, 5 },
		{ -85, RSSI_MIN, TX_POWER_MAX }
};

uint8_t tx_power_based_on_rssi_table_length = sizeof(tx_power_based_on_rssi_table) 	/ sizeof(tx_power_based_on_rssi_table[0]);

ble_status_t ble_status = {
		.connection_handle = 0,
		.htp_indication_status = false,
		.tx_power_based_on_rssi = tx_power_based_on_rssi_table,
		.connection_status = DISCONNECTED
};

#if BUILD_INCLUDES_BLE_CLIENT
ble_client_t ble_client ={
	.status ={
		.connection_handle = 0,
		.htp_indication_status = false,
		.tx_power_based_on_rssi = tx_power_based_on_rssi_table, // tx_power_based_on_rssi_table
		.connection_status = DISCONNECTED},
	.service ={
		.handle = 0,
		.procedure_complete_status = true},
	.characteristic ={
		.handle = 0,
		.procedure_complete_status = true},
	.indication ={
		.procedure_complete_status = true}
};

uint8_t* ptr_to_byte_array;
#endif

#if BUILD_INCLUDES_BLE_SERVER
/** ---------------------------------------------------------------------------------------------------------
 * @brief event handlers for gecko's bluetooth events
 *	Events :
 *		Gecko boot : gecko_evt_system_boot_id
 *		Connected : gecko_evt_le_connection_opened_id
 *		External event : gecko_evt_system_external_signal_id
 *		Indication status changed : gecko_evt_gatt_server_characteristic_status_id
 *		rssi value got changed : gecko_evt_le_connection_rssi_id
 *		Disconnected : gecko_evt_le_connection_closed_id
 *
 *
 * @param event occurred in bluetooth
 * @return None
 *
 * @Resource https://docs.silabs.com/resources/bluetooth/codeexamples/
 applicaCons/thermometer-example-with-efr32-internal-temperature-sensor/source/app.c
 *--------------------------------------------------------------------------------------------------------- **/

void handle_ble_event_server(struct gecko_cmd_packet *event)
{
	struct gecko_msg_le_gap_start_advertising_rsp_t* return_start_advertising; //return-value for start advertising
	struct gecko_msg_system_get_bt_address_rsp_t* bt_address; //return-value for gecko_cmd_system_get_bt_address()
	int8_t rssi;
	char bluetooth_addr[18];	//char array to display address on LCD
	char passkey_str[10];
	uint8_t pb0_button_value = false;

	switch (BGLIB_MSG_ID(event->header)) {

		//Event occurs when Blue gecko board boots up
		case gecko_evt_system_boot_id:
			LOG_DEBUG("Executing Bluetooth boot sequence");
			//displaying server's bluetooth address on LCD
			bt_address = gecko_cmd_system_get_bt_address();
			sprintf(bluetooth_addr, "%x:%x:%x:%x:%x:%x", bt_address->address.addr[5], bt_address->address.addr[4],
					bt_address->address.addr[3], bt_address->address.addr[2], bt_address->address.addr[1],
					bt_address->address.addr[0]);
			displayPrintf(DISPLAY_ROW_BTADDR, bluetooth_addr);

			//setting tx_power to 0 at boot
			gecko_cmd_system_set_tx_power(0);
			//delete bonding on boot... device would be required to pair again on connection
			BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_delete_bondings());
			gecko_cmd_sm_configure(0b00001000,1);//configure as 'bonding request needs to be configured' and 1 for Display with Yes/No-buttons
			//Setting advertising parameters .(BTSTACK_CHECK_RESPONSE checks and reports errors if any)
			BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_set_advertise_timing(0,ADVERTISE_INTERVAL_MIN_VALUE,ADVERTISE_INTERVAL_MAX_VALUE,0,0)); //no limit on max duration and events
			//Start advertising
			LOG_DEBUG("Starting to advertise");
			return_start_advertising = gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable,
					le_gap_connectable_scannable);
			if (return_start_advertising->result == bg_err_success)
				displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
			else
				displayPrintf(DISPLAY_ROW_CONNECTION, "Error advertising");

			break;

			//Event occurs when client is connected over bluetooth
		case gecko_evt_le_connection_opened_id:
			LOG_DEBUG("Device connected");
			displayPrintf(DISPLAY_ROW_CONNECTION, "Connected"); //Displaying bluetooth connected over LCD
			ble_status.connection_handle = event->data.evt_le_connection_opened.connection;
			ble_status.connection_status = CONNECTED;
			ble_status.button_indication_status = false; //setting default value to indications
			//Setting connection parameters
			BTSTACK_CHECK_RESPONSE(gecko_cmd_le_connection_set_parameters(ble_status.connection_handle,
					CONNECTION_INTERVAL_MIN_VALUE, CONNECTION_INTERVAL_MAX_VALUE, SLAVE_LATENCY_VALUE, CONNECTION_TIMEOUT_VALUE));
			break;

		case gecko_evt_le_connection_parameters_id:
			/*Checking connection parameters
			connection_handle = event->data.evt_le_connection_parameters.connection;
			interval = event->data.evt_le_connection_parameters.interval;
			latency = event->data.evt_le_connection_parameters.latency;
			timeout = event->data.evt_le_connection_parameters.timeout;
			LOG_INFO("Connection : %d, Interval : %d, latency : %d, timeout : %d", connection_handle, interval, latency, timeout);*/
			break;

			//Event occurs when External event occurs. In our case this occurs because of LETIMER0 and I2C0 interrupts
		case gecko_evt_system_external_signal_id:
			//LOG_DEBUG("External Event : %d",event->data.evt_system_external_signal.extsignals);
			//confirming passkey on button transition high to low
			if(event->data.evt_system_external_signal.extsignals == PB0_SWITCH_HIGH_TO_LOW)
			{
				//required as events occur at both rising and falling edge
				if(ble_status.connection_status == BONDING)
					BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_passkey_confirm(ble_status.connection_handle, 1));	//1 means accept connection
			}
			//setting up attribute value and indications if enabled and devices are paired
			if(ble_status.connection_status == BONDED)
			{
				if(event->data.evt_system_external_signal.extsignals == PB0_SWITCH_LOW_TO_HIGH)
				{
					pb0_button_value = true;
					BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_server_write_attribute_value(gattdb_button_state,0,1,&pb0_button_value));	//offset 0, value length 1, value 1
					if(ble_status.button_indication_status == true)
						BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_server_send_characteristic_notification(0xFF, gattdb_button_state, 1, &pb0_button_value));
				}
				else if(event->data.evt_system_external_signal.extsignals == PB0_SWITCH_HIGH_TO_LOW)
				{
					pb0_button_value = false;
					BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_server_write_attribute_value(gattdb_button_state,0,1,&pb0_button_value));	//offset 0, value length 1, value 0
					if(ble_status.button_indication_status == true)
						BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_server_send_characteristic_notification(0xFF, gattdb_button_state, 1, &pb0_button_value));
				}
			}
			break;

			//Event occurs when Indication status changed
		case gecko_evt_gatt_server_characteristic_status_id:
			//get current rssi value
			BTSTACK_CHECK_RESPONSE(gecko_cmd_le_connection_get_rssi(ble_status.connection_handle));
			//checking for indication status of temperature characteristic
			if ((event->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement) &&
					(event->data.evt_gatt_server_characteristic_status.status_flags == 0x01))
			{
				if (event->data.evt_gatt_server_characteristic_status.client_config_flags == 0x02)
				{
					LOG_DEBUG("HTP indications turned ON");
					ble_status.htp_indication_status = true;
				}
				else if (event->data.evt_gatt_server_characteristic_status.client_config_flags == 0x00)
				{
					LOG_DEBUG("HTP indications turned OFF");
					ble_status.htp_indication_status = false;
				}
			}
			//checking for indication status of button state characteristic
			if ((event->data.evt_gatt_server_characteristic_status.characteristic == gattdb_button_state) &&
								(event->data.evt_gatt_server_characteristic_status.status_flags == 0x01)) // && (ble_status.connection_status == BONDED))
			{
				if (event->data.evt_gatt_server_characteristic_status.client_config_flags == 0x02)
				{
					LOG_DEBUG("button indications turned ON");
					ble_status.button_indication_status = true;
				}
				else if (event->data.evt_gatt_server_characteristic_status.client_config_flags == 0x00)
				{
					LOG_DEBUG("button indications turned OFF");
					ble_status.button_indication_status = false;
				}
			}
			break;

			//Event occurs when rssi value changes
		case gecko_evt_le_connection_rssi_id:
			//Setting tx power based on rssi value. Ranges and desired tx power is defined in this file by tx_power_based_on_rssi_table
			//	{RSSI_MAX, -35, TX_POWER_MIN},
			//		{-35, -45, -20},
			//		{-45, -55, -15},
			//		{-55, -65, -5},
			//		{-65, -75, 0},
			//		{-75, -85, 5},
			//		{-85, RSSI_MIN, TX_POWER_MAX}
			LOG_DEBUG("RSSI value changed. Setting new tx power");
			rssi = event->data.evt_le_connection_rssi.rssi;
			for (int i = 0; i < tx_power_based_on_rssi_table_length; i++)
			{
				if ((rssi <= ble_status.tx_power_based_on_rssi[i].rssi_range_max)
						&& (rssi > ble_status.tx_power_based_on_rssi[i].rssi_range_min))
				{
					gecko_cmd_system_set_tx_power(ble_status.tx_power_based_on_rssi[i].tx_power * 10); //TX power in 0.1dBm steps
					//return_set_tx_power = gecko_cmd_system_set_tx_power(ble_status.tx_power_based_on_rssi[i].tx_power*10);	//TX power in 0.1dBm steps
					//LOG_DEBUG("RSSI is %d dBm,   Tx power (expected %d) is : %.1f", rssi, ble_status.tx_power_based_on_rssi[i].tx_power,  (return_set_tx_power->set_power/10));
					break;
				}
			}
			break;

			//Event occurs when client is disconnected
		case gecko_evt_le_connection_closed_id:
			LOG_DEBUG("Device disconnected");
			displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
			ble_status.connection_status = DISCONNECTED;
			ble_status.htp_indication_status = false;//Explicitly turning indications to off as this is the default value on connection
			//setting Tx power to default value of 0
			gecko_cmd_system_set_tx_power(0);
			//Again start advertising once connection is closed
			return_start_advertising = gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
			if (return_start_advertising->result == bg_err_success)
				displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
			else
				displayPrintf(DISPLAY_ROW_CONNECTION, "Error advertising");

			break;


			//Event occurs every second
		case gecko_evt_hardware_soft_timer_id:
			//Refreshing LCD
			displayUpdate();
			break;

			//event occurs when bonding initiated by client
		case gecko_evt_sm_confirm_bonding_id:
			BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_bonding_confirm(ble_status.connection_handle,1));
			break;

			//event occurs when passkey is received
		case gecko_evt_sm_confirm_passkey_id:
			LOG_DEBUG("Passkey : %d",event->data.evt_sm_confirm_passkey.passkey);
			sprintf(passkey_str, "%d", (int)event->data.evt_sm_confirm_passkey.passkey);
			displayPrintf(DISPLAY_ROW_PASSKEY, passkey_str);
			//displayPrintf(DISPLAY_ROW_ACTION, "Confirm? N(PB1) Y(PB0)");
			displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");
			//PB0 shall react to bonding event only when state is 'passkey confirmation'
			ble_status.connection_status = BONDING;

			break;

			//event occurs when device is bonded
		case gecko_evt_sm_bonded_id:
			ble_status.connection_status = BONDED;
			displayPrintf(DISPLAY_ROW_PASSKEY, "");
			displayPrintf(DISPLAY_ROW_ACTION, "");
			displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");
			break;

		case gecko_evt_sm_bonding_failed_id:// - This event is triggered if pairing or bonding was performed in this operation and the result is failure.
			displayPrintf(DISPLAY_ROW_PASSKEY, "");
			displayPrintf(DISPLAY_ROW_ACTION, "");
			displayPrintf(DISPLAY_ROW_CONNECTION, "Bonding Failed");
			break;

		default:
			break;

	}
}

/** ---------------------------------------------------------------------------------------------------------
 * @brief Send indications to client over ble
 *
 * @param temperature measured over i2c
 * @return None
 *
 * @Resource https://docs.silabs.com/resources/bluetooth/codeexamples/
 applicaCons/thermometer-example-with-efr32-internal-temperature-sensor/source/app.c
 *--------------------------------------------------------------------------------------------------------- **/

void indicate_temperature_over_ble(float temperature)
{
	uint8_t temperature_buffer[5];
	uint8_t flags = 0x00;
	uint8_t *ptr_temperature_buffer = temperature_buffer;
	uint32_t temperature_conveted_to_uint32;

	/* Convert flags to bitstream and append them in the HTM temperature data buffer (htm_temperature_buffer) */
	UINT8_TO_BITSTREAM(ptr_temperature_buffer, flags);
	//converting temperature data to desired format
	temperature_conveted_to_uint32 = FLT_TO_UINT32((temperature * 1000), -3);
	//adding tempearture data to bitstream
	UINT32_TO_BITSTREAM(ptr_temperature_buffer, temperature_conveted_to_uint32);

	//Sending Health Thermometer as an indication when enabled through client
	//  0xFF => indications are sent to all connections

	BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_server_send_characteristic_notification(0xFF, gattdb_temperature_measurement, 5, temperature_buffer));

}

#endif

#if BUILD_INCLUDES_BLE_CLIENT
/** ---------------------------------------------------------------------------------------------------------
 * @brief event handlers for gecko bluetooth client's events
 *	Events :
 *		Gecko boot : gecko_evt_system_boot_id
 *		device found : gecko_evt_le_gap_scan_response_id
 *		Connected : gecko_evt_le_connection_opened_id
 *		service found : gecko_evt_gatt_service_id
 *		characteristic found : gecko_evt_gatt_characteristic_id
 *		procedure completed : gecko_evt_gatt_procedure_completed_id
 *		indication set up : gecko_evt_gatt_characteristic_value_id
 *		rssi value got changed : gecko_evt_le_connection_rssi_id
 *		Disconnected : gecko_evt_le_connection_closed_id
 *		1Hz : gecko_evt_hardware_soft_timer_id
 *
 *
 * @param event occurred in bluetooth
 * @return None
 *
 * @Resource soc_client example by Silicon Labs
 *--------------------------------------------------------------------------------------------------------- **/

void handle_ble_event_client(struct gecko_cmd_packet *event)
{
	struct gecko_msg_system_get_bt_address_rsp_t* bt_address;
	static bd_addr server_address = SERVER_BT_ADDRESS;
	char passkey_str[10];
	static bd_addr servers_address_on_connection;
	int8_t rssi;
	char bluetooth_addr[18];

	switch (BGLIB_MSG_ID(event->header)) {

			//Event occurs when Blue gecko board boots up
		case gecko_evt_system_boot_id:
			LOG_DEBUG("\n\nExecuting Bluetooth boot sequence");
			//displaying server's bluetooth address on LCD
			bt_address = gecko_cmd_system_get_bt_address();
			sprintf(bluetooth_addr, "%x:%x:%x:%x:%x:%x", bt_address->address.addr[5], bt_address->address.addr[4],
					bt_address->address.addr[3], bt_address->address.addr[2], bt_address->address.addr[1],
					bt_address->address.addr[0]);
			displayPrintf(DISPLAY_ROW_BTADDR, bluetooth_addr);

			//setting Tx power to default value of 0
			gecko_cmd_system_set_tx_power(0);
			//delete bonding on boot... device would be required to pair again on connection
			BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_delete_bondings());
			gecko_cmd_sm_configure(0b00000001,1);//configure as 'bonding request needs to be configured' and 1 for Display with Yes/No-buttons
			//setting up discovery type to passive
			BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_set_discovery_type(le_gap_phy_1m, 0));	//PHY = 1 and Scanning Type = Passive(0)
			//setting up discovery timing
			BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_set_discovery_timing(le_gap_phy_1m, SCAN_INTERVAL_VALUE, SCAN_WINDOW_VALUE));
			//starting discovery
			BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic));

			displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
			break;

			//Event occurs when any bluetooth device is found
		case gecko_evt_le_gap_scan_response_id:
			LOG_DEBUG("Device found... checking if its a device of interest");
			//Checking if address of discovered device matches with required address
			if (is_device_found_by_address(event->data.evt_le_gap_scan_response.address, server_address))
			{
				//stop scanning
				BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_end_procedure());
				//connect to a device
				BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_connect(event->data.evt_le_gap_scan_response.address,
						event->data.evt_le_gap_scan_response.address_type, le_gap_phy_1m));
			}
			break;

			//Event occurs when client is connected over bluetooth
		case gecko_evt_le_connection_opened_id:
			//displaying server's address on LCD
			servers_address_on_connection = event->data.evt_le_connection_opened.address;
			sprintf(bluetooth_addr, "%x:%x:%x:%x:%x:%x", servers_address_on_connection.addr[5],
					servers_address_on_connection.addr[4], servers_address_on_connection.addr[3],
					servers_address_on_connection.addr[2], servers_address_on_connection.addr[1],
					servers_address_on_connection.addr[0]);
			displayPrintf(DISPLAY_ROW_BTADDR2, bluetooth_addr);
			displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");	//Displaying bluetooth connected over LCD
			LOG_DEBUG("Device connected");
			ble_client.status.connection_handle = event->data.evt_le_connection_opened.connection;
			ble_client.status.connection_status = CONNECTED;
			//searching for button service at connection
			BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_discover_primary_services_by_uuid(ble_client.status.connection_handle,
					16, (const uint8_t* )server_button_service));	//uuid length is 16 bytes
			ble_client.service.procedure_complete_status = false;

			break;

			//Event occurs when service for which discovery initiated is found
		case gecko_evt_gatt_service_id:
			LOG_DEBUG("Service found");
			ble_client.service.handle = event->data.evt_gatt_service.service;	//interested service found ... here server_button
			break;

			//Event occurs when procedure for various actions is completed
		case gecko_evt_gatt_procedure_completed_id:
			if (event->data.evt_gatt_procedure_completed.result != 0)
			{
				LOG_DEBUG("Procedure completed with error %d", event->data.evt_gatt_procedure_completed.result);
				if(event->data.evt_gatt_procedure_completed.result == 0x040F)
				{
					BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_increase_security(ble_client.status.connection_handle));
					ble_client.characteristic.procedure_complete_status = true; //procedure completed even though with an error
				}
			}
			else
			{
				//checking if procedure completed is for service discovery which would be executed only once on coonection
				if (ble_client.service.procedure_complete_status == false)
				{
					LOG_DEBUG("Procedure completed for button service read");
					ble_client.service.procedure_complete_status = true;
				}
				else if (ble_client.characteristic.procedure_complete_status == false) //checking if procedure completed is for characteristic discovery
				{
					LOG_DEBUG("Procedure completed for button characteristic read");
					ble_client.characteristic.procedure_complete_status = true;
				}

			}
			break;

		case gecko_evt_system_external_signal_id:
			//LOG_DEBUG("External Event : %d",event->data.evt_system_external_signal.extsignals);
			//Creating passkey on button transition high to low
			if(event->data.evt_system_external_signal.extsignals == PB1_SWITCH_LOW_TO_HIGH)
			{
				if(ble_client.service.procedure_complete_status == true)
				{
					//making sure procedure for previous characteristic read is completed
					if((ble_client.characteristic.procedure_complete_status == true) || (ble_client.status.connection_status != BONDED))
					{
						BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_read_characteristic_value_by_uuid(ble_client.status.connection_handle, ble_client.service.handle, 16, (const uint8_t* )server_button_characteristic));
						ble_client.characteristic.procedure_complete_status = false;
					}

				}
			}
			if(event->data.evt_system_external_signal.extsignals == PB0_SWITCH_HIGH_TO_LOW)
				{
					if(ble_client.status.connection_status == BONDING)
						BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_passkey_confirm(ble_client.status.connection_handle, 1));	//1 means accept connection
				}
			break;


			//This event is triggered if pairing or bonding was performed in this operation and the result is success.
		case gecko_evt_sm_bonded_id:
			ble_client.status.connection_status = BONDED;
			displayPrintf(DISPLAY_ROW_PASSKEY, "");
			displayPrintf(DISPLAY_ROW_ACTION, "");
			displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");
			break;

			//This event is triggered if pairing or bonding was performed in this operation and the result is failure.
		case gecko_evt_sm_bonding_failed_id:
			displayPrintf(DISPLAY_ROW_PASSKEY, "");
			displayPrintf(DISPLAY_ROW_ACTION, "");
			displayPrintf(DISPLAY_ROW_CONNECTION, "Bonding Failed");
			break;


		case gecko_evt_sm_confirm_passkey_id:
			LOG_DEBUG("Passkey : %d",event->data.evt_sm_confirm_passkey.passkey);
			sprintf(passkey_str, "%d", (int)event->data.evt_sm_confirm_passkey.passkey);
			displayPrintf(DISPLAY_ROW_PASSKEY, passkey_str);
			displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");
			//PB0 shall react to bonding event only when state is 'passkey confirmation'
			ble_client.status.connection_status = BONDING;

			break;

		case gecko_evt_gatt_characteristic_value_id:
			if(event->data.evt_gatt_characteristic_value.att_opcode == gatt_read_by_type_response)
			{
				if(event->data.evt_gatt_characteristic_value.value.data[0] == 1)
				{
					displayPrintf(DISPLAY_ROW_ACTION, "Button Released");
				}
				else if(event->data.evt_gatt_characteristic_value.value.data[0] == 0)
				{
					displayPrintf(DISPLAY_ROW_ACTION, "Button Pressed");
				}
			}
			// Trigger RSSI measurement on the connection
			BTSTACK_CHECK_RESPONSE(gecko_cmd_le_connection_get_rssi(ble_client.status.connection_handle));
			break;

			//Event occurs when rssi value changes
		case gecko_evt_le_connection_rssi_id:
			//Setting tx power based on rssi value. Ranges and desired tx power is defined in this file by tx_power_based_on_rssi_table
			//	{RSSI_MAX, -35, TX_POWER_MIN},
			//		{-35, -45, -20},
			//		{-45, -55, -15},
			//		{-55, -65, -5},
			//		{-65, -75, 0},
			//		{-75, -85, 5},
			//		{-85, RSSI_MIN, TX_POWER_MAX}
			LOG_DEBUG("RSSI value changed. Setting new tx power");
			rssi = event->data.evt_le_connection_rssi.rssi;
			for (int i = 0; i < tx_power_based_on_rssi_table_length; i++)
			{
				if ((rssi <= ble_client.status.tx_power_based_on_rssi[i].rssi_range_max)
						&& (rssi > ble_client.status.tx_power_based_on_rssi[i].rssi_range_min))
				{
					gecko_cmd_system_set_tx_power(ble_client.status.tx_power_based_on_rssi[i].tx_power * 10);//TX power in 0.1dBm steps
					//return_set_tx_power = gecko_cmd_system_set_tx_power(ble_status.tx_power_based_on_rssi[i].tx_power*10);	//TX power in 0.1dBm steps
					//LOG_DEBUG("RSSI is %d dBm,   Tx power (expected %d) is : %.1f", rssi, ble_status.tx_power_based_on_rssi[i].tx_power,  (return_set_tx_power->set_power/10));
					break;
				}
			}
			break;

			//Event occurs when disconnected
		case gecko_evt_le_connection_closed_id:
			BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic));
			displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
			displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
			displayPrintf(DISPLAY_ROW_BTADDR2, "");
			displayPrintf(DISPLAY_ROW_PASSKEY, "");
			displayPrintf(DISPLAY_ROW_ACTION, "");
			//setting Tx power to default value of 0
			gecko_cmd_system_set_tx_power(0);
			//resetting flags and handles
			ble_client.status.connection_handle = 0;
			ble_client.status.connection_status = DISCONNECTED;
			ble_client.status.htp_indication_status = false;
			ble_client.service.handle = 0;
			ble_client.service.procedure_complete_status = true;
			ble_client.characteristic.handle = 0;
			ble_client.characteristic.procedure_complete_status = true;
			ble_client.indication.procedure_complete_status = true;
			break;

			//Event occurs at 1Hz
		case gecko_evt_hardware_soft_timer_id:
			//refreshing LCD
			displayUpdate();
			break;

		default:
			break;

	}
}

/** ---------------------------------------------------------------------------------------------------------
 * @brief Compares for addresses and returns true if there is address match
 *
 * @param address to be discovered, address to be matched with
 * @return true if address match
 *--------------------------------------------------------------------------------------------------------- **/
bool is_device_found_by_address(bd_addr address_of_discovered_device, bd_addr required_address)
{
	LOG_DEBUG("Discovered device is -> %x:%x:%x:%x:%x:%x", address_of_discovered_device.addr[5], address_of_discovered_device.addr[4],
			address_of_discovered_device.addr[3], address_of_discovered_device.addr[2], address_of_discovered_device.addr[1],
			address_of_discovered_device.addr[0]);

	for (int i = 0; i < BLE_ADDR_LENGTH; i++)
	{
		//LOG_DEBUG("found : %x\t required : %x", address_of_discovered_device.addr[i], required_address.addr[i]);
		if (address_of_discovered_device.addr[i] != required_address.addr[i])
			return false;
	}
	return true;
}


/** ---------------------------------------------------------------------------------------------------------
 * @brief Converts temperature to float format
 *
 * @param bitstream received over bluetooth
 * @return float converted value of temperature
 *--------------------------------------------------------------------------------------------------------- **/
float bitstream_to_float(const uint8_t *ptr_to_byte_array)
{
	float temperature = 0;
	temperature = (ptr_to_byte_array[1] << 0) | (ptr_to_byte_array[2] << 8) | (ptr_to_byte_array[3] << 16);

	// msb of [3] defines sign
	if (ptr_to_byte_array[3] & 0x80)
	{
		temperature = -temperature;
	}
	temperature = temperature / 1000;

	return temperature;
} // bitstream_to_float

#endif

#else
#endif
