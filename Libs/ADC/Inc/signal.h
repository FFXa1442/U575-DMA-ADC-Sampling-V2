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
    ANA_RP_BYTE = 0x00,
    ANA_RP_HALF_WORD = 0x01,
    ANA_RP_WORD = 0x02,
} AnaRP_DataType;

typedef enum
{
	ANA_RP_OK = 0x00,
	ANA_RP_ERROR = 0x01,
	ANA_RP_BUSY = 0x02,
    ANA_RP_TIMEOUT = 0x03,

    ANA_RP_OUT_OF_MEMORY = 0x04,
    ANA_RP_ARGUMENT_OUT_OF_RANGE = 0x05,

    ANA_RP_MEMORY_ALLOCATE_FAILED = 0x06,

    ANA_RP_FAILED = 0xFF,
} AnaRP_Result;

extern AnaRP_Result AnaRP_Init(ADC_HandleTypeDef *hadc, const uint16_t dma_size, const AnaRP_DataType data_type);

extern AnaRP_Result AnaRP_DeInit(ADC_HandleTypeDef *hadc);

extern AnaRP_Result AnaRP_GetData(ADC_HandleTypeDef *hadc, uint8_t *buffer, uint16_t *size, uint32_t *data_size, uint8_t *type_size);

extern AnaRP_Result AnaRP_Start_DMA(ADC_HandleTypeDef *hadc);

extern AnaRP_Result AnaRP_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc, uint8_t *dst, uint16_t *size);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_SIGNAL_H */
