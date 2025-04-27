/*
 * processing.h
 *
 *  Created on: Apr 27, 2025
 *      Author: XQuan
 */

#ifndef __PROCESSING_H
#define __PROCESSING_H

#define USER_UART_MODE
// #define USER_SPI_MODE

#define ADC_SAMPLING_SIZE 5000
#define UART_RX_SIZE 4

#define UART_RX_FB_ID ((uint32_t) (214802728))

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32u5xx_hal.h"

typedef enum
{
    PROCESSING_OK = 0x00,
	PROCESSING_ERROR = 0x01,
	PROCESSING_BUSY = 0x02,
    PROCESSING_TIMEOUT = 0x03,

    PROCESSING_OUT_OF_MEMORY = 0x04,
    PROCESSING_ARGUMENT_OUT_OF_RANGE = 0x05,

    PROCESSING_MEMORY_ALLOCATE_FAILED = 0x06,

    PROCESSING_FAILED = 0xFF,
} Processing_Status;

typedef struct
{
    UART_HandleTypeDef *huart;

    ADC_HandleTypeDef *hadc;

    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *SPI_CS_GPIOx;
    uint16_t SPI_CS_Pin;

    GPIO_TypeDef *SPI_FB_GPIOx;
    uint16_t SPI_FB_Pin;

    uint8_t *tx_buffer;
    uint32_t tx_size;

    uint8_t rx_buffer[UART_RX_SIZE];
    uint32_t rx_size;
    
} Processing_Handle_t;

typedef struct
{
    union
    {
        uint16_t SPI_FB_Pin;
        UART_HandleTypeDef *huart;
        SPI_HandleTypeDef *hspi;
        ADC_HandleTypeDef *hadc;
    };
} Processing_Callback_Handle_t;

extern Processing_Status Processing_Init(Processing_Handle_t *handle);

extern Processing_Status Processing_Start_DMA(Processing_Handle_t *handle);

extern Processing_Status Processing_ADC_ConvCpltCallback(Processing_Handle_t *handle, Processing_Callback_Handle_t *callback_handle);

extern Processing_Status Processing_TxCpltCallback(Processing_Handle_t *handle, Processing_Callback_Handle_t *callback_handle);

extern Processing_Status Processing_FeedBack_Callback(Processing_Handle_t *handle, Processing_Callback_Handle_t *callback_handle);

#ifdef __cplusplus
}
#endif

#endif /* __PROCESSING_H */
