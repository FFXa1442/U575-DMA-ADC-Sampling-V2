/*
 * signal.h
 *
 *  Created on: Apr 22, 2025
 *      Author:
 */

#ifndef __ADC_SIGNAL_H
#define __ADC_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32u5xx_hal.h"

typedef enum
{
    ADC_BYTE = 0x00,
    ADC_HALF_WORD = 0x01,
    ADC_WORD = 0x02,
} ADC_DataType;

typedef enum
{
	ADC_OK = 0x00,
	ADC_ERROR = 0x01,
	ADC_BUSY = 0x02,
    ADC_TIMEOUT = 0x03,

    ADC_OUT_OF_MEMORY = 0x04,
    ADC_ARGUMENT_OUT_OF_RANGE = 0x05,

    ADC_MEMORY_ALLOCATE_FAILED = 0x06,

    ADC_FAILED = 0xFF,
} ADC_Result;

extern ADC_Result ADC_Init(ADC_HandleTypeDef *hadc, const uint16_t dma_size, const ADC_DataType data_type);

extern ADC_Result ADC_DeInit(ADC_HandleTypeDef *hadc);

extern ADC_Result ADC_Get(ADC_HandleTypeDef *hadc, uint8_t *buffer, uint16_t *size, uint32_t *data_size, uint8_t *type_size);

extern ADC_Result ADC_Start_DMA(ADC_HandleTypeDef *hadc);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_SIGNAL_H */
