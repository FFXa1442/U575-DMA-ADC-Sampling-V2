/*
 * uart_adc.h
 *
 *  Created on: Apr 23, 2025
 *      Author:
 */

#ifndef __UART_ADC_H
#define __UART_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32u5xx_hal.h"

typedef enum
{
	UART_ADC_OK = 0x00,
	UART_ADC_ERROR = 0x01,
	UART_ADC_BUSY = 0x02,
    UART_ADC_TIMEOUT = 0x03,

    UART_ADC_OUT_OF_MEMORY = 0x04,
    UART_ADC_ARGUMENT_OUT_OF_RANGE = 0x05,

    UART_ADC_MEMORY_ALLOCATE_FAILED = 0x06,

    UART_ADC_FAILED = 0xFF,
} UART_ADC_Result;

#ifdef __cplusplus
}
#endif

#endif /* __UART_ADC_H */
