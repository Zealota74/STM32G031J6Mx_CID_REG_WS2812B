/*
 * sw_led_blink.h
 *
 *  Created on: 05.07.2017
 *      Author: simon
 */

#ifndef SW_LED_BLINK_H_
#define SW_LED_BLINK_H_

enum { _led_blinking, _led_not_blinking };

extern volatile uint32_t ledEventFlag;

extern int8_t sw_led_get_status(void);
extern void sw_led_blinking(void);
extern void sw_led_start_blinking( uint8_t repeat, uint16_t delay );

extern void sw_led_debug_init( void );
extern void sw_led_on(  void );
extern void sw_led_off( void );
extern void sw_led_blink( void );

#endif /* SW_LED_BLINK_H_ */
