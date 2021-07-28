/*
 * sw_ws2812.h
 *
 *  Created on: 26.05.2019
 *      Author: simon
 */

#ifndef LIBS_SW_WS2812_SW_WS2812_H_
#define LIBS_SW_WS2812_SW_WS2812_H_

#define WS_LED_CNT	60
#define SPI_DMA

typedef struct {
	uint8_t G;
	uint8_t R;
	uint8_t B;
} T_WS2812_RGB;

#define BASE_EFFECT1
#define BASE_EFFECT2
#define BASE_EFFECT3
#define BASE_EFFECT4

static inline void sw_ws2812_send_buff(void) {
	DMA1_Channel3->CCR |= DMA_CCR_EN;
}
void sw_ws2812_init(void);
void ws2812_base_efekt1(void);
uint16_t ws2812_base_efekt2(void);
void ws2812_base_efekt3(void);
void ws2812_base_efekt4(void);

void changeEffect(void);
void show_effects(void);

void sw_send_spi_byte( uint8_t byte );
void ws2812_set_pixel(int Pixel, uint8_t red, uint8_t green, uint8_t blue);
void ws2812_get_pixel( int pixelNo, T_WS2812_RGB * pixel);

#endif /* LIBS_SW_WS2812_SW_WS2812_H_ */
