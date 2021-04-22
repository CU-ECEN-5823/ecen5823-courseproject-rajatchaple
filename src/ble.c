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

#include "ble.h"

// globals
uint32_t signal;
uint8_t conn_handle;
gatt_transfer_states_t gatt_state = 0;

// Service handle
uint32_t service_handle;


// Server security - MITM
uint32_t passkey;

uint8_t axis_orientation_indication_en_flag = 0;
uint8_t time_until_trigger_indication_en_flag = 0;
uint8_t connected_status_flag = 0;

uint8_t first_time_tut_send_flag = 0;

uint8_t time_until_trigger[3] = {10, 20, 30};
uint8_t current_tut_index = 0;
uint8_t tut_options_max = 3;

uint8_t remote_gatt_cmd_in_progress = 0;
// bit0=axis_orientation_indication
// bit1=time_until_trigger_indication
uint8_t indication_gatt_cmd_defer_flag = 0;


// Bonding not started=0
// Bonding in progress=1
// Bonding complete=2
uint8_t bonded = 0;

struct gecko_msg_system_get_bt_address_rsp_t* server_bt_addr;
struct gecko_msg_system_get_bt_address_rsp_t* client_bt_addr;

uint8_t done = 0;

void handle_ble_event(struct gecko_cmd_packet *evt)
{


	switch (BGLIB_MSG_ID(evt->header))
	{

		case gecko_evt_system_boot_id:
		{
			server_bt_addr = gecko_cmd_system_get_bt_address();
			bd_addr val = server_bt_addr->address;
			// Display to LCD
			displayPrintf(DISPLAY_ROW_NAME, "Server");
			displayPrintf(DISPLAY_ROW_BTADDR, "%x:%x:%x:%x:%x:%x", val.addr[5], val.addr[4], val.addr[3], val.addr[2],
					val.addr[1], val.addr[0]);

			struct gecko_msg_le_gap_set_advertise_timing_rsp_t *ret1 = gecko_cmd_le_gap_set_advertise_timing(0, 400, 400, 0, 0);
			if (ret1->result != 0)
			{
				#if INCLUDE_LOGGING
					LOG_ERROR("ERROR: %d | response code from gecko_cmd_le_gap_set_advertise_timing()", ret1->result);
				#endif
			}
			struct gecko_msg_le_gap_start_advertising_rsp_t *ret2 = gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
			if (ret2->result != 0)
			{
				#if INCLUDE_LOGGING
					LOG_ERROR("ERROR: %d | response code from gecko_cmd_le_gap_start_advertising()", ret2->result);
				#endif
			}

			// Server Security - MITM
			// Delete bondings here
			bonded = 0;
			struct gecko_msg_sm_delete_bondings_rsp_t *ret7 = gecko_cmd_sm_delete_bondings();
			if (ret7->result != 0)
			{
				#if INCLUDE_LOGGING
					LOG_ERROR("ERROR: %d | response code from gecko_cmd_sm_delete_bondings()", ret7->result);
				#endif
			}




			// Display to LCD
			displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");


		}
			break;

		case gecko_evt_le_connection_opened_id:
		{
			// for extra credit
			connected_status_flag = 1;

			// Display to LCD
			displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");

			conn_handle = evt->data.evt_le_connection_opened.connection;
			// 60 -> 75ms connection interval
			// 3 slave latency slots -> 4 * 75 = 300ms. But slaves listens the 4th slot. So 4-1 = 3.
			// 70 -> supervisor timeout = (1*3)*(75*2) = 600 -> 700ms (some value greater than 600ms). 70 * 10ms. So put 70.
			struct gecko_msg_le_connection_set_parameters_rsp_t *ret3 =  gecko_cmd_le_connection_set_parameters(conn_handle, 60, 60, 3, 70);
			if (ret3->result != 0)
			{
				#if INCLUDE_LOGGING
					LOG_ERROR("ERROR: %d | response code from gecko_cmd_le_connection_set_parameters()", ret3->result);
				#endif
			}


			//Configure security manager here
			struct gecko_msg_sm_configure_rsp_t *ret8 = gecko_cmd_sm_configure(SM_CONFIG_FLAGS, IO_CAPABILITY);
			if (ret8->result != 0)
			{
				#if INCLUDE_LOGGING
					LOG_ERROR("ERROR: %d | response code from gecko_cmd_sm_configure()", ret8->result);
				#endif
			}


		}
			break;

		case gecko_evt_gatt_server_characteristic_status_id:
		{
			remote_gatt_cmd_in_progress = 0;

			// bit0=axis_orientation_indication
			// bit1=time_until_trigger_indication


			if ((remote_gatt_cmd_in_progress == 0) && ((indication_gatt_cmd_defer_flag & 0x01) != 0))
			{
				indication_gatt_cmd_defer_flag &= ~(0x01);

				uint8_t t_buffer[3];
				uint8_t *ptr = t_buffer;
				UINT8_TO_BITSTREAM(ptr, 0x00)

				UINT16_TO_BITSTREAM(ptr, (uint16_t)accel_data.y);
				uint8_t f_buffer[3];
				f_buffer[2] = t_buffer[2];
				f_buffer[1] = t_buffer[1];
				f_buffer[0] = t_buffer[0];

				struct gecko_msg_gatt_server_send_characteristic_notification_rsp_t *ret21 = gecko_cmd_gatt_server_send_characteristic_notification(conn_handle,
						gattdb_y_axis_value, 3, f_buffer);

				remote_gatt_cmd_in_progress = 1;

				if (ret21->result != 0)
				{
					remote_gatt_cmd_in_progress = 0; // Because gatt_cmd failed.
					#if INCLUDE_LOGGING
						LOG_ERROR("ERROR: %d | response code from gecko_cmd_gatt_server_send_characteristic_notification()", ret21->result);
					#endif
				}

			}


			if ((remote_gatt_cmd_in_progress == 0) && ((indication_gatt_cmd_defer_flag & 0x02) != 0))
			{
				indication_gatt_cmd_defer_flag &= ~(0x02);

				uint8_t t_buffer[2];
				uint8_t *ptr = t_buffer;
				UINT8_TO_BITSTREAM(ptr, 0x00)

				UINT8_TO_BITSTREAM(ptr, (uint8_t)time_until_trigger[current_tut_index]);
				uint8_t f_buffer[2];
				f_buffer[1] = t_buffer[1];
				f_buffer[0] = t_buffer[0];

				struct gecko_msg_gatt_server_send_characteristic_notification_rsp_t *ret22 = gecko_cmd_gatt_server_send_characteristic_notification(conn_handle,
						gattdb_seconds, 2, f_buffer);

				remote_gatt_cmd_in_progress = 1;

				if (ret22->result != 0)
				{
					remote_gatt_cmd_in_progress = 0; // Because gatt_cmd failed.
					#if INCLUDE_LOGGING
						LOG_ERROR("ERROR: %d | response code from gecko_cmd_gatt_server_send_characteristic_notification()", ret22->result);
					#endif
				}

			}



			struct gecko_msg_le_connection_get_rssi_rsp_t *ret4 = gecko_cmd_le_connection_get_rssi(conn_handle);
			if (ret4->result != 0)
			{
				#if INCLUDE_LOGGING
					LOG_ERROR("ERROR: %d | response code from gecko_cmd_le_connection_get_rssi()", ret4->result);
				#endif
			}

			if ((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_y_axis_value)
					&& (evt->data.evt_gatt_server_characteristic_status.status_flags == gatt_server_client_config)
						&& (evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_indication))
			{

				// enabled
				axis_orientation_indication_en_flag = 1;

			}
			else if (evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_disable)
			{

				axis_orientation_indication_en_flag = 0;

			}



			if ((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_seconds)
					&& (evt->data.evt_gatt_server_characteristic_status.status_flags == gatt_server_client_config)
						&& (evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_indication))
			{

				// enabled
				time_until_trigger_indication_en_flag = 1;


			}
			else if (evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_disable)
			{

				// disabled
				time_until_trigger_indication_en_flag = 0;

			}
		}
			break;

		// For my state machine
		case gecko_evt_system_external_signal_id:
		{

			signal = evt->data.evt_system_external_signal.extsignals;


			// Send indications to client.
			// Need for reference. Don't remove this comment.
			// if (calibration_complete_flag==2 && axis_orientation_indication_en_flag && (signal == 130))
			if (bonded == 2 && calibration_complete_flag == 1 && axis_orientation_indication_en_flag && (signal == 130))
			{

				if (remote_gatt_cmd_in_progress == 0)
				{


					uint8_t t_buffer[3];
					uint8_t *ptr = t_buffer;
					UINT8_TO_BITSTREAM(ptr, 0x00)

					UINT16_TO_BITSTREAM(ptr, (uint16_t)accel_data.y);
					uint8_t f_buffer[3];
					f_buffer[2] = t_buffer[2];
					f_buffer[1] = t_buffer[1];
					f_buffer[0] = t_buffer[0];

					struct gecko_msg_gatt_server_send_characteristic_notification_rsp_t *ret19 = gecko_cmd_gatt_server_send_characteristic_notification(conn_handle,
							gattdb_y_axis_value, 3, f_buffer);

					remote_gatt_cmd_in_progress = 1;
					done = 1;

					if (first_time_tut_send_flag == 0)
					{
						first_time_tut_send_flag = 1;
						indication_gatt_cmd_defer_flag |= 0x02; // Defer it so that it is sent after first calib value was sent.

					}

					if (ret19->result != 0)
					{
						remote_gatt_cmd_in_progress = 0; // Because gatt_cmd failed.
						#if INCLUDE_LOGGING
							LOG_ERROR("ERROR: %d | response code from gecko_cmd_gatt_server_send_characteristic_notification()", ret19->result);
						#endif
					}

				}
				else if(remote_gatt_cmd_in_progress != 0)
				{
					// defer the gatt call. Once the previous gatt call finishes and gecko_evt_gatt_server_characteristic_status event
					// triggers, this gatt call will occur there of no other outstanding gatt calls are present at that time.

					indication_gatt_cmd_defer_flag |= 0x01;

				}

				return;

			}

			// PB0 press
			if (bonded == 2 && time_until_trigger_indication_en_flag && signal == 200)
			{
				current_tut_index = (current_tut_index + 1 ) % tut_options_max;

				displayPrintf(DISPLAY_ROW_TUT, "TUT: %us", time_until_trigger[current_tut_index]);

				if (remote_gatt_cmd_in_progress == 0)
				{

					uint8_t t_buffer[2];
					uint8_t *ptr = t_buffer;
					UINT8_TO_BITSTREAM(ptr, 0x00)

					UINT8_TO_BITSTREAM(ptr, (uint8_t)time_until_trigger[current_tut_index]);
					uint8_t f_buffer[2];
					f_buffer[1] = t_buffer[1];
					f_buffer[0] = t_buffer[0];

					struct gecko_msg_gatt_server_send_characteristic_notification_rsp_t *ret20 = gecko_cmd_gatt_server_send_characteristic_notification(conn_handle,
							gattdb_seconds, 2, f_buffer);

					remote_gatt_cmd_in_progress = 1;

					if (ret20->result != 0)
					{
						remote_gatt_cmd_in_progress = 0; // Because gatt_cmd failed.
						#if INCLUDE_LOGGING
							LOG_ERROR("ERROR: %d | response code from gecko_cmd_gatt_server_send_characteristic_notification()", ret20->result);
						#endif
					}

				}
				else if(remote_gatt_cmd_in_progress != 0)
				{
					// defer the gatt call. Once the previous gatt call finishes and gecko_evt_gatt_server_characteristic_status event
					// triggers, this gatt call will occur there of no other outstanding gatt calls are present at that time.

					indication_gatt_cmd_defer_flag |= 0x02;

				}


			}




			// Signal stored in a run-queue type array
			scheduler_set_event_common(signal);

		}
			break;

		case gecko_evt_le_connection_rssi_id:
		{
			int8 rssi = evt->data.evt_le_connection_rssi.rssi;



			if (rssi > -35)
			{
				gecko_cmd_system_set_tx_power(-300); // set tx_power to TX Min = -30 dBm

			}
			if ((-35 >= rssi) && (rssi > -45))
			{
				gecko_cmd_system_set_tx_power(-200); // set tx_power to -20db

			}
			if ((-45 >= rssi) && (rssi > -55))
			{
				gecko_cmd_system_set_tx_power(-150); // set tx_power to -15db

			}
			if ((-55 >= rssi) && (rssi > -65))
			{
				gecko_cmd_system_set_tx_power(-50); // set tx_power to -5db

			}
			if ((-65 >= rssi) && (rssi > -75))
			{
				gecko_cmd_system_set_tx_power(0); // set tx_power to 0db

			}
			if ((-75 >= rssi) && (rssi > -85))
			{
				gecko_cmd_system_set_tx_power(50); // set tx_power to 5db

			}
			// The  maximum TX power value from EFR32BG13P632F512GM48 is 10 dBm
			if (rssi <= -85)
			{
				gecko_cmd_system_set_tx_power(100); // set tx_power to TX Max

			}

			#if BOND_DISCONNECT
				// rssi is set for the next transmission.
				// Enable to sleep in EM3 mode here i.e., sleep_mode_blocked = sleepEM4
				if (calibration_complete_flag == 1 && done == 1 && indication_gatt_cmd_defer_flag == 0)
				{
					done = 0;
					struct gecko_msg_le_connection_close_rsp_t *ret23 = gecko_cmd_le_connection_close(conn_handle);
					if (ret23->result != 0)
					{

						#if INCLUDE_LOGGING
							LOG_ERROR("ERROR: %d | response code from gecko_cmd_le_connection_close()", ret23->result);
						#endif
					}


					SLEEP_SleepBlockEnd(sleepEM2);
					SLEEP_SleepBlockBegin(sleep_mode_blocked);
				}

			#endif

		}
			break;

		case gecko_evt_le_connection_closed_id:

			// for extra credit
			connected_status_flag = 0;
			axis_orientation_indication_en_flag = 0;
			time_until_trigger_indication_en_flag = 0;

			gecko_cmd_system_set_tx_power(0); //  TX Power should be set to 0db

			// Server Security - MITM
			// Delete bondings here
			bonded = 0;
			struct gecko_msg_sm_delete_bondings_rsp_t *ret11 = gecko_cmd_sm_delete_bondings();
			if (ret11->result != 0)
			{
				#if INCLUDE_LOGGING
					LOG_ERROR("ERROR: %d | response code from gecko_cmd_sm_delete_bondings()", ret11->result);
				#endif
			}


			struct gecko_msg_le_gap_start_advertising_rsp_t *ret6 = gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
			if (ret6->result != 0)
			{
				#if INCLUDE_LOGGING
					LOG_ERROR("ERROR: %d | response code from gecko_cmd_le_gap_start_advertising()", ret6->result);
				#endif
			}

			// Display to LCD
			displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");



			break;

		// for assignment questions only
		case gecko_evt_le_connection_parameters_id:
		{
			/* uint16_t value = evt->data.evt_le_connection_parameters.interval;
			{
				#if INCLUDE_LOGGING
					LOG_INFO("gecko_evt_le_connection_parameters_id-interval: %d", value);

				#endif
			}

			value = evt->data.evt_le_connection_parameters.latency;
			{
				#if INCLUDE_LOGGING
					LOG_INFO("gecko_evt_le_connection_parameters_id-latency: %d", value);

				#endif
			}
			value = evt->data.evt_le_connection_parameters.timeout;
			{
				#if INCLUDE_LOGGING
					LOG_INFO("gecko_evt_le_connection_parameters_id-timeout: %d", value);

				#endif
			}*/

		}
			break;

		case gecko_evt_hardware_soft_timer_id:
		{
			uint8_t handle = evt->data.evt_hardware_soft_timer.handle;

			if (handle == 2)
			{

				displayUpdate();

			}
		}
			break;

		// Server Security - MITM

		case gecko_evt_sm_confirm_bonding_id:
		{
			bonded = 1;
			// 1 -> To confirm or to say yes.
			struct gecko_msg_sm_bonding_confirm_rsp_t *ret9 = gecko_cmd_sm_bonding_confirm(conn_handle, 1);
			if (ret9->result != 0)
			{
				#if INCLUDE_LOGGING
					LOG_ERROR("ERROR: %d | response code from gecko_cmd_sm_bonding_confirm()", ret9->result);
				#endif
			}

		}
			break;


	  case gecko_evt_sm_bonded_id:

		// bonding complete
		bonded = 2;

		displayPrintf(DISPLAY_ROW_PASSKEY, " ");
		displayPrintf(DISPLAY_ROW_TUT, "TUT: %us", time_until_trigger[current_tut_index]);
		displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");


		break;


	  case gecko_evt_sm_bonding_failed_id:
	  {


		struct gecko_msg_le_connection_close_rsp_t *ret18 = gecko_cmd_le_connection_close(conn_handle);
		if (ret18->result != 0)
		{

			#if INCLUDE_LOGGING
				LOG_ERROR("ERROR: %d | response code from gecko_cmd_le_connection_close()", ret18->result);
			#endif
		}

	  }

		break;



	}
}
#endif
