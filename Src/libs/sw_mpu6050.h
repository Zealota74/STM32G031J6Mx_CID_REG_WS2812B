/*
 * sw_mpu6050.h
 *
 *  Created on: Jul 27, 2021
 *      Author: szymon.wroblewski
 */

#ifndef LIBS_SW_MPU6050_H_
#define LIBS_SW_MPU6050_H_

/*
 MPU6050.h - Header file for the MPU6050 Triple Axis Gyroscope & Accelerometer Arduino Library.
 Version: 1.0.3
 (c) 2014-2015 Korneliusz Jarzebski
 www.jarzebski.pl
 This program is free software: you can redistribute it and/or modify
 it under the terms of the version 3 GNU General Public License as
 published by the Free Software Foundation.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define MPU6050_ADDRESS               (0x68) // 0x69 when AD0 pin to Vcc

#define MPU6050_REG_ACCEL_XOFFS_H     (0x06)
#define MPU6050_REG_ACCEL_XOFFS_L     (0x07)
#define MPU6050_REG_ACCEL_YOFFS_H     (0x08)
#define MPU6050_REG_ACCEL_YOFFS_L     (0x09)
#define MPU6050_REG_ACCEL_ZOFFS_H     (0x0A)
#define MPU6050_REG_ACCEL_ZOFFS_L     (0x0B)
#define MPU6050_REG_GYRO_XOFFS_H      (0x13)
#define MPU6050_REG_GYRO_XOFFS_L      (0x14)
#define MPU6050_REG_GYRO_YOFFS_H      (0x15)
#define MPU6050_REG_GYRO_YOFFS_L      (0x16)
#define MPU6050_REG_GYRO_ZOFFS_H      (0x17)
#define MPU6050_REG_GYRO_ZOFFS_L      (0x18)
#define MPU6050_REG_CONFIG            (0x1A)
#define MPU6050_REG_GYRO_CONFIG       (0x1B) // Gyroscope Configuration
#define MPU6050_REG_ACCEL_CONFIG      (0x1C) // Accelerometer Configuration
#define MPU6050_REG_FF_THRESHOLD      (0x1D)
#define MPU6050_REG_FF_DURATION       (0x1E)
#define MPU6050_REG_MOT_THRESHOLD     (0x1F)
#define MPU6050_REG_MOT_DURATION      (0x20)
#define MPU6050_REG_ZMOT_THRESHOLD    (0x21)
#define MPU6050_REG_ZMOT_DURATION     (0x22)
#define MPU6050_REG_INT_PIN_CFG       (0x37) // INT Pin. Bypass Enable Configuration
#define MPU6050_REG_INT_ENABLE        (0x38) // INT Enable
#define MPU6050_REG_INT_STATUS        (0x3A)
#define MPU6050_REG_ACCEL_XOUT_H      (0x3B)
#define MPU6050_REG_ACCEL_XOUT_L      (0x3C)
#define MPU6050_REG_ACCEL_YOUT_H      (0x3D)
#define MPU6050_REG_ACCEL_YOUT_L      (0x3E)
#define MPU6050_REG_ACCEL_ZOUT_H      (0x3F)
#define MPU6050_REG_ACCEL_ZOUT_L      (0x40)
#define MPU6050_REG_TEMP_OUT_H        (0x41)
#define MPU6050_REG_TEMP_OUT_L        (0x42)
#define MPU6050_REG_GYRO_XOUT_H       (0x43)
#define MPU6050_REG_GYRO_XOUT_L       (0x44)
#define MPU6050_REG_GYRO_YOUT_H       (0x45)
#define MPU6050_REG_GYRO_YOUT_L       (0x46)
#define MPU6050_REG_GYRO_ZOUT_H       (0x47)
#define MPU6050_REG_GYRO_ZOUT_L       (0x48)
#define MPU6050_REG_MOT_DETECT_STATUS (0x61)
#define MPU6050_REG_MOT_DETECT_CTRL   (0x69)
#define MPU6050_REG_USER_CTRL         (0x6A) // User Control
#define MPU6050_REG_PWR_MGMT_1        (0x6B) // Power Management 1
#define MPU6050_REG_WHO_AM_I          (0x75) // Who Am I

//#ifndef VECTOR_STRUCT_H
//#define VECTOR_STRUCT_H
struct Vector {
	float XAxis;
	float YAxis;
	float ZAxis;
};
//#endif

struct Activites {
	bool isOverflow;
	bool isFreeFall;
	bool isInactivity;
	bool isActivity;
	bool isPosActivityOnX;
	bool isPosActivityOnY;
	bool isPosActivityOnZ;
	bool isNegActivityOnX;
	bool isNegActivityOnY;
	bool isNegActivityOnZ;
	bool isDataReady;
};

typedef enum {
	MPU6050_CLOCK_KEEP_RESET = 0b111,
	MPU6050_CLOCK_EXTERNAL_19MHZ = 0b101,
	MPU6050_CLOCK_EXTERNAL_32KHZ = 0b100,
	MPU6050_CLOCK_PLL_ZGYRO = 0b011,
	MPU6050_CLOCK_PLL_YGYRO = 0b010,
	MPU6050_CLOCK_PLL_XGYRO = 0b001,
	MPU6050_CLOCK_INTERNAL_8MHZ = 0b000
} mpu6050_clockSource_t;

typedef enum {
	MPU6050_SCALE_2000DPS = 0b11,
	MPU6050_SCALE_1000DPS = 0b10,
	MPU6050_SCALE_500DPS = 0b01,
	MPU6050_SCALE_250DPS = 0b00
} mpu6050_dps_t;

typedef enum {
	MPU6050_RANGE_16G = 0b11,
	MPU6050_RANGE_8G = 0b10,
	MPU6050_RANGE_4G = 0b01,
	MPU6050_RANGE_2G = 0b00,
} mpu6050_range_t;

typedef enum {
	MPU6050_DELAY_3MS = 0b11,
	MPU6050_DELAY_2MS = 0b10,
	MPU6050_DELAY_1MS = 0b01,
	MPU6050_NO_DELAY = 0b00,
} mpu6050_onDelay_t;

typedef enum {
	MPU6050_DHPF_HOLD = 0b111,
	MPU6050_DHPF_0_63HZ = 0b100,
	MPU6050_DHPF_1_25HZ = 0b011,
	MPU6050_DHPF_2_5HZ = 0b010,
	MPU6050_DHPF_5HZ = 0b001,
	MPU6050_DHPF_RESET = 0b000,
} mpu6050_dhpf_t;

typedef enum {
	MPU6050_DLPF_6 = 0b110,
	MPU6050_DLPF_5 = 0b101,
	MPU6050_DLPF_4 = 0b100,
	MPU6050_DLPF_3 = 0b011,
	MPU6050_DLPF_2 = 0b010,
	MPU6050_DLPF_1 = 0b001,
	MPU6050_DLPF_0 = 0b000,
} mpu6050_dlpf_t;

//class MPU6050
//{
//    public:

//	bool MPU6050__begin(mpu6050_dps_t scale = MPU6050_SCALE_2000DPS, mpu6050_range_t range = MPU6050_RANGE_2G, int mpua = MPU6050_ADDRESS);

extern void MPU6050__setClockSource(mpu6050_clockSource_t source);
extern void MPU6050__setScale(mpu6050_dps_t scale);
extern void MPU6050__setRange(mpu6050_range_t range);
extern mpu6050_clockSource_t MPU6050__getClockSource(void);
extern mpu6050_dps_t MPU6050__getScale(void);
extern mpu6050_range_t MPU6050__getRange(void);
extern void MPU6050__setDHPFMode(mpu6050_dhpf_t dhpf);
extern void MPU6050__setDLPFMode(mpu6050_dlpf_t dlpf);
extern mpu6050_onDelay_t getAccelPowerOnDelay();
extern void MPU6050__setAccelPowerOnDelay(mpu6050_onDelay_t delay);

extern uint8_t MPU6050__getIntStatus(void);

extern bool MPU6050__getIntZeroMotionEnabled(void);
extern void MPU6050__setIntZeroMotionEnabled(bool state);
extern bool MPU6050__getIntMotionEnabled(void);
extern void MPU6050__setIntMotionEnabled(bool state);
extern bool MPU6050__getIntFreeFallEnabled(void);
extern void MPU6050__setIntFreeFallEnabled(bool state);

extern uint8_t MPU6050__getMotionDetectionThreshold(void);
extern void MPU6050__setMotionDetectionThreshold(uint8_t threshold);
extern uint8_t MPU6050__getMotionDetectionDuration(void);
extern void MPU6050__setMotionDetectionDuration(uint8_t duration);

extern uint8_t MPU6050__getZeroMotionDetectionThreshold(void);
extern void MPU6050__setZeroMotionDetectionThreshold(uint8_t threshold);
extern uint8_t MPU6050__getZeroMotionDetectionDuration(void);
extern void MPU6050__setZeroMotionDetectionDuration(uint8_t duration);

extern uint8_t MPU6050__getFreeFallDetectionThreshold(void);
extern void MPU6050__setFreeFallDetectionThreshold(uint8_t threshold);
extern uint8_t MPU6050__getFreeFallDetectionDuration(void);
extern void MPU6050__setFreeFallDetectionDuration(uint8_t duration);

extern bool MPU6050__getSleepEnabled(void);
extern void MPU6050__setSleepEnabled(bool state);
extern bool MPU6050__getI2CMasterModeEnabled(void);
extern void MPU6050__setI2CMasterModeEnabled(bool state);
extern bool MPU6050__getI2CBypassEnabled(void);
extern void MPU6050__setI2CBypassEnabled(bool state);

extern float MPU6050__readTemperature(void);
extern struct Activites readActivites(void);

extern int16_t MPU6050__getGyroOffsetX(void);
extern void MPU6050__setGyroOffsetX(int16_t offset);
extern int16_t MPU6050__getGyroOffsetY(void);
void MPU6050__setGyroOffsetY(int16_t offset);
extern int16_t MPU6050__getGyroOffsetZ(void);
extern void MPU6050__setGyroOffsetZ(int16_t offset);

extern int16_t MPU6050__getAccelOffsetX(void);
extern void MPU6050__setAccelOffsetX(int16_t offset);
extern int16_t MPU6050__getAccelOffsetY(void);
extern void MPU6050__setAccelOffsetY(int16_t offset);
extern int16_t MPU6050__getAccelOffsetZ(void);
extern void MPU6050__setAccelOffsetZ(int16_t offset);

void MPU6050__calibrateGyro(uint8_t samples);
void MPU6050__setThreshold(uint8_t multiple);
//void MPU6050__calibrateGyro(uint8_t MPU6050__samples = 50);
//void MPU6050__setThreshold(uint8_t MPU6050__multiple = 1);
extern uint8_t MPU6050__getThreshold(void);

extern struct Vector MPU6050__readRawGyro(void);
extern struct Vector MPU6050__readNormalizeGyro(void);

extern struct Vector MPU6050__readRawAccel(void);
extern struct Vector MPU6050__readNormalizeAccel(void);
extern struct Vector MPU6050__readScaledAccel(void);

#endif /* LIBS_SW_MPU6050_H_ */
