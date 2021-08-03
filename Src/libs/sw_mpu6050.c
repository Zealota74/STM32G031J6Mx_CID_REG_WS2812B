/*
 * sw_MPU6050.c
 *
 *  Created on: Jul 27, 2021
 *      Author: szymon.wroblewski
 */
#include "sw_mcu_conf.h"
#include "SW_BOARD/gpio.h"
#include "SW_TIMERS/sw_soft_timers.h"
#include "sw_i2c_simple_v2.h"

#include <math.h>
#include <stdlib.h>

#include "sw_mpu6050.h"

static struct	Vector ra, rg; 	// Raw vectors
static struct 	Vector na, ng; 	// Normalized vectors
static struct 	Vector tg, dg; 	// Threshold and Delta for Gyro
static struct 	Vector th;     	// Threshold
static struct 	Activites a;			// Activities

static float	dpsPerDigit, rangePerDigit;
static float	actualThreshold;
static bool		useCalibrate;

/***************************** Static functions ****************************/
// Fast read 8-bit from register
static uint8_t MPU6050__fastRegister8( uint8_t reg ) {
	uint8_t value = 0;
	sw_i2c_read_reg( MPU6050_ADDRESS, reg, &value );
	return value;
}

// Read 8-bit from register
static uint8_t MPU6050__readRegister8( uint8_t reg ) {
	uint8_t value = 0;
	sw_i2c_read_reg( MPU6050_ADDRESS, reg, &value );
	return value;
}

// Write 8-bit to register
static void MPU6050__writeRegister8( uint8_t reg, uint8_t value ) {
	sw_i2c_write_reg( MPU6050_ADDRESS, reg, value );
}

static int16_t MPU6050__readRegister16( uint8_t reg ) {
	int16_t value = 0;

	uint8_t buff[2];
	sw_i2c_read_bulk( MPU6050_ADDRESS, reg, 2, buff );
	uint8_t vha = buff[0];
	uint8_t vla = buff[1];
	value = vha << 8 | vla;
	return value;
}

static void MPU6050__writeRegister16( uint8_t reg, int16_t value ) {
	uint8_t buff[2] = { ((uint8_t)value >> 8), (uint8_t)value };
	sw_i2c_write_bulk( MPU6050_ADDRESS, reg, 2, buff );
}

// Read register bit
static bool MPU6050__readRegisterBit( uint8_t reg, uint8_t pos ) {
	uint8_t value;
	value = MPU6050__readRegister8(reg);
	return ((value >> pos) & 1);
}

// Write register bit
static void MPU6050__writeRegisterBit( uint8_t reg, uint8_t pos, bool state ) {
	uint8_t value;
	value = MPU6050__readRegister8(reg);

	if (state) {
		value |= (1 << pos);
	} else {
		value &= ~(1 << pos);
	}
	MPU6050__writeRegister8(reg, value );
}
/****************************************************************************/

// Set threshold value
void MPU6050__setThreshold( uint8_t multiple ) {
	if (multiple > 0) {
		// If not calibrated, need calibrate
		if ( !useCalibrate ) {
			MPU6050__calibrateGyro( 50 );
		}
		// Calculate threshold vectors
		tg.XAxis = th.XAxis * multiple;
		tg.YAxis = th.YAxis * multiple;
		tg.ZAxis = th.ZAxis * multiple;
	} else {
		// No threshold
		tg.XAxis = 0;
		tg.YAxis = 0;
		tg.ZAxis = 0;
	}
	// Remember old threshold value
	actualThreshold = multiple;
}

// Calibrate algorithm
void MPU6050__calibrateGyro( uint8_t samples ) {
	useCalibrate = true;	// Set calibrate
	// Reset values
	float sumX = 0;
	float sumY = 0;
	float sumZ = 0;
	float sigmaX = 0;
	float sigmaY = 0;
	float sigmaZ = 0;
	// Read n-samples
	for ( uint8_t i = 0; i < samples; ++i) {
		MPU6050__readRawGyro();
		sumX += rg.XAxis;
		sumY += rg.YAxis;
		sumZ += rg.ZAxis;

		sigmaX += rg.XAxis * rg.XAxis;
		sigmaY += rg.YAxis * rg.YAxis;
		sigmaZ += rg.ZAxis * rg.ZAxis;
		delay_ms(5);
	}
	// Calculate delta vectors
	dg.XAxis = sumX / samples;
	dg.YAxis = sumY / samples;
	dg.ZAxis = sumZ / samples;
	// Calculate threshold vectors
	th.XAxis = sqrt((sigmaX / 50) - (dg.XAxis * dg.XAxis));
	th.YAxis = sqrt((sigmaY / 50) - (dg.YAxis * dg.YAxis));
	th.ZAxis = sqrt((sigmaZ / 50) - (dg.ZAxis * dg.ZAxis));
	// If already set threshold, recalculate threshold vectors
	if ( actualThreshold > 0 ) {
		MPU6050__setThreshold( actualThreshold );
	}
}


bool MPU6050__begin( mpu6050_dps_t scale, mpu6050_range_t range, int mpu ) {
	// Reset calibrate values
	dg.XAxis = 0;
	dg.YAxis = 0;
	dg.ZAxis = 0;
	useCalibrate = false;
	// Reset threshold values
	tg.XAxis = 0;
	tg.YAxis = 0;
	tg.ZAxis = 0;
	actualThreshold = 0;
	// Check MPU6050 Who Am I Register
	if ( MPU6050__fastRegister8( MPU6050_REG_WHO_AM_I) != 0x68 ) {
		return false;
	}
	// Set Clock Source
	MPU6050__setClockSource( MPU6050_CLOCK_PLL_XGYRO );
	// Set Scale & Range
	MPU6050__setScale( scale );
	MPU6050__setRange( range );
	// Disable Sleep Mode
	MPU6050__setSleepEnabled( false );

	return true;
}

void MPU6050__setScale( mpu6050_dps_t scale ) {
	uint8_t value;

	switch (scale) {
	case MPU6050_SCALE_250DPS:
		dpsPerDigit = .007633f;
		break;
	case MPU6050_SCALE_500DPS:
		dpsPerDigit = .015267f;
		break;
	case MPU6050_SCALE_1000DPS:
		dpsPerDigit = .030487f;
		break;
	case MPU6050_SCALE_2000DPS:
		dpsPerDigit = .060975f;
		break;
	default:
		break;
	}
	value = MPU6050__readRegister8( MPU6050_REG_GYRO_CONFIG);
	value &= 0b11100111;
	value |= (scale << 3);
	MPU6050__writeRegister8( MPU6050_REG_GYRO_CONFIG, value );
}

mpu6050_dps_t MPU6050__getScale(void) {
	uint8_t value;
	value = MPU6050__readRegister8( MPU6050_REG_GYRO_CONFIG);
	value &= 0b00011000;
	value >>= 3;
	return ( mpu6050_dps_t) value;
}

void MPU6050__setRange( mpu6050_range_t range) {
	uint8_t value;

	switch (range) {
	case MPU6050_RANGE_2G:
		rangePerDigit = .000061f;
		break;
	case MPU6050_RANGE_4G:
		rangePerDigit = .000122f;
		break;
	case MPU6050_RANGE_8G:
		rangePerDigit = .000244f;
		break;
	case MPU6050_RANGE_16G:
		rangePerDigit = .0004882f;
		break;
	default:
		break;
	}

	value = MPU6050__readRegister8( MPU6050_REG_ACCEL_CONFIG);
	value &= 0b11100111;
	value |= (range << 3);
	MPU6050__writeRegister8( MPU6050_REG_ACCEL_CONFIG, value );
}

mpu6050_range_t MPU6050__getRange(void) {
	uint8_t value;
	value = MPU6050__readRegister8( MPU6050_REG_ACCEL_CONFIG);
	value &= 0b00011000;
	value >>= 3;
	return ( mpu6050_range_t) value;
}

void MPU6050__setDHPFMode( mpu6050_dhpf_t dhpf) {
	uint8_t value;
	value = MPU6050__readRegister8( MPU6050_REG_ACCEL_CONFIG);
	value &= 0b11111000;
	value |= dhpf;
	MPU6050__writeRegister8( MPU6050_REG_ACCEL_CONFIG, value );
}

void MPU6050__setDLPFMode( mpu6050_dlpf_t dlpf) {
	uint8_t value;
	value = MPU6050__readRegister8( MPU6050_REG_CONFIG);
	value &= 0b11111000;
	value |= dlpf;
	MPU6050__writeRegister8( MPU6050_REG_CONFIG, value );
}

void MPU6050__setClockSource( mpu6050_clockSource_t source) {
	uint8_t value;
	value = MPU6050__readRegister8( MPU6050_REG_PWR_MGMT_1);
	value &= 0b11111000;
	value |= source;
	MPU6050__writeRegister8( MPU6050_REG_PWR_MGMT_1, value );
}

mpu6050_clockSource_t MPU6050__getClockSource(void) {
	uint8_t value;
	value = MPU6050__readRegister8( MPU6050_REG_PWR_MGMT_1);
	value &= 0b00000111;
	return ( mpu6050_clockSource_t) value;
}

bool MPU6050__getSleepEnabled(void) {
	return MPU6050__readRegisterBit( MPU6050_REG_PWR_MGMT_1, 6);
}

void MPU6050__setSleepEnabled( bool state) {
	MPU6050__writeRegisterBit( MPU6050_REG_PWR_MGMT_1, 6, state);
}

bool MPU6050__getIntZeroMotionEnabled(void) {
	return MPU6050__readRegisterBit( MPU6050_REG_INT_ENABLE, 5);
}

void MPU6050__setIntZeroMotionEnabled( bool state) {
	MPU6050__writeRegisterBit( MPU6050_REG_INT_ENABLE, 5, state);
}

bool MPU6050__getIntMotionEnabled(void) {
	return MPU6050__readRegisterBit( MPU6050_REG_INT_ENABLE, 6);
}

void MPU6050__setIntMotionEnabled( bool state) {
	MPU6050__writeRegisterBit( MPU6050_REG_INT_ENABLE, 6, state);
}

bool MPU6050__getIntFreeFallEnabled(void) {
	return MPU6050__readRegisterBit( MPU6050_REG_INT_ENABLE, 7);
}

void MPU6050__setIntFreeFallEnabled( bool state) {
	MPU6050__writeRegisterBit( MPU6050_REG_INT_ENABLE, 7, state);
}

uint8_t MPU6050__getMotionDetectionThreshold(void) {
	return MPU6050__readRegister8( MPU6050_REG_MOT_THRESHOLD);
}

void MPU6050__setMotionDetectionThreshold( uint8_t threshold) {
	MPU6050__writeRegister8( MPU6050_REG_MOT_THRESHOLD, threshold);
}

uint8_t MPU6050__getMotionDetectionDuration(void) {
	return MPU6050__readRegister8( MPU6050_REG_MOT_DURATION);
}

void MPU6050__setMotionDetectionDuration( uint8_t duration) {
	MPU6050__writeRegister8( MPU6050_REG_MOT_DURATION, duration);
}

uint8_t MPU6050__getZeroMotionDetectionThreshold(void) {
	return MPU6050__readRegister8( MPU6050_REG_ZMOT_THRESHOLD);
}

void MPU6050__setZeroMotionDetectionThreshold( uint8_t threshold) {
	MPU6050__writeRegister8( MPU6050_REG_ZMOT_THRESHOLD, threshold);
}

uint8_t MPU6050__getZeroMotionDetectionDuration(void) {
	return MPU6050__readRegister8( MPU6050_REG_ZMOT_DURATION);
}

void MPU6050__setZeroMotionDetectionDuration( uint8_t duration) {
	MPU6050__writeRegister8( MPU6050_REG_ZMOT_DURATION, duration);
}

uint8_t MPU6050__getFreeFallDetectionThreshold(void) {
	return MPU6050__readRegister8( MPU6050_REG_FF_THRESHOLD);
}

void MPU6050__setFreeFallDetectionThreshold( uint8_t threshold) {
	MPU6050__writeRegister8( MPU6050_REG_FF_THRESHOLD, threshold);
}

uint8_t MPU6050__getFreeFallDetectionDuration(void) {
	return MPU6050__readRegister8( MPU6050_REG_FF_DURATION);
}

void MPU6050__setFreeFallDetectionDuration( uint8_t duration) {
	MPU6050__writeRegister8( MPU6050_REG_FF_DURATION, duration);
}

bool MPU6050__getI2CMasterModeEnabled(void) {
	return MPU6050__readRegisterBit( MPU6050_REG_USER_CTRL, 5);
}

void MPU6050__setI2CMasterModeEnabled( bool state) {
	MPU6050__writeRegisterBit( MPU6050_REG_USER_CTRL, 5, state);
}

void MPU6050__setI2CBypassEnabled( bool state) {
	return MPU6050__writeRegisterBit( MPU6050_REG_INT_PIN_CFG, 1, state);
}

bool MPU6050__getI2CBypassEnabled(void) {
	return MPU6050__readRegisterBit( MPU6050_REG_INT_PIN_CFG, 1);
}

void MPU6050__setAccelPowerOnDelay( mpu6050_onDelay_t delay_ms) {
	uint8_t value;
	value = MPU6050__readRegister8( MPU6050_REG_MOT_DETECT_CTRL );
	value &= 0b11001111;
	value |= (delay_ms << 4);
	MPU6050__writeRegister8( MPU6050_REG_MOT_DETECT_CTRL, value );
}

mpu6050_onDelay_t MPU6050__getAccelPowerOnDelay(void) {
	uint8_t value;
	value = MPU6050__readRegister8( MPU6050_REG_MOT_DETECT_CTRL);
	value &= 0b00110000;
	return ( mpu6050_onDelay_t) (value >> 4);
}

uint8_t MPU6050__getIntStatus(void) {
	return MPU6050__readRegister8( MPU6050_REG_INT_STATUS);
}

struct Activites MPU6050__readActivites(void) {
	uint8_t data = MPU6050__readRegister8( MPU6050_REG_INT_STATUS);

	a.isOverflow = ((data >> 4) & 1);
	a.isFreeFall = ((data >> 7) & 1);
	a.isInactivity = ((data >> 5) & 1);
	a.isActivity = ((data >> 6) & 1);
	a.isDataReady = ((data >> 0) & 1);

	data = MPU6050__readRegister8( MPU6050_REG_MOT_DETECT_STATUS);

	a.isNegActivityOnX = ((data >> 7) & 1);
	a.isPosActivityOnX = ((data >> 6) & 1);

	a.isNegActivityOnY = ((data >> 5) & 1);
	a.isPosActivityOnY = ((data >> 4) & 1);

	a.isNegActivityOnZ = ((data >> 3) & 1);
	a.isPosActivityOnZ = ((data >> 2) & 1);

	return a;
}

struct Vector MPU6050__MPU6050__readRawAccel(void) {

//	Wire.beginTransmission( MPU6050_ADDRESS);


//#if ARDUINO >= 100
//	Wire.write( MPU6050_REG_ACCEL_XOUT_H);
//    #else
//	Wire.send( MPU6050_REG_ACCEL_XOUT_H);
//#endif
//	Wire.endTransmission();
//
//	Wire.beginTransmission( MPU6050_ADDRESS);
//	Wire.requestFrom( MPU6050_ADDRESS, 6);
//
//	while (Wire.available() < 6);

//#if ARDUINO >= 100
//	uint8_t xha = Wire.read();
//	uint8_t xla = Wire.read();
//        uint8_t yha = Wire.read();
//	uint8_t yla = Wire.read();
//	uint8_t zha = Wire.read();
//	uint8_t zla = Wire.read();
//    #else
//	uint8_t xha = Wire.receive();
//	uint8_t xla = Wire.receive();
//	uint8_t yha = Wire.receive();
//	uint8_t yla = Wire.receive();
//	uint8_t zha = Wire.receive();
//	uint8_t zla = Wire.receive();
//#endif

//	ra.XAxis = xha << 8 | xla;
//	ra.YAxis = yha << 8 | yla;
//	ra.ZAxis = zha << 8 | zla;

	return ra;
}

struct Vector MPU6050__readNormalizeAccel(void) {
	MPU6050__readRawAccel();

	na.XAxis = ra.XAxis * rangePerDigit * 9.80665f;
	na.YAxis = ra.YAxis * rangePerDigit * 9.80665f;
	na.ZAxis = ra.ZAxis * rangePerDigit * 9.80665f;

	return na;
}

struct Vector MPU6050__readScaledAccel(void) {
	MPU6050__readRawAccel();

	na.XAxis = ra.XAxis * rangePerDigit;
	na.YAxis = ra.YAxis * rangePerDigit;
	na.ZAxis = ra.ZAxis * rangePerDigit;

	return na;
}

struct Vector MPU6050__readRawGyro(void) {
//	Wire.beginTransmission( MPU6050_ADDRESS );
//	Wire.write( MPU6050_REG_GYRO_XOUT_H );
//	Wire.endTransmission();
//
//	Wire.beginTransmission( MPU6050_ADDRESS );
//	Wire.requestFrom( MPU6050_ADDRESS, 6);
//
//	while (Wire.available() < 6) ;
//
//	uint8_t xha = Wire.read();
//	uint8_t xla = Wire.read();
//	uint8_t yha = Wire.read();
//	uint8_t yla = Wire.read();
//	uint8_t zha = Wire.read();
//	uint8_t zla = Wire.read();
//
//	rg.XAxis = xha << 8 | xla;
//	rg.YAxis = yha << 8 | yla;
//	rg.ZAxis = zha << 8 | zla;

	uint8_t buff[6];
	sw_i2c_read_bulk( MPU6050_ADDRESS, MPU6050_REG_GYRO_XOUT_H, 6, buff );
	uint8_t xha = buff[0];
	uint8_t xla = buff[1];
	uint8_t yha = buff[2];
	uint8_t yla = buff[3];
	uint8_t zha = buff[4];
	uint8_t zla = buff[5];

	rg.XAxis = xha << 8 | xla;
	rg.YAxis = yha << 8 | yla;
	rg.ZAxis = zha << 8 | zla;

	return rg;
}

struct Vector MPU6050__readNormalizeGyro(void) {
	MPU6050__readRawGyro();

	if (useCalibrate) {
		ng.XAxis = (rg.XAxis - dg.XAxis) * dpsPerDigit;
		ng.YAxis = (rg.YAxis - dg.YAxis) * dpsPerDigit;
		ng.ZAxis = (rg.ZAxis - dg.ZAxis) * dpsPerDigit;
	} else {
		ng.XAxis = rg.XAxis * dpsPerDigit;
		ng.YAxis = rg.YAxis * dpsPerDigit;
		ng.ZAxis = rg.ZAxis * dpsPerDigit;
	}

	if (actualThreshold) {
		if (abs(ng.XAxis) < tg.XAxis)
			ng.XAxis = 0;
		if (abs(ng.YAxis) < tg.YAxis)
			ng.YAxis = 0;
		if (abs(ng.ZAxis) < tg.ZAxis)
			ng.ZAxis = 0;
	}

	return ng;
}

float MPU6050__readTemperature(void) {
	int16_t T;
	T = MPU6050__readRegister16( MPU6050_REG_TEMP_OUT_H );
	return (float) T / 340 + 36.53;
}

int16_t MPU6050__getGyroOffsetX(void) {
	return MPU6050__readRegister16( MPU6050_REG_GYRO_XOFFS_H );
}

int16_t MPU6050__getGyroOffsetY(void) {
	return MPU6050__readRegister16( MPU6050_REG_GYRO_YOFFS_H );
}

int16_t MPU6050__getGyroOffsetZ(void) {
	return MPU6050__readRegister16( MPU6050_REG_GYRO_ZOFFS_H );
}

void MPU6050__setGyroOffsetX(int16_t offset ) {
	MPU6050__writeRegister16( MPU6050_REG_GYRO_XOFFS_H, offset );
}

void MPU6050__setGyroOffsetY(int16_t offset ) {
	MPU6050__writeRegister16( MPU6050_REG_GYRO_YOFFS_H, offset );
}

void MPU6050__setGyroOffsetZ(int16_t offset ) {
	MPU6050__writeRegister16( MPU6050_REG_GYRO_ZOFFS_H, offset );
}

int16_t MPU6050__getAccelOffsetX(void) {
	return MPU6050__readRegister16( MPU6050_REG_ACCEL_XOFFS_H );
}

int16_t MPU6050__getAccelOffsetY(void) {
	return MPU6050__readRegister16( MPU6050_REG_ACCEL_YOFFS_H );
}

int16_t MPU6050__getAccelOffsetZ(void) {
	return MPU6050__readRegister16( MPU6050_REG_ACCEL_ZOFFS_H );
}

void MPU6050__setAccelOffsetX(int16_t offset ) {
	MPU6050__writeRegister16( MPU6050_REG_ACCEL_XOFFS_H, offset );
}

void MPU6050__setAccelOffsetY(int16_t offset ) {
	MPU6050__writeRegister16( MPU6050_REG_ACCEL_YOFFS_H, offset );
}

void MPU6050__setAccelOffsetZ(int16_t offset ) {
	MPU6050__writeRegister16( MPU6050_REG_ACCEL_ZOFFS_H, offset );
}

// Get current threshold value
uint8_t MPU6050__getThreshold(void) {
	return actualThreshold;
}

