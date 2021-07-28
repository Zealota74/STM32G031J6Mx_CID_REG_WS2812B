/*
 * sw_rcc.c
 *
 *  Created on: Jul 19, 2021
 *      Author: simon
 */
#include "sw_mcu_conf.h"
#include "sw_rcc.h"


/**********************************************************************/
static INLINE void rcc_FlashLatency_set( uint32_t latency ) {
	// Select flash latency
	MODIFY_REG( FLASH->ACR, FLASH_ACR_LATENCY, latency );
	//wait for latency set flag
	while ( (FLASH->ACR & FLASH_ACR_LATENCY) == 0 ) {}
}
/**********************************************************************/

/**********************************************************************/
static INLINE void pwr_VoltScale_sel(void) {
	// Set voltage scaling to range 1
	MODIFY_REG( PWR->CR1, PWR_CR1_VOS, PWR_CR1_VOS_0 );
	// Wait for voltage scaling flag
	while ( (PWR->SR2 & PWR_SR2_VOSF) ) {}
}
/**********************************************************************/

/**********************************************************************/
static INLINE void rcc_ClockSource_on( uint32_t clock ) {
	uint32_t checkClock, reg;
	switch ( clock ) {
		case RCC_CR_HSION:
			checkClock = RCC_CR_HSIRDY;
			reg = RCC_CR_CSSON | RCC_CR_HSION;
			break;
		case RCC_CR_HSEON:
			checkClock = RCC_CR_HSERDY;
			reg = RCC_CR_CSSON | RCC_CR_HSEBYP | RCC_CR_HSEON;
			break;
		default:
			break;
	}
	//turn on HSE external, HSE bypass and security
	SET_BIT( RCC->CR, reg ) ;
	//wait for the HSE to be ready
	while ( (RCC->CR & checkClock ) == 0 ) {}
}
/**********************************************************************/

/**********************************************************************/
static INLINE void rcc_PllMull_set(void) {
	// CLK = (HSI / PLLM) * PLLN / PLLR = 16 / 1 * 8 / 2 = 16 * 4 = 64 MHz
	uint16_t pllm = 1;			// 1, 2 - 8
	uint16_t plln = 8;			// 8, 9 - 86
	uint16_t pllr = 2;			// 2, 4, 6, 8,
	pllr = (uint8_t)pllr - 1;
	// reset and configure pll mull and div settings, and PLL source
	MODIFY_REG( RCC->PLLCFGR,
				RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLR,
				 ((pllm - 1)	<<	RCC_PLLCFGR_PLLM_Pos )
				|( plln			<<	RCC_PLLCFGR_PLLN_Pos )
				|( pllr			<<	RCC_PLLCFGR_PLLR_Pos )
			  );
}
/**********************************************************************/

/**********************************************************************/
static INLINE void rcc_PllSourceMux_switch( uint32_t source ) {
	SET_BIT( RCC->PLLCFGR, source );
}
/**********************************************************************/
/**********************************************************************/
static INLINE void rcc_SystemClockMux_switch( uint32_t pllSource ) {
	uint32_t sourceCheck;
	switch ( pllSource ) {
		case RCC_CFGR_SW_PLL:
			sourceCheck = RCC_CFGR_SWS_PLL;
			break;
		case RCC_CFGR_SW_HSI:
			sourceCheck = RCC_CFGR_SWS_HSI;
			break;
		case RCC_CFGR_SW_HSE:
			sourceCheck = RCC_CFGR_SWS_HSE;
			break;
		default:
			break;
	}
	// Set PLL as system clock
	SET_BIT( RCC->CFGR, pllSource);
	while ( (RCC->CFGR & sourceCheck) == 0 ) {}
}
/**********************************************************************/

/**********************************************************************/
static INLINE void rcc_Pll_on(void) {
	//turn on PLL, wait for ready
	SET_BIT( RCC->CR, RCC_CR_PLLON);
	SET_BIT( RCC->PLLCFGR, RCC_PLLCFGR_PLLREN );
	// wait for pll to ready
	while (((RCC->CR) & RCC_CR_PLLRDY) == 0) {}
}
/**********************************************************************/

/**********************************************************************/
static INLINE void rcc_SystemClock_bus_presscaler( void ) {
	MODIFY_REG( RCC->CFGR,
				RCC_CFGR_PPRE
			   |RCC_CFGR_PPRE
			   |RCC_CFGR_HPRE,
			    RCC_CFGR_PPRE_DIV1
			   |RCC_CFGR_HPRE_DIV1
			 );
}
/**********************************************************************/
/**********************************************************************/
void SystemClock_Config(void) {
	rcc_FlashLatency_set( FLASH_ACR_LATENCY_2WS );
	pwr_VoltScale_sel();

	rcc_ClockSource_on( RCC_CR_HSION );

	rcc_PllMull_set();
	rcc_PllSourceMux_switch( RCC_PLLCFGR_PLLSRC_HSI );
	rcc_Pll_on();

	rcc_SystemClock_bus_presscaler();

	rcc_SystemClockMux_switch( RCC_CFGR_SW_PLL );

	SystemCoreClockUpdate();
	SysTick_Config( SystemCoreClock / 1000 );	// Systick on 1 ms
}
/**********************************************************************/
