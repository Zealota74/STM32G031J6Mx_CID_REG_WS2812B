/*
 * gpio.h
 *
 *  Created on: 06.07.2017
 *      Author: simon
 */

#ifndef LIBS_SW_GPIO_GPIO_H_
#define LIBS_SW_GPIO_GPIO_H_


#include "gpio_defs.h"
#include "STM32G0316_DISCO.h"

extern void gpio_pin_cfg( GPIO_TypeDef * const port, T_GPIO_PIN pin,
						  T_GPIO_MODE mode);
extern void gpio_pin_LO( GPIO_TypeDef * const port, T_GPIO_PIN pin );
extern void gpio_pin_HI( GPIO_TypeDef * const port, T_GPIO_PIN pin );
extern void gpio_pin_XOR( GPIO_TypeDef * const port, T_GPIO_PIN pin );
extern void RCC_gpio_init( void );

static inline uint8_t gpio_pin_READ( GPIO_TypeDef * const port, T_GPIO_PIN pin ) {
//	if ( (port->IDR & pin) ) return 1;
	if ( !(port->IDR & pin) ) return 1;
	else return 0;
}


// BitBanging
enum { SRAM_BB_REGION_START = 0x20000000 };
enum { SRAM_BB_REGION_END = 0x200fffff };
enum { SRAM_BB_ALIAS = 0x22000000 };
enum { PERIPH_BB_REGION_START = 0x40000000 };
enum { PERIPH_BB_REGION_END = 0x400fffff };
enum { PERIPH_BB_ALIAS = 0x42000000 };
#define SRAM_ADR_COND(adres) ( (uint32_t)&adres >= SRAM_BB_REGION_START && (uint32_t)&adres <= SRAM_BB_REGION_END )
#define PERIPH_ADR_COND(adres) ( (uint32_t)&adres >= PERIPH_BB_REGION_START && (uint32_t)&adres <= PERIPH_BB_REGION_END )
#define BB_SRAM2(adres, bit)( SRAM_BB_ALIAS + ((uint32_t)&adres - SRAM_BB_REGION_START)*32u + (uint32_t)(bit*4u) )
#define BB_PERIPH(adres, bit)( PERIPH_BB_ALIAS + ((uint32_t)&adres - PERIPH_BB_REGION_START)*32u + (uint32_t)(__builtin_ctz(bit))*4u)
/* bit - bit mask, not bit position! */
#define BB(adres, bit) *(__IO uint32_t *)( SRAM_ADR_COND(adres) ? BB_SRAM2(adres, bit) : \
( PERIPH_ADR_COND(adres) ? BB_PERIPH(adres, bit) : 0 ))
#define BB_SRAM(adres, bit) *(__IO uint32_t *)BB_SRAM2(adres, bit)




#endif /* LIBS_SW_GPIO_GPIO_H_ */
