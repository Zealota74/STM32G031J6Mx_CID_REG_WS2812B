/* USER CODE BEGIN Header */
/**
 */

#include "sw_mcu_conf.h"
#include "libs/SW_BOARD/gpio.h"
#include "libs/SW_BOARD/sw_led_blink_debug.h"
#include "libs/SW_TIMERS/sw_soft_timers.h"
#include "libs/SW_WS2812/sw_ws2812.h"


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);


int main(void) {
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

