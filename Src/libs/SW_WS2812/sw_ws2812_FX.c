/*
 * sw_ws2812_FX.c
 *
 *  Created on: 29.07.2019
 *      Author: simon
 */

#include <stdlib.h>
#include <math.h>

#include "sw_mcu_conf.h"
#include "../SW_BOARD/gpio.h"
#include "../SW_TIMERS/sw_soft_timers.h"
#include "../SW_BOARD/sw_led_blink_debug.h"

#include "sw_ws2812.h"

#include "sw_ws2812_FX.h"

static bool stopEffect 			= false;
static uint8_t selectedEffect	= 5;

static const uint8_t bounceColors[][3] = {
	{ 0xff, 0x00, 0x00 },
	{ 0xff, 0xff, 0xff },
	{ 0x00, 0x00, 0xff }
};

// *************************
// ** LEDEffect Functions **
// *************************
static void setAll(uint8_t red, uint8_t blue, uint8_t green) {
	for (uint16_t i=0; i<WS_LED_CNT; i++) {
		ws2812_set_pixel( i, red, blue, green );
	}
}

static uint32_t map( int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max ) {
	uint32_t value;
	uint32_t a = (x - in_min);
	uint32_t b = (out_max - out_min);
	uint32_t c = (in_max - in_min);
	value = out_min + (uint64_t)a*b/c;

	return value;
}
static int32_t random1( uint32_t value ) {
	return map( random(), 0, RAND_MAX, 0, value );
}
static int32_t random2( uint32_t a, uint32_t b ) {
	uint32_t rand = random();
	rand = map( rand, 0, RAND_MAX, a, b );
	return rand;
}
static bool stop_effect(void) {
	if (stopEffect==true) {
		stopEffect = false;
		setAll(0,0,0);
		sw_ws2812_send_buff();
//		delay_ms(2);
		return true;
	} else {
		return false;
	}
}
/*******************************************************************/

#define PHASE_1		1
#define PHASE_2		2
#define PHASE_3		3
static uint8_t phase = PHASE_1;

static void FadeInOut2( uint8_t red, uint8_t green, uint8_t blue ) {
	if ( stop_effect() ) return;

	static uint16_t k 	 = 0;


	if (phase == PHASE_1) {
		softTimer5 = 5;
		k++;
		if (k == 256) { phase = PHASE_2; }
	} else
	if (phase == PHASE_2) {
		softTimer5 = 1;
		k--;
		if (k == 0) { phase = PHASE_1; }
	}
	red = (k * red)/256;
	green = (k * green)/256;
	blue = (k * green)/256;
	setAll(red, green, blue);

	sw_ws2812_send_buff();

}
static void RGBLoop2() {
	static uint16_t k = 0;
	static uint16_t j = 0;

	// Fade IN
	if (phase == PHASE_1) {
		softTimer5 = 5;
		k++;
		if (k == 256) { phase = PHASE_2; }
	}
	// Fade OUT
	if (phase == PHASE_2) {
		softTimer5 = 5;
		k--;
		if (k == 0) { phase = PHASE_1; j++; }
	}
	switch (j) {
		case 0:
			setAll(k, 0, 0); break;
		case 1:
			setAll(0, k, 0); break;
		case 2:
			setAll(0, 0, k); break;
		default:			 break;
	}
	sw_ws2812_send_buff();
	if (j == 3) j = 0;
}
// Strobe(0xff, 0xff, 0xff, 10, 50, 1000);
static void Strobe2( uint8_t red, uint8_t green, uint8_t blue, int StrobeCount,
					 int FlashDelay, int EndPause ) {
	static uint16_t k;
	static uint8_t phase = PHASE_1;
//	for (uint16_t j = 0; j < StrobeCount; j++) {
	if (phase == PHASE_1) {
		setAll(red, green, blue);
		phase = PHASE_2;
		softTimer5 = FlashDelay;
	} else
	if (phase == PHASE_2) {
		setAll(0, 0, 0);
		phase = PHASE_1;
		if ( k-- > 0 ) {
			softTimer5 = FlashDelay;
		}
		else {
			k = StrobeCount;
			softTimer5 = EndPause;
		}
	}
	sw_ws2812_send_buff();
//	}
}
static void HalloweenEyes2(  uint8_t red, uint8_t green, uint8_t blue, int EyeWidth,
							int EyeSpace, bool Fade, int steps, int FadeDelay, int EndPause) {
	int32_t i;
	static uint32_t j, stepsConst;

	static volatile int StartPoint;
	static volatile int Start2ndEye;

	if (phase == PHASE_1) {
		StartPoint = random2( 0, WS_LED_CNT - (2 * EyeWidth) - EyeSpace );
		Start2ndEye = StartPoint + EyeWidth + EyeSpace;
		for (i = 0; i < EyeWidth; i++) {
			ws2812_set_pixel( StartPoint  + i, red, green, blue );
			ws2812_set_pixel( Start2ndEye + i, red, green, blue );
		}
		j 			= steps;
		stepsConst 	= steps;
		phase = PHASE_2;
		sw_ws2812_send_buff();
		softTimer5 = 3;
	}
	else if (phase == PHASE_2) {
		uint8_t r, g, b;

		r = (j*red)   / stepsConst;
		g = (j*green) / stepsConst;
		b = (j*blue)  / stepsConst;

		for (i = 0; i < EyeWidth; i++) {
			ws2812_set_pixel( StartPoint  + i, r, g, b );
			ws2812_set_pixel( Start2ndEye + i, r, g, b );
		}
		sw_ws2812_send_buff();
		softTimer5 = FadeDelay;

		if (--j == 0) {
			phase = PHASE_3;
		}
	}
	if ( phase == PHASE_3 ){
		softTimer5 = EndPause;
		setAll(0, 0, 0); 		// Set all black
		phase = PHASE_1;
	}
}
void static CylonBounce2( uint8_t red, uint8_t green, uint8_t blue, int EyeSize,
						 int SpeedDelay, int ReturnDelay ) {
	static uint16_t i = 0;
	switch (phase) {
		case PHASE_1:
			setAll(0, 0, 0);
			ws2812_set_pixel(i, red / 10, green / 10, blue / 10);
			for ( uint_fast16_t j = 1; j <= EyeSize; j++) {
				ws2812_set_pixel(i + j, red, green, blue);
			}
			ws2812_set_pixel( i++ + EyeSize + 1, red / 10, green / 10, blue / 10);
			sw_ws2812_send_buff();
			softTimer5 = SpeedDelay;
			if (i == WS_LED_CNT - EyeSize - 2) {
				phase = PHASE_2;
			}
			break;
		case PHASE_2:
			softTimer5 = ReturnDelay;
			phase = PHASE_3;
			i = WS_LED_CNT - EyeSize - 2;
			break;
		case PHASE_3:
			setAll(0, 0, 0);
			ws2812_set_pixel( i--, red / 10, green / 10, blue / 10 );
			for (uint16_t j = 1; j <= EyeSize; j++) {
				ws2812_set_pixel(i + j, red, green, blue);
			}
			ws2812_set_pixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
			sw_ws2812_send_buff();
			softTimer5 = SpeedDelay;

			if (i == 0) {
				phase = PHASE_1;
			}
			break;
		default:
			softTimer5 = ReturnDelay;
			break;
	}
}

#define START_HEIGHT	1
#define GRAVITY 	-9.81
#define BALL_COUNT	4
float32_t const ImpactVelocityStart 	= sqrtf(-2 * GRAVITY * START_HEIGHT);
float32_t const Dampening				= 0.90;					// Tłumienie (ostudzenie?)

static void BouncingColoredBalls2( int BallCount, const uint8_t colors[][3], bool continuous ) {
	// lokalne STAŁE
	bool ballsStillBouncing = true;

	float32_t Height[BALL_COUNT];
	float32_t TimeSinceLastBounce[BALL_COUNT];

	static int Position[BALL_COUNT];
	static bool ballBouncing[BALL_COUNT];
	static uint32_t  ClockTimeSinceLastBounce[BALL_COUNT];
	static float32_t Dampening1[BALL_COUNT];
	static float32_t ImpactVelocity[BALL_COUNT];

	switch (phase) {
		case PHASE_1:
			for ( uint16_t i = 0; i < BALL_COUNT; i++ ) {
				ImpactVelocity[i] 	= ImpactVelocityStart;
				Height[i]			= START_HEIGHT;
				Dampening1[i] 		= 0.90 - (float32_t) i / powf(BALL_COUNT, 2);

				ClockTimeSinceLastBounce[i] = millis();
				Position[i] 				= 0;
				TimeSinceLastBounce[i]		= 0;
				ballBouncing[i] 			= true;
			}
			phase = PHASE_2;
			break;

		case PHASE_2:
			if (ballsStillBouncing) {

				for ( uint16_t i = 0; i < BALL_COUNT; i++ ) {

					TimeSinceLastBounce[i]  = millis() - ClockTimeSinceLastBounce[i];
					Height[i]				=  0.5 * GRAVITY * powf(TimeSinceLastBounce[i] / 1000, 2.0)
										 	  + ImpactVelocity[i] * TimeSinceLastBounce[i] / 1000;
					if (Height[i] < 0) {
						Height[i]					= 0;
						ImpactVelocity[i]			= Dampening1[i] * ImpactVelocity[i];
						ClockTimeSinceLastBounce[i]	= millis();

						if (ImpactVelocity[i] < 0.01) {
							if (continuous) ImpactVelocity[i]  	= ImpactVelocityStart;
							else  			ballBouncing[i]		= false;
						}
					}
					Position[i] = (int)(Height[i] * ( WS_LED_CNT - 1 ) / START_HEIGHT);
				}

				setAll(0, 0, 0);
				ballsStillBouncing = false; // assume no balls bouncing

				for (uint16_t i = 0; i < BALL_COUNT; i++) {
					ws2812_set_pixel(Position[i], colors[i][0], colors[i][1], colors[i][2]);

					if (ballBouncing[i]) {
						ballsStillBouncing = true;
					}
				}

				sw_ws2812_send_buff();
				softTimer5 = 2;
			}
			if (ballsStillBouncing==false){
				phase = PHASE_1;
			}
		break;
		default:
		break;
	}
}

void show_effects2(void) {
	switch ( selectedEffect ) {
	case 0:
		RGBLoop2();
		break;
	case 1:
		FadeInOut2(0,255,0);
		break;
	case 2:
		Strobe2( 0xff, 0xaa, 0xff, 10, 50, 1000 );
		break;
	case 3:
		HalloweenEyes2( 0xff, 0x00, 0x00, 1, 4, true, random2(5, 50), random2(10, 100), random2(1000, 2000) );
		break;
	case 4:
		CylonBounce2( 0xff, 0x00, 0x00, 4, 10, 50 );
		break;
	case 5:
		// multiple colored balls
		BouncingColoredBalls2( 3, bounceColors, false );
		break;
	default:
		break;
	}
}
INTERRUPT void TIM16_IRQHandler(void) {
	if ( TIM16->SR & TIM_SR_UIF ) {
		TIM16->SR = ~TIM_SR_UIF;
		if (softTimer5) {
			return;
		} else {
			gpio_pin_XOR( DEBUG_PORT0, DEBUG_PIN0);
			show_effects2();
		}
	}
}


/**********************************************************************/

#ifdef	RGB_LOOP
static void RGBLoop() {
	for (uint16_t j = 0; j < 3; j++) {

		// Fade IN
		for (uint16_t k = 0; k < 256; k++) {
			switch (j) {
			case 0:
				setAll(k, 0, 0);
				break;
			case 1:
				setAll(0, k, 0);
				break;
			case 2:
				setAll(0, 0, k);
				break;
			}
			sw_ws2812_send_buff();
			delay_ms(3);

			if ( stop_effect() ) return;
		}
		// Fade OUT
		for (uint16_t k = 255; k >= 0; k--) {
			switch (j) {
			case 0:
				setAll(k, 0, 0);
				break;
			case 1:
				setAll(0, k, 0);
				break;
			case 2:
				setAll(0, 0, k);
				break;
			}
			sw_ws2812_send_buff();
			delay_ms(3);

			if ( stop_effect() ) return;
		}
	}
}
#endif
#ifdef	FADE_INOUT
static void FadeInOut( uint8_t red, uint8_t green, uint8_t blue ) {
	uint32_t r, g, b;

	for (uint16_t k = 0; k < 256; k++ ) {
		r = (k * red)/256;
		g = (k * green)/256;
		b = (k * green)/256;
		setAll(r, g, b);
		sw_ws2812_send_buff();
		delay_ms(10);

		if ( stop_effect() ) return;
	}
	for (uint16_t k = 255; k >= 0; k = k-2) {
		r = (k * red)/256;
		g = (k * green)/256;
		b = (k * green)/256;
		setAll(r, g, b);
		sw_ws2812_send_buff();
		delay_ms(10);

		if ( stop_effect() ) return;
	}
}
#endif
#ifdef	STROBE
static void Strobe( uint8_t red, uint8_t green, uint8_t blue, int StrobeCount,
					int FlashDelay, int EndPause ) {
	for (uint16_t j = 0; j < StrobeCount; j++) {
		setAll(red, green, blue);
		sw_ws2812_send_buff();
		delay_ms(FlashDelay);

		if ( stop_effect() ) return;

		setAll(0, 0, 0);
		sw_ws2812_send_buff();
		delay_ms(FlashDelay);

		if ( stop_effect() ) return;
	}
	delay_ms(EndPause);
}
#endif
#ifdef	HALLOWEEN_EYES
static void HalloweenEyes(  uint8_t red, uint8_t green, uint8_t blue, int EyeWidth,
							int EyeSpace, bool Fade, int Steps, int FadeDelay, int EndPause) {
	int32_t i;
	volatile int StartPoint = random2( 0, WS_LED_CNT - (2 * EyeWidth) - EyeSpace );
	volatile int Start2ndEye = StartPoint + EyeWidth + EyeSpace;

	for (i = 0; i < EyeWidth; i++) {
		ws2812_set_pixel(StartPoint + i, red, green, blue);
		ws2812_set_pixel(Start2ndEye + i, red, green, blue);
	}
	sw_ws2812_send_buff();
	delay_ms(3);

	if (Fade == true) {
		uint8_t r, g, b;
		uint32_t j = Steps;
		while( j--) {
			r = (j*red)  / Steps;
			g = (j*green)/ Steps;
			b = (j*blue) / Steps;

			for (i = 0; i < EyeWidth; i++) {
				ws2812_set_pixel(StartPoint + i, r, g, b);
				ws2812_set_pixel(Start2ndEye + i, r, g, b);
			}
			sw_ws2812_send_buff();
			delay_ms(FadeDelay);

			if ( stop_effect() ) return;
		}
	}
	setAll(0, 0, 0); // Set all black
	delay_ms(EndPause);

	if ( stop_effect() ) return;
}
#endif
#ifdef	CYLON_BOUNCE
void static CylonBounce( uint8_t red, uint8_t green, uint8_t blue, int EyeSize,
						 int SpeedDelay, int ReturnDelay ) {
	for ( uint16_t i = 0; i < WS_LED_CNT - EyeSize - 2; i++ ) {
		setAll(0, 0, 0);
		ws2812_set_pixel(i, red / 10, green / 10, blue / 10);
		for (uint16_t j = 1; j <= EyeSize; j++) {
			ws2812_set_pixel(i + j, red, green, blue);
		}
		ws2812_set_pixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
		sw_ws2812_send_buff();
		delay_ms(SpeedDelay);

		if ( stop_effect() ) return;
	}
	delay_ms(ReturnDelay);
	for (uint16_t i = WS_LED_CNT - EyeSize - 2; i > 0; i--) {
		setAll(0, 0, 0);
		ws2812_set_pixel(i, red / 10, green / 10, blue / 10);
		for (uint16_t j = 1; j <= EyeSize; j++) {
			ws2812_set_pixel(i + j, red, green, blue);
		}
		ws2812_set_pixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
		sw_ws2812_send_buff();
		delay_ms(SpeedDelay);

		if ( stop_effect() ) return;
	}
	delay_ms(ReturnDelay);
}
#endif
#ifdef	NEW_KITT
// used by NewKITT
void static CenterToOutside(uint8_t red, uint8_t green, uint8_t blue, int EyeSize,
							int SpeedDelay, int ReturnDelay) {
	for ( int16_t i = ((WS_LED_CNT - EyeSize) / 2); i >= 0; i--) {
		setAll(0, 0, 0);

		ws2812_set_pixel(i, red / 10, green / 10, blue / 10);
		for (uint16_t j = 1; j <= EyeSize; j++) {
			ws2812_set_pixel(i + j, red, green, blue);
		}
		ws2812_set_pixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);

		ws2812_set_pixel(WS_LED_CNT - i, red / 10, green / 10, blue / 10);
		for (uint16_t j = 1; j <= EyeSize; j++) {
			ws2812_set_pixel(WS_LED_CNT - i - j, red, green, blue);
		}
		ws2812_set_pixel(WS_LED_CNT - i - EyeSize - 1, red / 10, green / 10, blue / 10);

		sw_ws2812_send_buff();
		delay_ms(SpeedDelay);

		if ( stop_effect() ) return;
	}
	delay_ms(ReturnDelay);
}
// used by NewKITT
void static OutsideToCenter(uint8_t red, uint8_t green, uint8_t blue, int EyeSize,
					 int SpeedDelay, int ReturnDelay) {

	for (uint16_t i = 0; i <= ((WS_LED_CNT - EyeSize) / 2); i++) {
		setAll(0, 0, 0);

		ws2812_set_pixel(i, red / 10, green / 10, blue / 10);
		for (uint16_t j = 1; j <= EyeSize; j++) {
			ws2812_set_pixel(i + j, red, green, blue);
		}
		ws2812_set_pixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);

		ws2812_set_pixel(WS_LED_CNT - i, red / 10, green / 10, blue / 10);
		for (uint16_t j = 1; j <= EyeSize; j++) {
			ws2812_set_pixel(WS_LED_CNT - i - j, red, green, blue);
		}
		ws2812_set_pixel(WS_LED_CNT - i - EyeSize - 1, red / 10, green / 10, blue / 10);

		sw_ws2812_send_buff();
		delay_ms(SpeedDelay);

		if ( stop_effect() ) return;
	}
	delay_ms(ReturnDelay);
}
// used by NewKITT
static void LeftToRight(uint8_t red, uint8_t green, uint8_t blue, int EyeSize,
		int SpeedDelay, int ReturnDelay) {
	for (uint16_t i = 0; i < WS_LED_CNT - EyeSize - 2; i++) {
		setAll(0, 0, 0);
		ws2812_set_pixel(i, red / 10, green / 10, blue / 10);
		for (uint16_t j = 1; j <= EyeSize; j++) {
			ws2812_set_pixel(i + j, red, green, blue);
		}
		ws2812_set_pixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
		sw_ws2812_send_buff();
		delay_ms(SpeedDelay);

		if ( stop_effect() ) return;
	}
	delay_ms(ReturnDelay);
}
// used by NewKITT
static void RightToLeft(uint8_t red, uint8_t green, uint8_t blue, int EyeSize,
						int SpeedDelay, int ReturnDelay) {
	for (uint16_t i = WS_LED_CNT - EyeSize - 2; i > 0; i--) {
		setAll(0, 0, 0);
		ws2812_set_pixel(i, red / 10, green / 10, blue / 10);
		for (uint16_t j = 1; j <= EyeSize; j++) {
			ws2812_set_pixel(i + j, red, green, blue);
		}
		ws2812_set_pixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
		sw_ws2812_send_buff();
		delay_ms(SpeedDelay);

		if ( stop_effect() ) return;
	}
	delay_ms(ReturnDelay);
}
void static NewKITT( uint8_t red, uint8_t green, uint8_t blue, int EyeSize,
			 	 	 int SpeedDelay, int ReturnDelay) {
	RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
}
#endif
#ifdef	TWINKLE
static void Twinkle(uint8_t red, uint8_t green, uint8_t blue, int Count,
					int SpeedDelay, bool OnlyOne) {
	setAll(0, 0, 0);

	for (uint16_t i = 0; i < Count; i++) {
		ws2812_set_pixel( random1(WS_LED_CNT), red, green, blue);
		sw_ws2812_send_buff();
		delay_ms(SpeedDelay);

		if ( stop_effect() ) return;

		if (OnlyOne) {
			setAll(0, 0, 0);
		}
	}
	delay_ms(SpeedDelay);
}
#endif
#ifdef	TWINKLE_RANDOM
static void TwinkleRandom(int Count, int SpeedDelay, bool OnlyOne) {
	setAll(0, 0, 0);

	for (uint16_t i = 0; i < Count; i++) {
		ws2812_set_pixel( random1(WS_LED_CNT), random2(0, 255), random2(0, 255),
				random2(0, 255));
		sw_ws2812_send_buff();
		delay_ms(SpeedDelay);

		if ( stop_effect() ) return;

		if (OnlyOne) {
			setAll(0, 0, 0);
		}
	}

	delay_ms(SpeedDelay);
}
#endif
#ifdef	SPARKLE
static void Sparkle(uint8_t red, uint8_t green, uint8_t blue, int SpeedDelay) {
	volatile int Pixel = random1(WS_LED_CNT);
	ws2812_set_pixel(Pixel, red, green, blue);
	sw_ws2812_send_buff();
	delay_ms(SpeedDelay);

	if ( stop_effect() ) return;

	ws2812_set_pixel(Pixel, 0, 0, 0);
}
#endif
#ifdef	SNOW_SPARKLE
static void SnowSparkle(uint8_t red, uint8_t green, uint8_t blue, int SparkleDelay,
						int SpeedDelay) {
	setAll(red, green, blue);

	int Pixel = random1(WS_LED_CNT);
	ws2812_set_pixel(Pixel, 0xff, 0xff, 0xff);
	sw_ws2812_send_buff();
	delay_ms(SparkleDelay);

	if ( stop_effect() ) return;

	ws2812_set_pixel(Pixel, red, green, blue);
	sw_ws2812_send_buff();
	delay_ms(SpeedDelay);
}
#endif
#ifdef	RUNNING_LIGHTS
static void RunningLights(uint8_t red, uint8_t green, uint8_t blue, int WaveDelay) {
	int Position = 0;
	for (uint16_t i = 0; i < WS_LED_CNT * 2; i++) {
		Position++; // = 0; //Position + Rate;
		for (uint16_t i = 0; i < WS_LED_CNT; i++) {
			// sine wave, 3 offset waves make a rainbow!
			//float32_t level = sin(i+Position) * 127 + 128;
			//ws2812_set_pixel(i,level,0,0);
			//float32_t level = sin(i+Position) * 127 + 128;
			ws2812_set_pixel(i,((sin(i + Position) * 127 + 128) / 255) * red,
						((sin(i + Position) * 127 + 128) / 255) * green,
						((sin(i + Position) * 127 + 128) / 255) * blue);
		}
		sw_ws2812_send_buff();
		delay_ms( WaveDelay );

		if ( stop_effect() ) return;
	}
}
#endif
#ifdef	COLOR_WIPE
static void colorWipe(uint8_t red, uint8_t green, uint8_t blue, int SpeedDelay) {
	for (uint16_t i = 0; i < WS_LED_CNT; i++) {
		ws2812_set_pixel(i, red, green, blue);

		sw_ws2812_send_buff();
		delay_ms(SpeedDelay);

		if ( stop_effect() ) return;
	}
}
#endif
#ifdef	RAINBOW_CYCLE
// used by rainbowCycle and theaterChaseRainbow
static uint8_t * Wheel(uint8_t WheelPos) {
	static uint8_t c[3];

	if (WheelPos < 85) {
		c[0] = WheelPos * 3;
		c[1] = 255 - WheelPos * 3;
		c[2] = 0;
	} else if (WheelPos < 170) {
		WheelPos -= 85;
		c[0] = 255 - WheelPos * 3;
		c[1] = 0;
		c[2] = WheelPos * 3;
	} else {
		WheelPos -= 170;
		c[0] = 0;
		c[1] = WheelPos * 3;
		c[2] = 255 - WheelPos * 3;
	}
	return c;
}
static void rainbowCycle(int SpeedDelay) {
	uint8_t *c;
	uint16_t i, j;

	for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
		for (i = 0; i < WS_LED_CNT; i++) {
			c = Wheel(((i * 256 / WS_LED_CNT) + j) & 255);
			ws2812_set_pixel(i, *c, *(c + 1), *(c + 2));
		}
		sw_ws2812_send_buff();
		delay_ms(SpeedDelay);

		if ( stop_effect() ) return;
	}
}
#endif
#ifdef	THEATER_CHASE
static void theaterChase( uint8_t red, uint8_t green, uint8_t blue, int SpeedDelay ) {
	for (uint16_t j = 0; j < 10; j++) {  //do 10 cycles of chasing
		for (uint16_t q = 0; q < 3; q++) {
			for (uint16_t i = 0; i < WS_LED_CNT; i = i + 3) {
				ws2812_set_pixel(i + q, red, green, blue);   //turn every third pixel on
			}
			sw_ws2812_send_buff();
			delay_ms(SpeedDelay);

			if ( stop_effect() ) return;

			for (uint16_t i = 0; i < WS_LED_CNT; i = i + 3) {
				ws2812_set_pixel(i + q, 0, 0, 0);        //turn every third pixel off
			}
		}
	}
}
#endif
#ifdef	THEATER_CHASERAINBOW
static void theaterChaseRainbow(int SpeedDelay) {
	uint8_t *c;

	for (uint16_t j = 0; j < 256; j++) {     // cycle all 256 colors in the wheel
		for (uint16_t q = 0; q < 3; q++) {
			for (uint16_t i = 0; i < WS_LED_CNT; i = i + 3) {
				c = Wheel((i + j) % 255);
				ws2812_set_pixel(i + q, *c, *(c + 1), *(c + 2)); //turn every third pixel on
			}
			sw_ws2812_send_buff();
			delay_ms(SpeedDelay);

			if ( stop_effect() ) return;

			for (uint16_t i = 0; i < WS_LED_CNT; i = i + 3) {
				ws2812_set_pixel(i + q, 0, 0, 0);        //turn every third pixel off
			}
		}
	}
}
#endif
#ifdef	FIRE
/**********************************************************************/
static void setPixelHeatColor( int Pixel, uint8_t temperature ) {
// Scale 'heat' down from 0-255 to 0-191
	uint8_t t192 = (uint8_t)((temperature / 255.0) * 191);
// calculate ramp up from
	uint8_t heatramp = t192 & 0x3F; // 0..63
	heatramp <<= 2; 				// scale up to 0..252

// figure out which third of the spectrum we're in:
	if (t192 > 0x80) {                     		// hottest
		ws2812_set_pixel(Pixel, 255, 255, heatramp);
	} else if (t192 > 0x40) {             		// middle
		ws2812_set_pixel(Pixel, 255, heatramp, 0);
	} else {                               		// coolest
		ws2812_set_pixel(Pixel, heatramp, 0, 0);
	}
}
static void Fire(int Cooling, int Sparking, int SpeedDelay) {
	static uint8_t heat[ WS_LED_CNT ];
	int cooldown;

// Step 1.  Cool down every cell a little
	for (uint16_t i = 0; i < WS_LED_CNT; i++) {
		cooldown = random2(0, ((Cooling * 10) / WS_LED_CNT) + 2);

		if (cooldown > heat[i]) {
			heat[i] = 0;
		} else {
			heat[i] = heat[i] - cooldown;
		}
	}
// Step 2.  Heat from each cell drifts 'up' and diffuses a little
	for (uint16_t k = WS_LED_CNT - 1; k >= 2; k--) {
		heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
	}
// Step 3.  Randomly ignite new 'sparks' near the bottom
	if (random1(255) < Sparking) {
		int y = random1(7);
		heat[y] = heat[y] + random2(160, 255);
		//heat[y] = random(160,255);
	}
// Step 4.  Convert heat to LED colors
	for (uint16_t j = 0; j < WS_LED_CNT; j++) {
		setPixelHeatColor(j, heat[j]);
	}
	sw_ws2812_send_buff();
	delay_ms(SpeedDelay);
}
/**********************************************************************/
#endif
#ifdef	BOUNCING_COLORED_BALLS
/**********************************************************************/
static void BouncingColoredBalls( int BallCount, const uint8_t colors[][3], bool continuous ) {
	float32_t Gravity 	= -9.81;
	int StartHeight 	= 1;

	float32_t Height[BallCount];
	float32_t ImpactVelocityStart = sqrtf(-2 * Gravity * StartHeight);
	float32_t ImpactVelocity[BallCount];
	float32_t TimeSinceLastBounce[BallCount];

	int Position[BallCount];
	uint32_t ClockTimeSinceLastBounce[BallCount];
	float32_t Dampening[BallCount];
	bool ballBouncing[BallCount];
	bool ballsStillBouncing = true;

	for (uint16_t i = 0; i < BallCount; i++) {
		ClockTimeSinceLastBounce[i] = millis();
		Height[i] = StartHeight;
		Position[i] = 0;
		ImpactVelocity[i] = ImpactVelocityStart;
		TimeSinceLastBounce[i] = 0;
		Dampening[i] = 0.90 - (float32_t) i / powf(BallCount, 2);
		ballBouncing[i] = true;
	}
	while (ballsStillBouncing) {
		for (uint16_t i = 0; i < BallCount; i++) {
			TimeSinceLastBounce[i] = millis() - ClockTimeSinceLastBounce[i];
			Height[i] =  0.5 * Gravity * powf(TimeSinceLastBounce[i] / 1000, 2.0)
						+ ImpactVelocity[i] * TimeSinceLastBounce[i] / 1000;

			if (Height[i] < 0) {
				Height[i] = 0;
				ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
				ClockTimeSinceLastBounce[i] = millis();

				if (ImpactVelocity[i] < 0.01) {
					if (continuous) {
						ImpactVelocity[i] = ImpactVelocityStart;
					} else {
						ballBouncing[i] = false;
					}
				}
			}
			Position[i] = (int)(Height[i] * ( WS_LED_CNT - 1 ) / StartHeight);
		}

		ballsStillBouncing = false; // assume no balls bouncing
		for (uint16_t i = 0; i < BallCount; i++) {
			ws2812_set_pixel(Position[i], colors[i][0], colors[i][1], colors[i][2]);

			if (ballBouncing[i]) {
				ballsStillBouncing = true;
			}
		}
		sw_ws2812_send_buff();
		delay_ms(2);

		if ( stop_effect() ) return;
		setAll(0, 0, 0);
	}
}
#endif
#ifdef	METEOR_RAIN
// used by meteor-rain
static void fadeToBlack(int ledNo, uint8_t fadeValue) {
	uint8_t r, g, b;
	T_WS2812_RGB pixel;
	ws2812_get_pixel( ledNo, &pixel );
	r = pixel.R;
	b = pixel.B;
	g = pixel.G;

	r = (r<=10)? 0 : (int) r-(r*fadeValue/256);
	g = (g<=10)? 0 : (int) g-(g*fadeValue/256);
	b = (b<=10)? 0 : (int) b-(b*fadeValue/256);

	ws2812_set_pixel( ledNo, r, g, b );
}
static void meteorRain( uint8_t red, uint8_t green, uint8_t blue, uint8_t meteorSize,
						uint8_t meteorTrailDecay, bool meteorRandomDecay, int SpeedDelay) {
	setAll(0, 0, 0);
	for (uint16_t i = 0; i < WS_LED_CNT + WS_LED_CNT; i++) {
		// fade brightness all LEDs one step
		for (uint16_t j = 0; j < WS_LED_CNT; j++) {
			if ((!meteorRandomDecay) || (random1(10) > 5)) {
				fadeToBlack(j, meteorTrailDecay);
			}
		}
		// draw meteor
		for (uint16_t j = 0; j < meteorSize; j++) {
			if ((i - j < WS_LED_CNT) && (i - j >= 0)) {
				ws2812_set_pixel(i - j, red, green, blue);
			}
		}
		sw_ws2812_send_buff();
		delay_ms(SpeedDelay);

		if ( stop_effect() ) return;
	}
}
#endif


void changeEffect() {
//	if ( sw_ir_return_code()->command == Menu_Up) {
//		selectedEffect++;
//	}
//	else if (sw_ir_return_code()->command == Menu_Down) {
//		selectedEffect--;
//	}
//	else if (sw_ir_return_code()->command == Power) {
//		selectedEffect = 19;
//	}
	if(selectedEffect == 5) 	selectedEffect = 0;
	if(selectedEffect == 255) 	selectedEffect = 4;
	stopEffect = true;
}

void show_effects(void) {
	switch ( selectedEffect ) {
		case 0: {
#ifdef RGB_LOOP
			// RGBLoop - no parameters
			RGBLoop();
#endif
			break;
		}
		case 1: {
#ifdef FADE_INOUT
			// FadeInOut - Color (red, green. blue)
			FadeInOut( 0xff, 0x00, 0x00 ); // red
//			FadeInOut( 0xff, 0xff, 0xff ); // white
//			FadeInOut( 0x00, 0x00, 0xff ); // blue
#endif
			break;
		}
		case 2: {
#ifdef STROBE
			// Strobe - Color (red, green, blue), number of flashes, flash speed, end pause
			Strobe(0xff, 0xff, 0xff, 10, 50, 1000);
#endif
			break;
		}
		case 3: {
#ifdef HALLOWEEN_EYES
			// HalloweenEyes - Color (red, green, blue), Size of eye, space between eyes, fade (true/false), steps, fade delay, end pause
			HalloweenEyes(0xff, 0x00, 0x00, 3, 4, true, random2(5, 50), random2(10, 100), random2(1000, 10000));
#endif
			break;
		}
		case 4: {
#ifdef CYLON_BOUNCE
			// CylonBounce - Color (red, green, blue), eye size, speed delay, end pause
			CylonBounce(0xff, 0x00, 0x00, 4, 10, 50);
#endif
			break;
		}
		case 5: {
#ifdef NEW_KITT
			// NewKITT - Color (red, green, blue), eye size, speed delay, end pause
			NewKITT(0xff, 0x00, 0x00, 8, 10, 50);
#endif
			break;
		}
		case 6: {
#ifdef TWINKLE
			// Twinkle - Color (red, green, blue), count, speed delay, only one twinkle (true/false)
			Twinkle(0xff, 0x00, 0x00, 10, 100, false);
#endif
			break;
		}
		case 7: {
#ifdef TWINKLE_RANDOM
			// TwinkleRandom - twinkle count, speed delay, only one (true/false)
			TwinkleRandom(20, 100, false);
#endif
			break;
		}
		// Chyba nie działa...
		case 8: {
#ifdef SPARKLE
			// Sparkle - Color (red, green, blue), speed delay
			Sparkle(0xff, 0xff, 0xff, 3);
#endif
			break;
		}
		case 9: {
#ifdef SNOW_SPARKLE
			// SnowSparkle - Color (red, green, blue), sparkle delay, speed delay
			SnowSparkle(0x2, 0x2, 0x2, 20, random2(100, 1000));
#endif
			break;
		}
		case 10: {
#ifdef RUNNING_LIGHTS
			// Running Lights - Color (red, green, blue), wave dealy
			RunningLights( 0xff, 0x00, 0x00, 50 ); // red
			RunningLights( 0xff, 0xff, 0xff, 50 ); // white
			RunningLights( 0x00, 0x00, 0xff, 50 ); // blue
#endif
			break;
		}
		case 11: {
#ifdef COLOR_WIPE
			// colorWipe - Color (red, green, blue), speed delay
			colorWipe(0x00, 0xff, 0x00, 50);
			colorWipe(0x00, 0x00, 0x00, 50);
#endif
			break;
		}
		case 12: {
#ifdef RAINBOW_CYCLE
			// rainbowCycle - speed delay
			rainbowCycle(20);
#endif
			break;
		}
		case 13: {
#ifdef THEATER_CHASE
			// theatherChase - Color (red, green, blue), speed delay
			theaterChase(0xff, 0, 0, 50);

#endif
			break;
		}
		case 14: {
#ifdef THEATER_CHASERAINBOW
			// theaterChaseRainbow - Speed delay
			theaterChaseRainbow(50);
#endif
			break;
		}
		case 15: {
#ifdef FIRE
			// Fire - Cooling rate, Sparking rate, speed delay
			Fire(55, 120, 15);
#endif
			break;
		}
			// simple bouncingBalls not included, since BouncingColoredBalls can perform this as well as shown below
			// BouncingColoredBalls - Number of balls, color (red, green, blue) array, continuous
			// CAUTION: If set to continuous then this effect will never stop!!!
		case 16: {
#ifdef BOUNCING_COLORED_BALLS
			// mimic BouncingBalls
			uint8_t onecolor[1][3] = {
				{ 0xff, 0x00, 0x00 }
			};
			BouncingColoredBalls( 1, onecolor, false );
#endif
			break;
		}
		case 17: {
#ifdef BOUNCING_COLORED_3BALLS
			// multiple colored balls
			BouncingColoredBalls( 3, bounceColors, false );
#endif
			break;
		}
		case 18: {
#ifdef METEOR_RAIN
			// meteorRain - Color (red, green, blue), meteor size, trail decay, random trail decay (true/false), speed delay
			meteorRain(0xff, 0xff, 0xff, 10, 64, true, 30);
#endif
			break;
		}
		case 19:
			stop_effect();
			break;
		default:
			break;
	}
}

