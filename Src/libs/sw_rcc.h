/*
 * sw_rcc.h
 *
 *  Created on: Jul 19, 2021
 *      Author: simon
 */

#ifndef SRC_LIBS_SW_RCC_H_
#define SRC_LIBS_SW_RCC_H_

#ifdef STM32G031xx
#define	RCC_CFGR_HPRE_DIV1		0

#define	RCC_CFGR_PPRE_DIV1		0
#define	RCC_CFGR_PPRE_DIV2		(RCC_CFGR_PPRE_2)
#define	RCC_CFGR_PPRE_DIV4		(RCC_CFGR_PPRE_2 | RCC_CFGR_PPRE_0)
#define	RCC_CFGR_PPRE_DIV8		(RCC_CFGR_PPRE_2 | RCC_CFGR_PPRE_1)
#define	RCC_CFGR_PPRE_DIV16		(RCC_CFGR_PPRE_2 | RCC_CFGR_PPRE_1 | RCC_CFGR_PPRE_0)

#define FLASH_ACR_LATENCY_0WS	0
#define FLASH_ACR_LATENCY_1WS	FLASH_ACR_LATENCY_0
#define FLASH_ACR_LATENCY_2WS	FLASH_ACR_LATENCY_1

#define RCC_CFGR_SW_HSI			0
#define RCC_CFGR_SW_HSE			RCC_CFGR_SW_0
#define RCC_CFGR_SW_LSI			(RCC_CFGR_SW_0 | RCC_CFGR_SW_1)
#define RCC_CFGR_SW_PLL			RCC_CFGR_SW_1
#define RCC_CFGR_SW_LSE			RCC_CFGR_SW_2

#endif

extern void SystemClock_Config(void);

#endif /* SRC_LIBS_SW_RCC_H_ */
