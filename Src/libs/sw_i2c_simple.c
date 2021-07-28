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

#include "sw_i2c_simple.h"


static const I2C_t * hI2Cx = &i2c1Alt1;


static INLINE void sw_i2c_nBytes( uint8_t nBytes ) {
	MODIFY_REG( I2C1->CR2, I2C_CR2_NBYTES, nBytes << I2C_CR2_NBYTES_Pos );
}
static INLINE void sw_i2c_read_dir (void) { hI2Cx->I2C->CR2 |= I2C_CR2_RD_WRN; }
static INLINE void sw_i2c_write_dir(void) { hI2Cx->I2C->CR2 &= ~I2C_CR2_RD_WRN; }

static void sw_i2c_write_buff( uint16_t nBytes, const uint8_t * pBuff ) {
	for ( uint16_t i=0; i< nBytes; i++ ) {
		while( sw_is_TXIS_flag_ready() == false) {}
		sw_i2c_write( *pBuff++ );
	}
}
static void sw_i2c_read_buff( uint16_t nBytes, uint8_t * pBuff ) {
	for ( uint16_t i=0; i< nBytes; i++ ) {
		while( sw_is_RXNE_flag_ready() == false) {}
		*pBuff++ = sw_i2c_read();
	}
}

void sw_i2c_write_block( uint8_t devAddr, uint8_t regAddr,
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
	while ( sw_is_TXIS_flag_ready() == false ) {}
	sw_i2c_write( regAddr ); 							// First byte, address
	while ( sw_is_TXIS_flag_ready() == false ) {}

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
}

void sw_i2c_write_byte( uint8_t byte ) {
	sw_i2c_write_dir();
	sw_i2c_autoend_off();

	sw_i2c_nBytes(1);
	sw_i2c_start();
	while( sw_is_TXIS_flag_ready() == false ) {}	// All bytes send nBytes = 1
	sw_i2c_write( byte );
	while( sw_is_TC_flag_ready() == false ) {}		// All bytes send nBytes = 1
}
uint8_t sw_i2c_read_byte(void) {
	uint8_t byte;
	sw_i2c_read_dir();
	sw_i2c_autoend_off();

	sw_i2c_nBytes(1);
	sw_i2c_start();
	while( sw_is_RXNE_flag_ready() == false ) {}
	byte = sw_i2c_read();
	while( sw_is_TC_flag_ready() == false ) {}
	return byte;
}
void sw_i2c_read_block( uint8_t  devAddr, uint8_t regAddr,
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
			nBytes = 0;									// End of while() loop
		}
	}
	sw_i2c_stop();
}


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


void sw_i2c_write_reg( uint8_t devAddr, uint8_t reg, uint8_t data ) {
//	hI2Cx->I2C->CR2 = 0; hI2Cx->I2C->ICR = 0xffffffff;
	sw_i2c_set_7bitAddr( devAddr );
	sw_i2c_write_dir();
	sw_i2c_autoend_off();

	sw_i2c_nBytes(2);
	sw_i2c_start(); 		while ( sw_is_TXIS_flag_ready() == false ) {}
	sw_i2c_write( reg ); 	while ( sw_is_TXIS_flag_ready() == false );
	sw_i2c_write( data ); 	while ( sw_is_TC_flag_ready() 	== false );
	sw_i2c_stop();
}


I2CSTATUS sw_i2c_read_reg( uint8_t devAddr, uint8_t reg, uint8_t * data ) {
	sw_i2c_set_7bitAddr( devAddr );
	sw_i2c_write_dir();
	sw_i2c_autoend_off();
	I2CSTATUS status = I2C_Ok;
	sw_i2c_nBytes(1);
	status = sw_i2c_start();
	while ( sw_is_TXIS_flag_ready() == false );
	sw_i2c_write( reg ); 	while ( sw_is_TC_flag_ready() 	== false );

	sw_i2c_read_dir();
	sw_i2c_nBytes(1);
	sw_i2c_start();			while ( sw_is_RXNE_flag_ready() == false );

	*data = sw_i2c_read();	while ( sw_is_TC_flag_ready()	== false );
	sw_i2c_stop();

	return status;
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
//		*ptr++  = sw_i2c_read();
//	}
//	while ( sw_is_TC_flag_ready() == 0 ) {}
//	sw_i2c_stop();

}

void sw_i2c_simple_init(void) {
#ifdef STM32L4
	RCC->APB1RSTR1 |=  RCC_APB1RSTR1_I2C1RST;
	RCC->APB1RSTR1 &= ~RCC_APB1RSTR1_I2C1RST;

	RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
#elif defined STM32G031xx
	RCC->APBRSTR1 |=  RCC_APBRSTR1_I2C1RST;
	RCC->APBRSTR1 &= ~RCC_APBRSTR1_I2C1RST;

	RCC->APBENR1 |= RCC_APBENR1_I2C1EN;
#endif
	CLEAR_BIT( hI2Cx->I2C->CR1, I2C_CR1_PE );
	while( (hI2Cx->I2C->CR1 & I2C_CR1_PE) );

	gpio_pin_cfg( hI2Cx->scl_port,  hI2Cx->scl_pin,  hI2Cx->alternateFun );
	gpio_pin_cfg( hI2Cx->sda_port,  hI2Cx->sda_pin,  hI2Cx->alternateFun );
	gpio_pin_HI ( hI2Cx->scl_port,  hI2Cx->scl_pin );
	gpio_pin_HI ( hI2Cx->sda_port,  hI2Cx->sda_pin );

	hI2Cx->I2C->TIMINGR  = (uint32_t)I2C_TIMING_80MHz_400KHz;
	SET_BIT( hI2Cx->I2C->CR1, I2C_CR1_PE );
}



I2CSTATUS sw_i2c_IsDeviceReady( uint8_t devAddr, uint32_t trials ) {
	uint32_t reg;

	while (trials--) {
		// Generate START
		sw_i2c_set_7bitAddr( devAddr );
		sw_i2c_start();

		// Wait for STOP, NACK or BERR
		whileTimer = 2;
		while ( !((reg = hI2Cx->I2C->ISR) & (I2C_ISR_STOPF | I2C_ISR_NACKF | I2C_ISR_BERR)) ) {
			if ( whileTimer == 0 ) return I2C_Error;
		}

		// Wait while STOP flag is reset
		whileTimer = 2;
		while ( !(hI2Cx->I2C->ISR & I2C_ISR_STOPF) ) {
			if (whileTimer == 0) return I2C_Error;
		}

		// Clear the NACK, STOP and BERR flags
		hI2Cx->I2C->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF | I2C_ICR_BERRCF;

		// Check for BERR flag
		if ( reg & I2C_ISR_BERR ) {
			// TODO: I2C: how to handle the misplaced START/STOP?
		} else {
			// Device responded if NACK flag is not set
			if (!(reg & I2C_ISR_NACKF)) {
				sw_i2c_stop();
				return I2C_Ok;
			}
		}
	}
	return I2C_Error;
}



