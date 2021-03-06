/*
 * sw_dma.h
 *
 *  Created on: Jan 21, 2021
 *      Author: szymon.wroblewski
 */

#ifndef LIBS_SW_WS2812_SW_DMA_H_
#define LIBS_SW_WS2812_SW_DMA_H_

enum {
	dmamux_req_gen0 = 1,
	dmamux_req_gen1 = 2,
	dmamux_req_gen2 = 3,
	dmamux_req_gen3 = 4,

	DMAMUX_ADC 			= 5,
	DMAMUX_AES_IN 		= 6,
	DMAMUX_AES_OUT 		= 7,
	DMAMUX_DAC_Channel1 = 8,
	DMAMUX_DAC_Channel2 = 9,
	DMAMUX_I2C1_RX 		= 10,
	DMAMUX_I2C1_TX 		= 11,
	DMAMUX_I2C2_RX 		= 12,
	DMAMUX_I2C2_TX 		= 13,
	DMAMUX_LPUART_RX 	= 14,
	DMAMUX_LPUART_TX 	= 15,
	DMAMUX_SPI1_RX 		= 16,
	DMAMUX_SPI1_TX 		= 17,
	DMAMUX_SPI2_RX 		= 18,
	DMAMUX_SPI2_TX 		= 19,
	DMAMUX_TIM1_CH1 	= 20,
	DMAMUX_TIM1_CH2 	= 21,
	DMAMUX_TIM1_CH3 	= 22,
	DMAMUX_TIM1_CH4 	= 23,
	DMAMUX_TIM1_TRIG_COM = 24,
	DMAMUX_TIM1_UP 		= 25,
	DMAMUX_TIM2_CH1 	= 26,
	DMAMUX_TIM2_CH2 	= 27,
	DMAMUX_TIM2_CH3 	= 28,
	DMAMUX_TIM2_CH4 	= 29,
	DMAMUX_TIM2_TRIG 	= 30,
	DMAMUX_TIM2_UP 		= 31,
	DMAMUX_TIM3_CH1 	= 32,
	DMAMUX_TIM3_CH2 	= 33,
	DMAMUX_TIM3_CH3 	= 34,
	DMAMUX_TIM3_CH4 	= 35,
	DMAMUX_TIM3_TRIG 	= 36,
	DMAMUX_TIM3_UP 		= 37,
	DMAMUX_TIM6_UP 		= 38,
	DMAMUX_TIM7_UP 		= 39,
	DMAMUX_TIM15_CH1 	= 40,
	DMAMUX_TIM15_CH2 	= 41,
	DMAMUX_TIM15_TRIG_COM = 42,
	DMAMUX_TIM15_UP 	= 43,
	DMAMUX_TIM16_CH1 	= 44,
	DMAMUX_TIM16_COM 	= 45,
	DMAMUX_TIM16_UP 	= 46,
	DMAMUX_TIM17_CH1 	= 47,
	DMAMUX_TIM17_COM 	= 48,
	DMAMUX_TIM17_UP 	= 49,
	DMAMUX_USART1_RX 	= 50,
	DMAMUX_USART1_TX 	= 51,
	DMAMUX_USART2_RX 	= 52,
	DMAMUX_USART2_TX 	= 53,
	DMAMUX_USART3_RX 	= 54,
	DMAMUX_USART3_TX 	= 55,
	DMAMUX_USART4_RX 	= 56,
	DMAMUX_USART4_TX 	= 57,
	DMAMUX_UCPD1_RX 	= 58,
	DMAMUX_UCPD1_TX 	= 59,
	DMAMUX_UCPD2_RX 	= 60,
	DMAMUX_UCPD2_TX 	= 61,
};

#endif /* LIBS_SW_WS2812_SW_DMA_H_ */
