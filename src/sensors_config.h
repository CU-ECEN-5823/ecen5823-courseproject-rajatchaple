/*********************************************************************************************
 *  @file sensors_config.h
 *	@brief This file contains defines, includes and typedefs for sensors_config.c
 *
 *  @author : Rajat Chaple
 *  @date Created on: Apr 15, 2021
 **********************************************************************************************/
#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT
#ifndef SRC_SENSORS_CONFIG_H_
#define SRC_SENSORS_CONFIG_H_

void proximity_sensor_config(void);

#endif /* SRC_SENSORS_CONFIG_H_ */

#else
#endif
