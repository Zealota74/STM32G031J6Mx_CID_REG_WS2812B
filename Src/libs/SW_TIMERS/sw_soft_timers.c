/*
 * sw_soft_timers.c
 *
 *  Created on: 05.07.2017
 *      Author: simon
 */

#include "sw_mcu_conf.h"
#include "../SW_BOARD/gpio.h"

#include "sw_soft_timers.h"

#include "../SW_BOARD/sw_led_blink_debug.h"


/**** Inicjujemy SysTick pod timery programowe *****/
uint32_t sw_softTimers_init( uint32_t timeBase_ms, uint32_t type ) {
//	register_irDecoded_event_callback(sw_switch_mode);

	if 		(type==MILI_SEC)	type = 1000;	// 1 ms
	else if (type==MICRO_SEC) 	type = 1;		// 1 us
	else return 0;								// błąd

	SysTick_Config( 48000000/type/timeBase_ms );
	return 1;									// ok
}
/***************************************************/

/******************************************************************************/
volatile uint16_t pressTimer, debounceTimer, repeatTimer;
volatile uint32_t riseTimer, fallTimer, peakTimer, fallAvgTimer, peakAvgTimer;
volatile uint16_t showMenuTimer;
volatile uint16_t softTimer2, softTimer5, softTimer3, whileTimer;

volatile static uint16_t delayTimer;
volatile static uint32_t milis = 0;
void SysTick_Handler( void ) {
	milis++;
	register uint16_t n;
	n = pressTimer; 	if (n) pressTimer = --n;
	n = debounceTimer; 	if (n) debounceTimer = --n;
	n = repeatTimer; 	if (n) repeatTimer = --n;
	n = delayTimer; 	if (n) delayTimer = --n;
	n = showMenuTimer; 	if (n) showMenuTimer = --n;
	n = softTimer2; 	if (n) softTimer2 = --n;
	n = softTimer3; 	if (n) softTimer3 = --n;
	n = softTimer5; 	if (n) softTimer5 = --n;
	n = whileTimer; 	if (n) whileTimer = --n;

	sw_led_blinking();
//	SW_IR_DECODED_EVENT();
}
/******************************************************************************/
uint32_t millis(void) {
	return milis;
}
/*********************************************************/
/*********************************************************/
void delay_ms( uint16_t ms ) {
	delayTimer = ms;
	while ( delayTimer ) {}
}
/*********************************************************/

