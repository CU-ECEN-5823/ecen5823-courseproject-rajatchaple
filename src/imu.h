/***********************************************************************
 * @file      imu.h
 * @brief     Contains imu sensor related headers.
 *
 * @author	  Sundar Krishnakumar, sundar.krishnakumar@Colorado.edu (updates)
 * @date      April 16, 2020 (last update)
 *
 * @institution University of Colorado Boulder (UCB)
 * @course      ECEN 5823-001: IoT Embedded Firmware (Spring 2021)
 * @instructor  David Sluiter
 *
 * @assignment ecen5823-courseproject-rajatchaple
 * @due        April 16, 2020
 *
 * @resources  Utilized Silicon Labs' EMLIB peripheral libraries to
 * implement functionality.
 *
 *
 * @copyright  All rights reserved. Distribution allowed only for the
 * use of assignment grading. Use of code excerpts allowed at the
 * discretion of author. Contact for permission.
 */
#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

#else

#ifndef __IMU_H__
#define __IMU_H__

#include "i2cspm.h"
#include "gpio.h"
#include "timers.h"
#include "log.h"

////////////////////////////////////////////// FXOS8700 related ///////////////////////////////////////////////////////////////
#define FXOS8700_ADDRESS (0x1F) // 00111117 bit slave address

#define FXOS8700CQ_XYZ_DATA_CFG 		0x0E
#define FXOS8700CQ_CTRL_REG1 				0x2A
#define FXOS8700CQ_M_CTRL_REG1 			0x5B
#define FXOS8700CQ_M_CTRL_REG2 			0x5C

// hyb_autoinc_mode bit has been set to enable the
// reading of all accelerometer X, Y, Z data in a single-burst read operation.
#define FXOS8700CQ_STATUS 0x00
// status byte  + 6 bytes accelerometer reading
#define FXOS8700CQ_READ_LEN 				7


// Macro for mg per LSB at +/- 2g sensitivity (1 LSB = 0.244mg)
#define ACCEL_MG_LSB_2G (0.244F)
// Macro for mg per LSB at +/- 4g sensitivity (1 LSB = 0.488mg)
#define ACCEL_MG_LSB_4G (0.488F)
// Macro for mg per LSB at +/- 8g sensitivity (1 LSB = 0.976mg)
#define ACCEL_MG_LSB_8G (0.976F)


// For raw X,Y and Z accelerometer values.
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;

}accel_raw_typedef;

// For X,Y and Z values in milliG(accelerometer)
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;

}accel_data_typedef;

extern accel_data_typedef accel_data;


////////////////////////////////////////////// FXOS8700 related ///////////////////////////////////////////////////////////////



////////////////////////////////////////////// FXAS21002C related ///////////////////////////////////////////////////////////////

// 7-bit address for this sensor
#define FXAS21002C_ADDRESS (0x21) // 0100001

// status byte  + 6 bytes gyroscope axis reading
#define FXAS21002C_READ_LEN 7

#define FXAS21002C_REGISTER_STATUS (0x00)
#define FXAS21002C_REGISTER_OUT_X_MSB (0x01)
#define FXAS21002C_REGISTER_OUT_X_LSB (0x02)
#define FXAS21002C_REGISTER_OUT_Y_MSB (0x03)
#define FXAS21002C_REGISTER_OUT_Y_LSB (0x04)
#define FXAS21002C_REGISTER_OUT_Z_MSB (0x05)
#define FXAS21002C_REGISTER_OUT_Z_LSB (0x06)

#define FXAS21002C_REGISTER_CTRL_REG0 (0x0D)
#define FXAS21002C_REGISTER_CTRL_REG1 (0x13)

// Gyroscope sensitivity at 250dps
#define GYRO_SENSITIVITY_250DPS (7.8125F) // 1LSB = 7.8125 mdps
// Gyroscope sensitivity at 500dps
#define GYRO_SENSITIVITY_500DPS (15.625F) // 1LSB = 15.625 mdps
// Gyroscope sensitivity at 1000dps
#define GYRO_SENSITIVITY_1000DPS (31.25F) // 1LSB = 31.25 mdps
// Gyroscope sensitivity at 2000dps
#define GYRO_SENSITIVITY_2000DPS (62.5F) // 1LSB = 62.5 mdps

#define FXAS21002C_MDPS_TO_MRAD (0.01745F) // 1 degree = 0.0174533 rad


// For raw X,Y and Z gyroscope values.
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;

}gyro_raw_typedef;

// For X,Y and Z values in millirad/s
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;

}gyro_data_typedef;

extern uint16_t tilt;
extern uint8_t calibration_complete_flag;

void I2C0_init(void);
void turn_on_IMU(void);
void FXOS_send_standby_signal(void);
void FXOS_DATA_CFG_start(void);
void FXOS_CTRL_REG1_signal_start(void);
void FXAS_standby_signal_send(void);
void FXAS_reset_signal_send(void);
void FXAS_CTRL_REG0_signal_start(void);
void FXAS_CTRL_REG1_signal_start(void);
void wait_for_65_millis(void);
void FXOS_measure_start(void);
void FXAS_measure_start(void);
void FXAS_measure_stop_off_read(void);



#endif /* __IMU_H__ */

#endif
