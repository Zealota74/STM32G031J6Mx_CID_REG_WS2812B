/* USER CODE BEGIN Header */
/**
 */

#include "sw_mcu_conf.h"
#include "libs/SW_BOARD/gpio.h"
#include "libs/SW_BOARD/sw_led_blink_debug.h"
#include "libs/SW_TIMERS/sw_soft_timers.h"
#include "libs/SW_WS2812/sw_ws2812.h"

#include "libs/sw_i2c_simple_v2.h"
#include "libs/test.h"


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);


int main(void) {

	simon.a	= 5;
	simon.b = 5;

	set_variable( 120 );
	zmienna_init2();

	SystemClock_Config();
	RCC_gpio_init();
	sw_led_debug_init();

	sw_ws2812_init();

	while (1) {
		if (softTimer5 == 0) {
			softTimer5 = 500;
			sw_led_blink();
		}
	}

}

