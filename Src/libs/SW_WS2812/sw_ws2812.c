/*
 * sw_ws2812.c
 *
 *  Created on: 26.05.2019
 *      Author: simon
 */
#include <stdlib.h>

#include "sw_mcu_conf.h"
#include "../SW_BOARD/gpio.h"
#include "../SW_TIMERS/sw_soft_timers.h"

#include "math.h"
#include "sw_ws2812.h"
#include "sw_dma.h"

static T_WS2812_RGB LedsBuffer[ WS_LED_CNT ];

static void spi_init(void) {
	//================ SPI ================
	RCC->APBENR2 |= RCC_APBENR2_SPI1EN;

	SPI1->CR1  = SPI_CR1_MSTR				// Select as master
				|SPI_CR1_BR_2|SPI_CR1_BR_0	// CLK/64 = 48 MHz / 64 = 0,75 MHz = 750 KHz
				|SPI_CR1_CPHA				// Clock Phase
				;
	SPI1->CR2  = SPI_CR2_SSOE				// SS Output Enable
#ifdef SPI_DMA
				|SPI_CR2_TXDMAEN			// TX Buffer DMA Enable
#endif
				;
	SPI1->CR1 |= SPI_CR1_SPE				// SPI enable
				;
	//======================================
}
static void tim3_init(void) {
	//================ TIM3 ================
	RCC->APBENR1 |= RCC_APBENR1_TIM3EN;
	TIM3->PSC  = 18 - 1;	// 48 MHz / 18 = 2666 KHz
	TIM3->ARR  = 4  - 1;	// 2666 KHz / 4 = 666 666 Hz -> 1.5 us
	TIM3->CCR3 = 2;			// 1.5 us / 2 = 750 ns
	TIM3->CCR4 = 2;			// TIM capture/compare register 4
												// Capture 1 Selection
	TIM3->CCMR1 = TIM_CCMR1_CC1S_0;				// Channel_1 is configured as input, IC1 is mapped on TI1
	TIM3->SMCR =
				 TIM_SMCR_TS_2 |TIM_SMCR_TS_0 	// 101: Filtered Timer Input 1 (TI1FP1)
				|TIM_SMCR_SMS_2|TIM_SMCR_SMS_1 	// 110: Trigger Mode
				;
	TIM3->CCER = TIM_CCER_CC1P | TIM_CCER_CC1NP	// 0x11, non-inverted/both edges. The circuit is sensitive to both TIxFP1 rising and falling edges
				;


//	TIM3->CCMR2 = TIM_CCMR2_IC1F_2 | TIM_CCMR2_IC1F_1;	// 0110: fSAMPLING = fDTS / 4, N = 6									// Channel 4
	TIM3->CCMR1 = TIM_CCMR1_IC1F_2 | TIM_CCMR1_IC1F_1;	// ???

	TIM3->CCER |= TIM_CCER_CC3E		// Compare 3 output enable
				 |TIM_CCER_CC4E		// Compare 4 output enable
				;

//	TIM3->DIER = TIM_DIER_CC3DE;	// Capture 3 DMA request enable
	TIM3->DIER = TIM_DIER_TDE;		// Trigger DMA request enable

	TIM3->CR1  = TIM_CR1_OPM		// One pulse mode
				|TIM_CR1_DIR		// Direction down counter
				|TIM_CR1_CEN;		// Counter enable
}
static void dma_init(void) {
	//================ DMA ================
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA1_Channel2->CCR 		= 0;
	DMA1_Channel2->CNDTR 	= 1;
	DMA1_Channel2->CPAR 	= (uint32_t)&GPIOF->IDR;
	DMA1_Channel2->CMAR 	= (uint32_t)&TIM3->CNT;
	DMA1_Channel2->CCR		= DMA_CCR_MSIZE_0 | DMA_CCR_CIRC | DMA_CCR_EN;
	DMAMUX1_Channel2->CCR  &= ~( DMAMUX_CxCR_DMAREQ_ID );							// DMA Request ID
	DMAMUX1_Channel2->CCR  |=  ( DMAMUX_TIM3_TRIG << DMAMUX_CxCR_DMAREQ_ID_Pos );


#ifdef SPI_DMA
// SPI DMA settings
	DMA1_Channel3->CNDTR 	= 3*WS_LED_CNT;//sizeof(LedsBuffer);
	DMA1_Channel3->CMAR 	= (uint32_t)(uint8_t *)LedsBuffer;
	DMA1_Channel3->CPAR 	= (uint32_t)( (volatile uint8_t *)&SPI1->DR );
	DMA1_Channel3->CCR 		=
							  DMA_CCR_MINC		// Memory increment mode
//							 |DMA_CCR_MSIZE_0	// 8 bit transfer
//							 |DMA_CCR_PSIZE_0	// 8 bit transfer
							 |DMA_CCR_DIR		// Read from memory to peripheral
//							 |DMA_CCR_CIRC
//							 |DMA_CCR_EN
							 |DMA_CCR_TCIE		// Transfer complete interrupt enable
							 ;

	DMAMUX1_Channel3->CCR &= ~( DMAMUX_CxCR_DMAREQ_ID );			// DMA Request ID
	DMAMUX1_Channel3->CCR |=  ( DMAMUX_SPI1_TX << DMAMUX_CxCR_DMAREQ_ID_Pos );

	NVIC_EnableIRQ( DMA1_Channel2_3_IRQn );
#endif
}

void DMA1_Channel2_3_IRQHandler(void) {
	if ( DMA1->ISR & DMA_ISR_TCIF3 ) {
		DMA1->IFCR = DMA_IFCR_CTCIF3;
		DMA1_Channel3->CCR &= ~DMA_CCR_EN;
		DMA1_Channel3->CNDTR = 3*WS_LED_CNT;
	}
}

void ws2812effect_base_timer( void ) {
	RCC->APBENR2 |= RCC_APBENR2_TIM16EN;
	TIM16->CR1 = 0;
	TIM16->PSC = 4800 - 1;			// 48 MHz / 4800 = 1000 Hz
	TIM16->ARR = 10	  - 1;			// 1000 Hz/ 1 = 1 KHz -> 1ms

	TIM16->DIER = TIM_DIER_UIE;		// Update interrupt enable
	TIM16->CR1  =
				 TIM_CR1_CEN		// Counter enable
				;
	NVIC_EnableIRQ( TIM16_IRQn );
}



void sw_ws2812_init(void) {
//	RCC->AHBENR |= RCC_AHBENR_GPIOFEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOAEN;

	gpio_pin_cfg( PORTB, PB1, gpio_mode_AF1_PP_LS );	// TIM3_CH4 - WS2812B output pin

	gpio_pin_cfg( PORTA, PA5, gpio_mode_AF0_PP_LS );	// SPI1_SCK
	gpio_pin_cfg( PORTB, PB5, gpio_mode_AF0_PP_LS );	// SPI1_MOSI

	gpio_pin_cfg( PORTA, PA6, gpio_mode_AF1_PP_LS );	// 	TIM3_CH1
	gpio_pin_cfg( PORTF, PF2, gpio_mode_in_PD );		// 	FP1 always 0

	dma_init();
	spi_init();
	tim3_init();
	ws2812effect_base_timer();
}

// Set a LED color (not yet visible)
void ws2812_set_pixel(int Pixel, uint8_t red, uint8_t green, uint8_t blue) {
	LedsBuffer[Pixel].R = red;
	LedsBuffer[Pixel].G = green;
	LedsBuffer[Pixel].B = blue;
}
void ws2812_get_pixel( int pixelNo, T_WS2812_RGB * pixel) {
	*pixel = LedsBuffer[pixelNo];
}

void sw_ws2812_send_color( uint8_t byte ) {
	while((SPI1->SR & SPI_SR_TXE)==0);
	*(volatile uint8_t *)&SPI1->DR = byte;
}


/***************************** Base effects **********************************************/
void ws2812_base_efekt1(void){
	uint16_t i, j=0, color;

	while(1){
		color = j;
		for(i=0; i<WS_LED_CNT; i++){
			if(color<256){
				sw_ws2812_send_color((color&255));
				sw_ws2812_send_color(255-(color&255));
				sw_ws2812_send_color(0);
			}
			else if(color<512){
				sw_ws2812_send_color(255-(color&255));
				sw_ws2812_send_color(0);
				sw_ws2812_send_color((color&255));
			}
			else if(color<768){
				sw_ws2812_send_color(0);
				sw_ws2812_send_color((color&255));
				sw_ws2812_send_color(255-(color&255));
			}
			color = (color+32)%768;
		}
		j = (j+16)%768;
		delay_ms(20);
	}
}
uint16_t ws2812_base_efekt2(void) {
	static uint16_t j=0;
	static uint16_t color;

	color = j;
	for ( uint16_t i = 0; i < WS_LED_CNT; i++) {
		if (color < 256) {
			LedsBuffer[i].G = (color & 255);
			LedsBuffer[i].R = 255 - (color & 255);
			LedsBuffer[i].B = 0;
		} else if (color < 512) {
			LedsBuffer[i].G = 255 - (color & 255);
			LedsBuffer[i].R = 0;
			LedsBuffer[i].B = (color & 255);
		} else if (color < 768) {
			LedsBuffer[i].G = 0;
			LedsBuffer[i].R = color & 255;
			LedsBuffer[i].B = 255 - (color & 255);
		}
		color = (color + 32) % 768;
	}
	j = (j + 16) % 768;

	sw_ws2812_send_buff();
	return 20;				// ms
}
void ws2812_base_efekt3(void) {
	for (uint8_t i=0; i<WS_LED_CNT; i++) {
		sw_ws2812_send_color(1);
		sw_ws2812_send_color(1);
		sw_ws2812_send_color(1);
	}
	delay_ms( 5000 );
	for (uint8_t i=0; i<24; i++) {
		sw_ws2812_send_color(0);
		sw_ws2812_send_color(0);
		sw_ws2812_send_color(0);
	}
	delay_ms( 1000 );
}
void ws2812_base_efekt4(void) {
	for (uint8_t i=0; i<WS_LED_CNT-10; i++) {
		LedsBuffer[i].G = 0;
		LedsBuffer[i].R = 0;
		LedsBuffer[i].B = 255;
	}
}
/**************************************************************************/
