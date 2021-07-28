/*
 * sw_ws2812_FX.h
 *
 *  Created on: 29.07.2019
 *      Author: simon
 */

#ifndef LIBS_SW_WS2812_SW_WS2812_FX_H_
#define LIBS_SW_WS2812_SW_WS2812_FX_H_

typedef float float32_t;

#define	RGB_LOOP
#define	FADE_INOUT
#define	STROBE
#define	HALLOWEEN_EYES
#define	CYLON_BOUNCE
#define	NEW_KITT
#define	TWINKLE
#define	TWINKLE_RANDOM
#define	SPARKLE
#define	SNOW_SPARKLE
//#define	RUNNING_LIGHTS					// Large
#define	COLOR_WIPE
#define	RAINBOW_CYCLE
#define	THEATER_CHASE
#define	THEATER_CHASERAINBOW
//#define	FIRE							// Large
#define	BOUNCING_COLORED_BALLS			// Large
#define	BOUNCING_COLORED_3BALLS			// Large
#define	METEOR_RAIN					// Large



void changeEffect(void);
void show_effects(void);


#endif /* LIBS_SW_WS2812_SW_WS2812_FX_H_ */
