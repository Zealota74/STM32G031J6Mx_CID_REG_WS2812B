/*
 * led_blink.c
 *
 *  Created on: 05.07.2017
 *      Author: simon
 */

#include "sw_mcu_conf.h"
#include "gpio.h"

#include "../SW_TIMERS/sw_soft_timers.h"
#include "../SW_BOARD/sw_led_blink_debug.h"


volatile uint32_t ledEventFlag = 0;


#ifdef BSRR_REG
void sw_led_off( void ) {
	LED_PORT->BSRR = LED_PIN;
}
void sw_led_on( void ) {
	LED_PORT->BRR = LED_PIN;
}
#else
	void sw_led_off( void ) {
		LED_PORT->BSRRL = LED_PIN;
	}
	void sw_led_on( void ) {
		LED_PORT->BSRRH = LED_PIN;
	}
#endif
void sw_led_xor( void ) {
	LED_PORT->ODR ^= LED_PIN;
}




void sw_led_blink( void ) { sw_led_xor(); }

void sw_led_debug_init( void ) {

#ifdef KA_NUCLEO
	// tryb wyjściowy, push-pull, 2MHz dla pinu PB1
	gpio_pin_cfg( LED_PORT, LED_PIN, 		gpio_mode_output_PP_HS );

//	gpio_pin_cfg( LED2_PORT, LED2_PIN_BLUE,	gpio_mode_output_PP_HS );
	gpio_pin_cfg( LED2_PORT, LED2_PIN_RED,	gpio_mode_output_PP_HS );
//	gpio_pin_cfg( LED2_PORT, LED2_PIN_GREEN,gpio_mode_output_PP_HS );

	// Wyłączamy wszystkie diody
	sw_led_off( LED_PORT, LED_PIN );

	sw_led_off( LED2_PORT, LED2_PIN_BLUE );
	sw_led_off( LED2_PORT, LED2_PIN_RED );
	sw_led_off( LED2_PORT, LED2_PIN_GREEN );
#else
	gpio_pin_cfg( LED_PORT, LED_PIN, 	gpio_mode_output_PP_LS );
	sw_led_off();
#endif

#ifdef	DEBUG_PIN0
	gpio_pin_cfg( DEBUG_PORT0, DEBUG_PIN0, gpio_mode_output_PP_HS );
#endif
#ifdef	DEBUG_PIN1
	gpio_pin_cfg( DEBUG_PORT1, DEBUG_PIN1, gpio_mode_output_PP_HS );
#endif
#ifdef	DEBUG_PIN2
	gpio_pin_cfg( DEBUG_PORT2, DEBUG_PIN2, gpio_mode_output_PP_HS );
#endif
}

static uint8_t blinkCounter = 0;
static uint8_t blinkDelay = 0;
void sw_led_start_blinking( uint8_t repeat, uint16_t delay ) {
	sw_led_off();
	blinkCounter = 2*repeat;
	blinkDelay = delay;
}
int8_t sw_led_get_status(void) {
	if (blinkCounter == 0) 	return (int8_t)_led_not_blinking;
	else					return (int8_t)_led_blinking;
}

void sw_led_blinking(void) {
	if ( blinkCounter == 0 || softTimer3 != 0 ) return;	// Nic nie robimy jeśli
	else {
		softTimer3 = blinkDelay;
		sw_led_blink();
		blinkCounter--;
	}
}
