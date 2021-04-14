/*********************************************************************************************
 *  @file ble.h
 *	@brief This file contains defines, includes anf function prototypes for ble.c
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 25, 2021
 **********************************************************************************************/
#ifndef SRC_BLE_H_
#define SRC_BLE_H_

#include "native_gecko.h"
#include "gecko_ble_errors.h"
#include "ble_device_type.h"
#include "gatt_db.h"
#include "gpio.h"
#include "infrastructure.h"


#define RSSI_MIN -128
#define RSSI_MAX 127

#define TX_POWER_MIN (-100)	//as defined in characteristic
#define TX_POWER_MAX (20)	//as defined in characteristic

#define ADVERTISE_INTERVAL_MIN_MS 	(250)
#define ADVERTISE_INTERVAL_MAX_MS 	(250)

#define CONNECTION_INTERVAL_MIN_MS 	(75)
#define CONNECTION_INTERVAL_MAX_MS 	(75)
#define SLAVE_LATENCY_MS		   	(300)
#define CONNECTION_TIMEOUT_MS		(900)	// shall be more than (1 + latency_in_Num_connection_intervals) * max_interval * 2

#define SCAN_INTERVAL_MS			(50)
#define SCAN_WINDOW_MS				(25)


#define ADVERTISE_INTERVAL_MIN_VALUE 	(ADVERTISE_INTERVAL_MIN_MS/0.625)
#define ADVERTISE_INTERVAL_MAX_VALUE 	(ADVERTISE_INTERVAL_MAX_MS/0.625)

#define CONNECTION_INTERVAL_MIN_VALUE 	(CONNECTION_INTERVAL_MIN_MS/1.25)
#define CONNECTION_INTERVAL_MAX_VALUE 	(CONNECTION_INTERVAL_MAX_MS/1.25)
#define SLAVE_LATENCY_VALUE			   	((SLAVE_LATENCY_MS/CONNECTION_INTERVAL_MAX_MS) - 1)
#define CONNECTION_TIMEOUT_VALUE		(CONNECTION_TIMEOUT_MS/10)

#define SCAN_INTERVAL_VALUE				(SCAN_INTERVAL_MS/0.625)
#define SCAN_WINDOW_VALUE				(SCAN_WINDOW_MS/0.625)

#define BLE_ADDR_LENGTH					(6)

//datatypes and global variables
typedef struct tx_power_based_on_rssi_t{
	int8_t rssi_range_max;
	int8_t rssi_range_min;
	int16_t tx_power;
}tx_power_based_on_rssi_t;

typedef enum{
	DISCONNECTED,
	CONNECTED,
	BONDING,
	BONDED
}connection_status_t;

typedef struct ble_status_s{
	uint8_t connection_handle;
	bool htp_indication_status;
	bool button_indication_status;
	tx_power_based_on_rssi_t* tx_power_based_on_rssi;
	connection_status_t connection_status;
}ble_status_t;

typedef struct ble_client_s{
	ble_status_t status;
	struct{
		uint32_t handle;
		bool procedure_complete_status;
	}service;
	struct{
			uint32_t handle;
			bool procedure_complete_status;
		}characteristic;
	struct{
			bool procedure_complete_status;
		}indication;
}ble_client_t;

//function prototypes
void handle_ble_event_server(struct gecko_cmd_packet *event);
void handle_ble_event_client(struct gecko_cmd_packet *event);
bool is_device_found_by_address(bd_addr address_of_discovered_device, bd_addr required_address);
float bitstream_to_float(const uint8_t *);

#endif /* SRC_BLE_H_ */
