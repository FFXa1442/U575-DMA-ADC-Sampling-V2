/*
 * spi_adc.h
 *
 *  Created on: Apr 22, 2025
 *      Author:
 */

#ifndef __SPI_ADC_H
#define __SPI_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32u5xx_hal.h"

typedef enum
{
	SPI_ADC_OK = 0x00,
	SPI_ADC_ERROR = 0x01,
	SPI_ADC_BUSY = 0x02,
    SPI_ADC_TIMEOUT = 0x03,

    SPI_ADC_OUT_OF_MEMORY = 0x04,
    SPI_ADC_ARGUMENT_OUT_OF_RANGE = 0x05,

    SPI_ADC_MEMORY_ALLOCATE_FAILED = 0x06,

    SPI_ADC_FAILED = 0xFF,
} SPI_ADC_Result;

extern SPI_ADC_Result SPI_ADC_Init(SPI_HandleTypeDef *hspi, ADC_HandleTypeDef *hadc, uint8_t **ptr_rx_buffer, const uint8_t ack_code);

extern SPI_ADC_Result SPI_ADC_DeInit(SPI_HandleTypeDef *hspi, ADC_HandleTypeDef *hadc);

extern SPI_ADC_Result SPI_ADC_GetBuffer(SPI_HandleTypeDef *hspi, ADC_HandleTypeDef *hadc, uint8_t **ptr_tx_buffer, uint32_t *size);

extern SPI_ADC_Result SPI_ADC_ValidateBuffer(SPI_HandleTypeDef *hspi, ADC_HandleTypeDef *hadc, uint8_t *rx_buffer);

#ifdef __cplusplus
}
#endif

#endif /* __SPI_ADC_H */
