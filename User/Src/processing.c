#include "processing.h"

#if defined(USER_UART_MODE) && defined(USER_SPI_MODE)
#error "Please define only one mode: USER_UART_MODE or USER_SPI_MODE"
#endif

#include "sampling.h"

#if defined(USER_UART_MODE)
#include "uart_adc.h"
#elif defined(USER_SPI_MODE)
#include "spi_adc.h"
#endif

#include <string.h>
#include <stdlib.h>

/**
 * @brief Initialize Processing_Handle_t structure and related peripherals
 * @param handle Pointer to Processing_Handle_t structure
 * @retval PROCESSING_OK Initialization successful
 * @retval PROCESSING_ERROR Initialization failed
 */
Processing_Status Processing_Init(Processing_Handle_t *handle)
{
    if (ADC_Init(handle->hadc, ADC_SAMPLING_SIZE, ADC_HALF_WORD) != ADC_OK)
    {
        return PROCESSING_ERROR;
    }

    handle->tx_buffer = NULL;
    handle->tx_size = 0;

    handle->rx_size = UART_RX_SIZE;
    memset(handle->rx_buffer, 0, sizeof(handle->rx_buffer));

#if defined(USER_UART_MODE)
    if (UART_ADC_Init(handle->huart, handle->hadc) == UART_ADC_OK)
    {
        HAL_UART_Receive_DMA(handle->huart, &handle->rx_buffer[0], handle->rx_size);
    }
    else
    {
        return PROCESSING_ERROR;
    }

#elif defined(USER_SPI_MODE)
    if (SPI_ADC_Init(handle->hspi, handle->hadc) != SPI_ADC_OK)
    {
        return PROCESSING_ERROR;
    }
#endif

    return PROCESSING_OK;
}

/**
 * @brief Start DMA transfer for ADC
 * @param handle Pointer to Processing_Handle_t structure
 * @retval PROCESSING_OK Start successful
 * @retval PROCESSING_ERROR Start failed
 */
Processing_Status Processing_Start_DMA(Processing_Handle_t *handle)
{
    if (ADC_Start_DMA(handle->hadc) != ADC_OK)
    {
        return PROCESSING_ERROR;
    }
    return PROCESSING_OK;
}

/**
 * @brief ADC conversion complete callback, process data and start transmission
 * @param handle Pointer to Processing_Handle_t structure
 * @param callback_handle Pointer to Processing_Callback_Handle_t structure
 * @retval PROCESSING_OK Data processing and transmission started successfully
 * @retval PROCESSING_FAILED Processing failed
 */
Processing_Status Processing_ADC_ConvCpltCallback(Processing_Handle_t *handle, Processing_Callback_Handle_t *callback_handle)
{
    if (handle->hadc->Instance != callback_handle->hadc->Instance)
    {
        return PROCESSING_FAILED;
    }

#if defined(USER_UART_MODE)
    if (UART_ADC_Get(handle->huart, handle->hadc, &handle->tx_buffer, &handle->tx_size) == UART_ADC_OK)
    {
        HAL_UART_Transmit_DMA(handle->huart, handle->tx_buffer, handle->tx_size);
        return PROCESSING_OK;
    }
#elif defined(USER_SPI_MODE)
    if (SPI_ADC_Get(handle->hspi, handle->hadc, &handle->tx_buffer, NULL, &handle->tx_size) == SPI_ADC_OK)
    {
        HAL_GPIO_WritePin(handle->SPI_CS_GPIOx, handle->SPI_CS_Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit_DMA(handle->hspi, handle->tx_buffer, handle->tx_size);
        return PROCESSING_OK;
    }
#else
    UNUSED(handle);
    UNUSED(callback_handle);
#endif

    return PROCESSING_FAILED;
}

/**
 * @brief Transmission complete callback, release CS pin in SPI mode
 * @param handle Pointer to Processing_Handle_t structure
 * @param callback_handle Pointer to Processing_Callback_Handle_t structure
 * @retval PROCESSING_OK Processing successful
 * @retval PROCESSING_FAILED Processing failed
 */
Processing_Status Processing_TxCpltCallback(Processing_Handle_t *handle, Processing_Callback_Handle_t *callback_handle)
{
#if defined(USER_UART_MODE)
    UNUSED(handle);
    UNUSED(callback_handle);
    // if (handle->huart->Instance == callback_handle->huart->Instance)
    // {

    //     return PROCESSING_OK;
    // }
#elif defined(USER_SPI_MODE)
    if (handle->hspi->Instance == callback_handle->hspi->Instance)
    {
        HAL_GPIO_WritePin(handle->SPI_CS_GPIOx, handle->SPI_CS_Pin, GPIO_PIN_SET);
        return PROCESSING_OK;
    }
#else

    UNUSED(handle);
    UNUSED(callback_handle);
#endif
    return PROCESSING_FAILED;
}

/**
 * @brief Handle UART/SPI feedback command, restart ADC DMA after receiving feedback
 * @param handle Pointer to Processing_Handle_t structure
 * @param callback_handle Pointer to Processing_Callback_Handle_t structure
 * @retval PROCESSING_OK Feedback received and processed successfully
 * @retval PROCESSING_FAILED Feedback not received correctly or processing failed
 */
Processing_Status Processing_FeedBack_Callback(Processing_Handle_t *handle, Processing_Callback_Handle_t *callback_handle)
{
#if defined(USER_UART_MODE)
    if (handle->huart->Instance == callback_handle->huart->Instance)
    {
        uint32_t* rx_id = (uint32_t*)handle->rx_buffer;

        if (*rx_id == UART_RX_FB_ID)
        {
            ADC_Start_DMA(handle->hadc);
            return PROCESSING_OK;
        }

        memset(handle->rx_buffer, 0, sizeof(handle->rx_buffer));
    }
#elif defined(USER_SPI_MODE)
    if (handle->SPI_FB_Pin == callback_handle->SPI_FB_Pin)
    {
        ADC_Start_DMA(handle->hadc);
        return PROCESSING_OK;
    }
#else
    UNUSED(handle);
    UNUSED(callback_handle);
#endif
    return PROCESSING_FAILED;
}