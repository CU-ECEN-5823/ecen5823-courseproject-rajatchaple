/***********************************************************************
 * @file      imu.c
 * @brief     Contains IMU sensor library. Works based on interrupt based I2C.
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
 * @ reference https://github.com/adafruit/Adafruit_FXAS21002C/blob/master/Adafruit_FXAS21002C.cpp
 * 			   https://github.com/adafruit/Adafruit_FXOS8700/blob/master/Adafruit_FXOS8700.cpp
 *
 * @copyright  All rights reserved. Distribution allowed only for the
 * use of assignment grading. Use of code excerpts allowed at the
 * discretion of author. Contact for permission.
 */
#include "ble_device_type.h"
#if BUILD_INCLUDES_BLE_CLIENT

#else

#include "imu.h"

I2CSPM_Init_TypeDef i2cspm_init_custom =
{ I2C0,                       /* Use I2C instance 0 */                       \
  gpioPortC,                  /* SCL port */                                 \
  10,                         /* SCL pin */                                  \
  gpioPortC,                  /* SDA port */                                 \
  11,                         /* SDA pin */                                  \
  14,                         /* Location of SCL */                          \
  16,                         /* Location of SDA */                          \
  0,                          /* Use currently configured reference clock */ \
  I2C_FREQ_STANDARD_MAX,      /* Set to standard rate  */                    \
  i2cClockHLRStandard,        /* Set to use 4:4 low/high duty cycle */       \
};




// Must be global.
I2C_TransferSeq_TypeDef seq;
uint8_t write_data[2];
uint8_t read_data[7];

uint8_t accel_buff[7];
uint8_t gyro_buff[7];

accel_raw_typedef accel_raw;
accel_data_typedef accel_data;

gyro_raw_typedef gyro_raw;
gyro_data_typedef gyro_data;

uint8_t calibration_complete_flag = 0; // 0=not complete
										// 1=calibration complete.
										// 2=stop calibration
uint8_t calibration_count = 0;

uint16_t tilt = 0; // Will hold the average of Y-axis accelerometer reading.

void I2C0_init(void)
{

	I2CSPM_Init(&i2cspm_init_custom);

}

void turn_on_IMU(void)
{

	// Turn on sensor.
	// Boot time for FXOS8700 is 1000uS or 1ms. Since there is an enable I2C pins instruction below before we send
	// standby signal we need not wait for an additional 1ms.
	// Also there will be enough time for FXAS21002 to boot before standby signal is issued to it.


	// Enable SCL and SDA lines.
	GPIO_PinModeSet(I2C0_SCL_port, I2C0_SCL_pin, gpioModeWiredAndPullUp, true);
	GPIO_PinModeSet(I2C0_SDA_port, I2C0_SDA_pin, gpioModeWiredAndPullUp, true);

	// Enable interrupt
	NVIC_EnableIRQ(I2C0_IRQn);

	timerWaitUs(1000); // 5ms

}

void FXOS_send_standby_signal(void)
{

	seq.addr  = FXOS8700_ADDRESS << 1;
	seq.flags = I2C_FLAG_WRITE;

	write_data[0] = FXOS8700CQ_CTRL_REG1;
	write_data[1] = 0x00;
	seq.buf[0].data   = write_data;
	seq.buf[0].len    = 2;



	I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C0, &seq);

	if (ret < 0)
	{
		#if INCLUDE_LOGGING
			LOG_ERROR("ERROR: %d | While doing FXOS_send_standby_signal()", ret);
		#endif
	}


}

// write 0000 0001= 0x01 to XYZ_DATA_CFG register
// [7]: reserved
// [6]: reserved
// [5]: reserved
// [4]: hpf_out=0
// [3]: reserved
// [2]: reserved
// [1-0]: fs=01 for accelerometer range of +/-4g range with
// 0.488mg/LSB
void FXOS_DATA_CFG_start(void)
{

	seq.addr  = FXOS8700_ADDRESS << 1;
	seq.flags = I2C_FLAG_WRITE;

	write_data[0] = FXOS8700CQ_XYZ_DATA_CFG;
	write_data[1] = 0x01;
	seq.buf[0].data   = write_data;
	seq.buf[0].len    = 2;



	I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C0, &seq);

	if (ret < 0)
	{
		#if INCLUDE_LOGGING
			LOG_ERROR("ERROR: %d | While doing FXOS_send_standby_signal()", ret);
		#endif
	}


}

// write 0010 1101 = 0x2D to accelerometer control register 1
// [7-6]: aslp_rate=00
// [5-3]: dr=101 for 12.5Hz data rate (default accelerometer only mode)
// [2]: lnoise=1 for low noise mode
// [1]: f_read=0 for normal 16 bit reads
// [0]: active=1 to take the part out of standby and enable
void FXOS_CTRL_REG1_signal_start(void)
{

	seq.addr  = FXOS8700_ADDRESS << 1;
	seq.flags = I2C_FLAG_WRITE;

	write_data[0] = FXOS8700CQ_CTRL_REG1;
	write_data[1] = 0x2D;
	seq.buf[0].data   = write_data;
	seq.buf[0].len    = 2;



	I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C0, &seq);

	if (ret < 0)
	{
		#if INCLUDE_LOGGING
			LOG_ERROR("ERROR: %d | While doing FXOS_send_standby_signal()", ret);
		#endif
	}


}


void FXAS_standby_signal_send(void)
{

	seq.addr  = FXAS21002C_ADDRESS << 1;
	seq.flags = I2C_FLAG_WRITE;

	write_data[0] = FXAS21002C_REGISTER_CTRL_REG1;
	write_data[1] = 0x00;
	seq.buf[0].data   = write_data;
	seq.buf[0].len    = 2;



	I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C0, &seq);

	if (ret < 0)
	{
		#if INCLUDE_LOGGING
			LOG_ERROR("ERROR: %d | While doing FXOS_send_standby_signal()", ret);
		#endif
	}

}

void FXAS_reset_signal_send(void)
{

	seq.addr  = FXAS21002C_ADDRESS << 1;
	seq.flags = I2C_FLAG_WRITE;

	write_data[0] = FXAS21002C_REGISTER_CTRL_REG1;
	write_data[1] = (1 << 6);
	seq.buf[0].data   = write_data;
	seq.buf[0].len    = 2;



	I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C0, &seq);

	if (ret < 0)
	{
		#if INCLUDE_LOGGING
			LOG_ERROR("ERROR: %d | While doing FXOS_send_standby_signal()", ret);
		#endif
	}


}

/* Set CTRL_REG0 (0x0D)  Default value 0x00
=====================================================================
BIT  Symbol     Description                                   Default
7:6  BW         cut-off frequency of low-pass filter               00
  5  SPIW       SPI interface mode selection                        0
4:3  SEL        High-pass filter cutoff frequency selection        00
  2  HPF_EN     High-pass filter enable                             0
1:0  FS         Full-scale range selection
                00 = +-2000 dps
                01 = +-1000 dps
                10 = +-500 dps
                11 = +-250 dps
The bit fields in CTRL_REG0 should be changed only in Standby or Ready modes.
*/
void FXAS_CTRL_REG0_signal_start(void)
{

	seq.addr  = FXAS21002C_ADDRESS << 1;
	seq.flags = I2C_FLAG_WRITE;

	write_data[0] = FXAS21002C_REGISTER_CTRL_REG0;
	write_data[1] = 0x00;
	seq.buf[0].data   = write_data;
	seq.buf[0].len    = 2;



	I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C0, &seq);

	if (ret < 0)
	{
		#if INCLUDE_LOGGING
			LOG_ERROR("ERROR: %d | While doing FXOS_send_standby_signal()", ret);
		#endif
	}

}



/* Set CTRL_REG1 (0x1A)
  ====================================================================
  BIT  Symbol    Description                                   Default
  ---  ------    --------------------------------------------- -------
    6  RESET     Reset device on 1                                   0
    5  ST        Self test enabled on 1                              0
  4:2  DR        Output data rate                                  000
                 000 = 800 Hz
                 001 = 400 Hz
                 010 = 200 Hz
                 011 = 100 Hz
                 100 = 50 Hz
                 101 = 25 Hz
                 110 = 12.5 Hz
                 111 = 12.5 Hz
    1  ACTIVE    Standby(0)/Active(1)                                0
    0  READY     Standby(0)/Ready(1)                                 0
 */
void FXAS_CTRL_REG1_signal_start(void)
{

	seq.addr  = FXAS21002C_ADDRESS << 1;
	seq.flags = I2C_FLAG_WRITE;

	write_data[0] = FXAS21002C_REGISTER_CTRL_REG1;
	write_data[1] = 0x1A; // Active and ODR=12.5
	seq.buf[0].data   = write_data;
	seq.buf[0].len    = 2;



	I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C0, &seq);

	if (ret < 0)
	{
		#if INCLUDE_LOGGING
			LOG_ERROR("ERROR: %d | While doing FXOS_send_standby_signal()", ret);
		#endif
	}

}


void wait_for_65_millis(void)
{

	timerWaitUs(100000); // Wait for at least 65ms

}

// Read
void FXOS_measure_start(void)
{

	seq.addr  = FXOS8700_ADDRESS << 1;
	seq.flags = I2C_FLAG_READ;
	seq.buf[0].data   = read_data;
	seq.buf[0].len    = FXOS8700CQ_READ_LEN; //  status + X,Y,Z = 7 bytes.



	I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C0, &seq);

	if (ret < 0)
	{
		#if INCLUDE_LOGGING
			LOG_ERROR("ERROR: %d | While writing temperature measure command init", ret);
		#endif
	}


}

// Read
void FXAS_measure_start(void)
{
	// First move  read_data-acceleromter readings into a global variable.
	for (int i = 0; i < 7; i++ )
	{

		accel_buff[i] = read_data[i];

	}


	// Start reading gyro sensor readings here.
	seq.addr  = FXAS21002C_ADDRESS << 1;
	seq.flags = I2C_FLAG_READ;
	seq.buf[0].data   = read_data;
	seq.buf[0].len    = FXAS21002C_READ_LEN; //  status + X,Y,Z = 7 bytes.



	I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C0, &seq);

	if (ret < 0)
	{
		#if INCLUDE_LOGGING
			LOG_ERROR("ERROR: %d | While writing temperature measure command init", ret);
		#endif
	}




}


void FXAS_measure_stop_off_read(void)
{

	// This sequence will take FXOS and FXAS to low power standby mode.
	gpioIMUSensorEnSetOff();
	gpioIMUSensorEnSetOn();


	// Disable I2C0 interrupt
	NVIC_DisableIRQ(I2C0_IRQn);

	// Actions done in reverse.
	// Disable I2C pins
	GPIO_PinModeSet(I2C0_SCL_port, I2C0_SCL_pin, gpioModeDisabled, false);
	GPIO_PinModeSet(I2C0_SDA_port, I2C0_SDA_pin, gpioModeDisabled, false);


	// First move  read_data-gyroscope readings into a global variable.
	for (int i = 0; i < 7; i++ )
	{

		gyro_buff[i] = read_data[i];

	}




	// Convert to units from raw readings. Both accel and gyro readings.
	// Accel
	accel_raw.x = 0;
	accel_raw.y = 0;
	accel_raw.z = 0;


	if (accel_buff[0] < 0) // Checks if data was over-written by reading or not
	{
		// If overwritten, do nothing
		accel_raw.x = 0;
		accel_raw.y = 0;
		accel_raw.z = 0;
	}
	else
	{

	    // copy the 14 bit accelerometer byte data into 16 bit words
	    accel_raw.x = (int16_t)(((accel_buff[1] << 8) | accel_buff[2]))>> 2;
	    accel_raw.y = (int16_t)(((accel_buff[3] << 8) | accel_buff[4]))>> 2;
	    accel_raw.z = (int16_t)(((accel_buff[5] << 8) | accel_buff[6]))>> 2;



	    // +/- 4g selected during FXOS8700 configuration
	    // Conversion from raw readings to milliGs
	    accel_data.x = accel_raw.x * ACCEL_MG_LSB_4G;
	    accel_data.y = accel_raw.y * ACCEL_MG_LSB_4G;
	    accel_data.z = accel_raw.z * ACCEL_MG_LSB_4G;


	}

	// Gyro
	gyro_raw.x = 0;
	gyro_raw.y = 0;
	gyro_raw.z = 0;


	if (gyro_buff[0] < 0) // Checks if data was over-written by reading or not
	{
		// If overwritten, do nothing
		gyro_raw.x = 0;
		gyro_raw.y = 0;
		gyro_raw.z = 0;
	}
	else
	{


	    // Copy the gyroscope byte data into 16 bit words
	    gyro_raw.x = (gyro_buff[1] << 8) | gyro_buff[2];
	    gyro_raw.y = (gyro_buff[3] << 8) | gyro_buff[4];
	    gyro_raw.z = (gyro_buff[5] << 8) | gyro_buff[6];


	    // Conversion from raw readings to millidps
	    gyro_data.x = gyro_raw.x * GYRO_SENSITIVITY_2000DPS;
	    gyro_data.y = gyro_raw.y * GYRO_SENSITIVITY_2000DPS;
	    gyro_data.z = gyro_raw.z * GYRO_SENSITIVITY_2000DPS;

	    // Conversion from dps to millrad/s
	    gyro_data.x *= FXAS21002C_MDPS_TO_MRAD;
	    gyro_data.y *= FXAS21002C_MDPS_TO_MRAD;
	    gyro_data.z *= FXAS21002C_MDPS_TO_MRAD;

	}

	#if INCLUDE_LOGGING
		{
			LOG_INFO("Accelerometer::: X: %d Y: %d Z: %d", accel_data.x, accel_data.y, accel_data.z);
		}

		{
			LOG_INFO("Gyroscope::: X: %d Y: %d Z: %d", gyro_data.x, gyro_data.y, gyro_data.z);
		}
	#endif

	// While calibration the person must be still. Checked using gyroscope.
	// If person moves start calibration from beginning.
	if (calibration_complete_flag == 0 && (abs(gyro_data.x) > 20 || abs(gyro_data.y) > 20 || abs(gyro_data.z) > 20))
	{
		calibration_count = 0;
		tilt = 0;
	}


	if (calibration_complete_flag == 0)
	{

		calibration_count++;
	}


	// Ignore first 4 readings. Let the sensor stabilize.
	if (calibration_count == 4)
	{
		calibration_complete_flag = 1; // Start sending indications only when calibration_complete_flag = 1
		calibration_count++;

	}



	timerWaitUs(1000); // 1ms




}

#endif
