/*
 * sw_i2c_simple.c
 *
 *  Created on: Jul 17, 2021
 *      Author: simon
 */
#include "sw_mcu_conf.h"

#include "SW_BOARD/gpio.h"
#include "SW_BOARD/sw_led_blink_debug.h"
#include "SW_TIMERS/sw_soft_timers.h"
#include "sw_i2c_simple_v2.h"



int zmienna = 12;

void zmienna_init2(void) {
	zmienna = 7;
}


//STRUCT_t simon;


static INLINE void sw_i2c_autoend_on(void)  { hI2Cx->I2C->CR2 |=  I2C_CR2_AUTOEND; }
static INLINE void sw_i2c_autoend_off(void) { hI2Cx->I2C->CR2 &= ~I2C_CR2_AUTOEND; }
static INLINE void sw_i2c_nBytes( uint8_t nBytes ) {
	MODIFY_REG( I2C1->CR2, I2C_CR2_NBYTES, nBytes << I2C_CR2_NBYTES_Pos );
}
static INLINE void sw_i2c_read_dir (void) { hI2Cx->I2C->CR2 |= I2C_CR2_RD_WRN; }
static INLINE void sw_i2c_write_dir(void) { hI2Cx->I2C->CR2 &= ~I2C_CR2_RD_WRN; }

static INLINE I2CSTATUS sw_i2c_isTXIS_error(void) {
	whileTimer = 2;
	while( sw_is_TXIS_flag_ready() == false ) {
		if ( whileTimer == 0 && sw_is_NACK_flag_ready() ) {
			return I2C_Nack;
		}
	}	// All bytes send nBytes = 1
	return I2C_Ok;
}
static INLINE I2CSTATUS sw_i2c_isTC_error(void) {
	whileTimer = 2;
	while( sw_is_TC_flag_ready() == false ) {
		if ( whileTimer == 0 && sw_is_NACK_flag_ready() ) {
			return I2C_Nack;
		}
	}	// All bytes send nBytes = 1
	return I2C_Ok;
}

static I2CSTATUS sw_i2c_write_buff( uint16_t nBytes, const uint8_t * pBuff ) {
	for ( uint16_t i=0; i< nBytes; i++ ) {
		if ( sw_i2c_isTXIS_error() != I2C_Ok ) return I2C_Error;
		sw_i2c_write( *pBuff++ );
	}
	return I2C_Ok;
}
static I2CSTATUS sw_i2c_read_buff( uint16_t nBytes, uint8_t * pBuff ) {
	for ( uint16_t i=0; i< nBytes; i++ ) {
		while( sw_is_RXNE_flag_ready() == false ) {}
		*pBuff++ = sw_i2c_read( DUMMY );
	}
	return I2C_Ok;
}


/*********************************** Write 1 byte ********************************/
I2CSTATUS sw_i2c_write_byte( uint8_t byte ) {
	sw_i2c_write_dir();
	sw_i2c_autoend_off();

	sw_i2c_nBytes(1);
	sw_i2c_start();
	if ( sw_i2c_isTXIS_error() != I2C_Ok ) return I2C_Error;

	sw_i2c_write( byte );
	whileTimer = 2;
	if ( sw_i2c_isTC_error() != I2C_Ok ) return I2C_Error;

	sw_i2c_stop();
	return I2C_Ok;
}
/********************************************************************************/
/*********************************** Read 1 byte ********************************/
I2CSTATUS sw_i2c_read_byte( uint8_t * byte ) {
	sw_i2c_read_dir();
	sw_i2c_autoend_off();

	sw_i2c_nBytes(1);
	sw_i2c_start();
	while( sw_is_RXNE_flag_ready() == false ) {}

	*byte = sw_i2c_read(DUMMY);
	if ( sw_i2c_isTC_error() != I2C_Ok ) return I2C_Error;

	sw_i2c_stop();
	return I2C_Ok;
}
/********************************************************************************/

I2CSTATUS sw_i2c_read_bulk( uint8_t  devAddr, uint8_t regAddr,
							uint16_t nBytes,  uint8_t * pBuff ) {
	sw_i2c_set_7bitAddr( devAddr );
	sw_i2c_write_byte( regAddr );
	sw_i2c_read_dir();

	if ( nBytes <= I2C_CR2_NBYTE_MAX ) {
		CLEAR_BIT( hI2Cx->I2C->CR2, I2C_CR2_RELOAD );
		sw_i2c_nBytes( nBytes );
	} else {
		SET_BIT( hI2Cx->I2C->CR2, I2C_CR2_RELOAD );
		sw_i2c_nBytes( I2C_CR2_NBYTE_MAX );
	}

	uint16_t nBlock = 0;
	uint16_t n 		= I2C_CR2_NBYTE_MAX - 1;
	sw_i2c_start();
	while ( nBytes ) {
		if ( nBytes >=  I2C_CR2_NBYTE_MAX ) {
			if ( nBlock != 0) n = I2C_CR2_NBYTE_MAX;

			sw_i2c_read_buff( n, (uint8_t *)pBuff +  nBlock++ * I2C_CR2_NBYTE_MAX  );
			while( sw_is_TCR_flag_ready() == false ) {}

			nBytes = nBytes - I2C_CR2_NBYTE_MAX;
			if ( nBytes < I2C_CR2_NBYTE_MAX ) {
				CLEAR_BIT( hI2Cx->I2C->CR2, I2C_CR2_RELOAD );
				sw_i2c_nBytes( nBytes );						// Last nBytes < 255
			}
			else
				sw_i2c_nBytes( I2C_CR2_NBYTE_MAX );
		} else {
			sw_i2c_read_buff( nBytes, (uint8_t *)pBuff +  nBlock*I2C_CR2_NBYTE_MAX );
			nBytes = 0;											// End of while() loop
		}
	}
	sw_i2c_stop();
	return I2C_Ok;
}
I2CSTATUS sw_i2c_write_bulk( uint8_t devAddr, uint8_t regAddr,
						 	  uint16_t nBytes, const uint8_t * pBuff ) {
	hI2Cx->I2C->CR2 = 0; hI2Cx->I2C->ICR = 0xffffffff;
	sw_i2c_set_7bitAddr( devAddr );
	sw_i2c_autoend_off();
	sw_i2c_write_dir();

	nBytes++;											// nBytes + 1 (regAddr byte)
	if ( nBytes < I2C_CR2_NBYTE_MAX + 1) {
		sw_i2c_nBytes( nBytes );
		nBytes--;
	} else {
		sw_i2c_nBytes( I2C_CR2_NBYTE_MAX );
		SET_BIT( hI2Cx->I2C->CR2, I2C_CR2_RELOAD );
	}

	sw_i2c_start();
	if ( sw_i2c_isTXIS_error() != I2C_Ok) return I2C_Error;

	sw_i2c_write( regAddr ); 							// First byte, address
	if ( sw_i2c_isTXIS_error() != I2C_Ok) return I2C_Error;

	uint16_t nBlock = 0;
	uint16_t n 		= I2C_CR2_NBYTE_MAX - 1;			//
	uint8_t shift	= 0;
	while ( nBytes ) {
		if ( nBytes >=  I2C_CR2_NBYTE_MAX ) {
			if ( nBlock == 0) {
				n = I2C_CR2_NBYTE_MAX - 1;
				sw_i2c_write_buff( n, (uint8_t *)pBuff );
				nBlock++;
			} else {
				n = I2C_CR2_NBYTE_MAX;
				sw_i2c_write_buff( n, (uint8_t *)pBuff +  nBlock++ * I2C_CR2_NBYTE_MAX - 1 );
			}
			while( sw_is_TCR_flag_ready() == false ) {}

			nBytes = nBytes - I2C_CR2_NBYTE_MAX;
			if ( nBytes < I2C_CR2_NBYTE_MAX ) {
				CLEAR_BIT( hI2Cx->I2C->CR2, I2C_CR2_RELOAD );
				sw_i2c_nBytes( nBytes );
				shift = 1;
			} else {
				sw_i2c_nBytes( I2C_CR2_NBYTE_MAX );
			}
		} else {
			sw_i2c_write_buff( nBytes , (uint8_t *)pBuff + nBlock*I2C_CR2_NBYTE_MAX - shift );
			nBytes = 0;									// End of while() loop
		}
	}
	sw_i2c_stop();
	return I2C_Ok;
}


I2CSTATUS sw_i2c_write_reg( uint8_t devAddr, uint8_t reg, uint8_t data ) {
	sw_i2c_set_7bitAddr( devAddr );
	sw_i2c_write_dir();
	sw_i2c_autoend_off();

	sw_i2c_nBytes(2);
	sw_i2c_start();
	if ( sw_i2c_isTXIS_error() != I2C_Ok ) return I2C_Error;

	sw_i2c_write( reg );
	if ( sw_i2c_isTXIS_error() != I2C_Ok ) return I2C_Error;

	sw_i2c_write( data );
	if ( sw_i2c_isTC_error() != I2C_Ok ) return I2C_Error;
	sw_i2c_stop();

	return I2C_Ok;
}


I2CSTATUS sw_i2c_read_reg( uint8_t devAddr, uint8_t reg, uint8_t * data ) {
	sw_i2c_set_7bitAddr( devAddr );
	sw_i2c_write_dir();
	sw_i2c_autoend_off();
	sw_i2c_nBytes(1);
	sw_i2c_start();
	if ( sw_i2c_isTXIS_error() != I2C_Ok) return I2C_Error;

	sw_i2c_write( reg );	while ( sw_is_TC_flag_ready() 	== false );

	sw_i2c_read_dir();
	sw_i2c_nBytes(1);
	sw_i2c_start();			while ( sw_is_RXNE_flag_ready() == false );

	*data = sw_i2c_read(DUMMY);
	if ( sw_i2c_isTC_error() != I2C_Ok ) return I2C_Error;
	sw_i2c_stop();

	return I2C_Ok;;
}

void sw_i2c_simple_init(void) {
#ifdef STM32L4
	RCC->APB1RSTR1 |=  RCC_APB1RSTR1_I2C1RST;
	RCC->APB1RSTR1 &= ~RCC_APB1RSTR1_I2C1RST;
	RCC->APB1ENR1  |=  RCC_APB1ENR1_I2C1EN;
#elif defined STM32F3
	RCC->APB1RSTR |=  RCC_APB1RSTR_I2C1RST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
	RCC->APB1ENR  |=  RCC_APB1ENR_I2C1EN;
#endif

	SET_BIT	 ( hI2Cx->I2C->CR1, I2C_CR1_SWRST );
	CLEAR_BIT( hI2Cx->I2C->CR1, I2C_CR1_SWRST );

	CLEAR_BIT( hI2Cx->I2C->CR1, I2C_CR1_SWRST );
	while( (hI2Cx->I2C->CR1 & I2C_CR1_PE) );

	gpio_pin_cfg( hI2Cx->scl_port,  hI2Cx->scl_pin,  gpio_mode_output_PP_FS );

	gpio_pin_LO ( hI2Cx->scl_port,  hI2Cx->scl_pin );
	delay_ms(1);
	gpio_pin_HI ( hI2Cx->scl_port,  hI2Cx->scl_pin );
	delay_ms(1);
	gpio_pin_LO ( hI2Cx->scl_port,  hI2Cx->scl_pin );
	delay_ms(1);
	gpio_pin_HI ( hI2Cx->scl_port,  hI2Cx->scl_pin );

	gpio_pin_cfg( hI2Cx->scl_port,  hI2Cx->scl_pin,  hI2Cx->alternateFun );
	gpio_pin_cfg( hI2Cx->sda_port,  hI2Cx->sda_pin,  hI2Cx->alternateFun );

	hI2Cx->I2C->TIMINGR  = (uint32_t)I2C_TIMING_80MHz_400KHz;
	SET_BIT( hI2Cx->I2C->CR1, I2C_CR1_PE );
}

I2CSTATUS sw_i2c_IsDeviceReady( uint8_t devAddr, uint32_t trials, uint16_t delayMS ) {
	sw_i2c_set_7bitAddr( devAddr );
	while (trials--) {
		sw_i2c_start();
		sw_i2c_stop();
		delay_ms( delayMS );
	}
	if ( hI2Cx->I2C->ISR & I2C_ISR_NACKF ) {
		return I2C_Nack;
	}
	return I2C_Ok;
}

I2CSTATUS sw_i2c_slave_test( uint8_t devAddr ) {
	I2CSTATUS i2cstatus = I2C_Ok;
	sw_i2c_set_7bitAddr( devAddr );

	sw_i2c_autoend_off();
	sw_i2c_write_dir();
	if ( sw_i2c_start() == I2C_Ok )
		i2cstatus = I2C_Ok;
	else
		i2cstatus = I2C_Error;

	return i2cstatus;
}


#ifdef I2C_TEST
void sw_i2c_test_write( uint8_t devAddr ) {
	sw_i2c_set_7bitAddr( devAddr );

	sw_i2c_autoend_off();
	sw_i2c_write_dir();
	sw_i2c_nBytes(2);

	sw_i2c_start();
	while ( sw_is_TXIS_flag_ready() == false ) { }
	sw_i2c_write( 0xaa );
	while ( sw_is_TXIS_flag_ready() == false ) { }
	sw_i2c_write( 0x55 );
	while ( sw_is_TC_flag_ready() == false ) { }
	sw_i2c_stop();
}
#define DS3231_EEPROM	0xAE
#define DS3231_ADDR		0xD0
#define DS3231_CONTROL	0x0E
void sw_ds3231_test( void ) {
	uint8_t ctrl;
	sw_i2c_read_reg( DS3231_ADDR, DS3231_CONTROL, &ctrl );
}
#include "string.h"
#define WORD	2
uint8_t TestText[] = "Simon testuje";	// 13 letters + '0'
void sw_ds3231_eeprom_test( void ) {
	// AT24C32
	sw_i2c_set_7bitAddr( DS3231_EEPROM );

	sw_i2c_write_dir();
	sw_i2c_autoend_off();

	uint8_t * 	ptr		= TestText;
	uint8_t		len 	= strlen( (const char *)TestText);
	uint8_t 	lenLast = len;
	uint16_t	regAddr = 0x0000;

	sw_i2c_nBytes( len + WORD );
	sw_i2c_start();						while ( sw_is_TXIS_flag_ready()	== false );
	sw_i2c_write( regAddr >> 8 );		while ( sw_is_TXIS_flag_ready()	== false );
	sw_i2c_write( (uint8_t)regAddr );	while ( sw_is_TXIS_flag_ready()	== false );

	while( len-- ) {
		while ( sw_is_TXIS_flag_ready() == false );
		sw_i2c_write( *ptr++ );
	}
	while ( sw_is_TC_flag_ready() == 0 ) {}
	sw_i2c_stop();
	delay_ms(5);

	sw_i2c_read_block( DS3231_EEPROM, 0x0000, lenLast,  &TestText[0] );

//	ptr = TestText;
//	sw_i2c_nBytes( WORD );
//	sw_i2c_start();			while ( sw_is_TXIS_flag_ready()	== false );
//	sw_i2c_write( 0x00 );	while ( sw_is_TXIS_flag_ready()	== false );
//	sw_i2c_write( 0x00 );	while ( sw_is_TC_flag_ready()	== false );
//
//	sw_i2c_read_dir();
//	sw_i2c_nBytes( lenLast );
//	sw_i2c_start();			while ( sw_is_RXNE_flag_ready()	== false );
//
//	while( lenLast-- ) {
//		*ptr++  = sw_i2c_read(DUMMY);
//	}
//	while ( sw_is_TC_flag_ready() == 0 ) {}
//	sw_i2c_stop();

}

#endif




