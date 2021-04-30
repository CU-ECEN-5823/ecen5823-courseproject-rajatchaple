/*********************************************************************************************
 *  @file proximity.h
 *	@brief This file contains defines, includes and typedefs for proximity.c
 *
 *  @author : Rajat Chaple (GATT client code)
 *
 *
 *  @date     April 29, 2020 (last update)
 **********************************************************************************************/
#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT
#ifndef SRC_SENSORS_CONFIG_H_
#define SRC_SENSORS_CONFIG_H_


#define PROXIMITY_SENSOR_READ_INTERVAL	500000 //microseconds
#define PROXIMITY_SENSOR_THRESHOLD_VALUE	(3000)


//typedef enum proximity_sensor_tests_e
enum proximity_sensor_tests
{
	STATE_TEST_COMMUNICATION,
	STATE_TEST_DATA_IN_VALID_RANGE,
	STATE_TEST_GESTURE,
	TOTAL_NUM_OF_TESTS
};

typedef enum proximity_sensor_read_e
{
	PROXIMITY_PRODUCT_ID,
	IR_LED_CURRENT,
	PROXIMITY_SENSED_VALUE_RAW,
	PROXIMITY_INT_STATUS_REGISTER
}proximity_sensor_read_t;

void blocking_read_i2c(uint8_t reg, uint8_t* data_buffer);
void blocking_write_i2c(uint8_t reg, uint8_t data);
void proximity_sensor_config(void);
void test_proximity_sensor(void);
uint16_t proximity_sensor_read(proximity_sensor_read_t);

#endif /* SRC_SENSORS_CONFIG_H_ */

#else
#endif
