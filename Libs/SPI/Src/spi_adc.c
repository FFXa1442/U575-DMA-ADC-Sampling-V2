/*
 * spi_adc.c
 *
 *  Created on: Apr 22, 2025
 *      Author:
 */

#include "spi_adc.h"

#include <stdlib.h>
#include <string.h>

#include "sampling.h"


#define FRAME_HEADER_1   0xCA    // Frame header first byte
#define FRAME_HEADER_2   0x78    // Frame header second byte
// #define FRAME_FOOTER_1   0x5A    // Frame footer first byte
// #define FRAME_FOOTER_2   0xA5    // Frame footer second byte


typedef struct {
    uint8_t* spi_tx_buffer; // Allocate After
    uint8_t* spi_rx_buffer; // Allocate After
    uint32_t spi_size; // Header + Size + ADC Data


    SPI_HandleTypeDef* handle;
    size_t adc_ref;

} SPI_ADC_Data_Handle_t;

#if 1 // List Implementation

typedef struct __SPI_ADC_Node_t SPI_ADC_Node_t;
typedef struct __SPI_ADC_Node_t
{
    SPI_ADC_Data_Handle_t* data_handle;
    SPI_ADC_Node_t* next;
} SPI_ADC_Node_t;

typedef struct
{
    SPI_ADC_Node_t* head;
    SPI_ADC_Node_t* tail;
} SPI_ADC_List_t;

SPI_ADC_List_t __SPI_ADC_List = {0};
uint8_t __SPI_ADC_List_Inited = 0;

/**
 * @brief Initialize the SPI ADC list if not already initialized.
 * @retval 1 if initialized, 0 if already initialized.
 */
uint8_t __SPI_ADC_List_Init(void)
{
    if (__SPI_ADC_List_Inited == 1)
    {
        return 0;
    }

    __SPI_ADC_List.head = NULL;
    __SPI_ADC_List.tail = NULL;
    __SPI_ADC_List_Inited = 1;

    return 1;
}

/**
 * @brief Deinitialize the SPI ADC list and free all allocated memory.
 *        This function releases all memory used by the list and its data handles.
 */
void __SPI_ADC_List_DeInit(void)
{
    if (__SPI_ADC_List_Inited == 0)
    {
        return;
    }

    SPI_ADC_Node_t* current = __SPI_ADC_List.head;
    SPI_ADC_Node_t* next_node = NULL;

    while (current != NULL)
    {
        next_node = current->next;
        if (current->data_handle->spi_tx_buffer != NULL)
        {
            free(current->data_handle->spi_tx_buffer);
            current->data_handle->spi_tx_buffer = NULL;
        }
        if (current->data_handle->spi_rx_buffer != NULL)
        {
            free(current->data_handle->spi_rx_buffer);
            current->data_handle->spi_rx_buffer = NULL;
        }
        if (current->data_handle != NULL)
        {
            free(current->data_handle);
            current->data_handle = NULL;
        }
        free(current);
        current = next_node;
    }

    __SPI_ADC_List.head = NULL;
    __SPI_ADC_List.tail = NULL;
    __SPI_ADC_List_Inited = 0;
}

/**
 * @brief Add a new SPI_ADC_Data_Handle_t to the list.
 * @param data_handle: Pointer to the data handle to add.
 * @retval 1 if added successfully, 0 otherwise.
 */
uint8_t __SPI_ADC_List_Add(SPI_ADC_Data_Handle_t* data_handle)
{
    if (__SPI_ADC_List_Inited == 0)
    {
        return 0;
    }

    SPI_ADC_Node_t* new_node = (SPI_ADC_Node_t*)malloc(sizeof(SPI_ADC_Node_t));
    if (new_node == NULL)
    {
        return 0;
    }

    new_node->data_handle = data_handle;
    new_node->next = NULL;

    if (__SPI_ADC_List.head == NULL)
    {
        __SPI_ADC_List.head = new_node;
        __SPI_ADC_List.tail = new_node;
    }
    else
    {
        __SPI_ADC_List.tail->next = new_node;
        __SPI_ADC_List.tail = new_node;
    }

    return 1;
}

/**
 * @brief Remove a SPI_ADC_Data_Handle_t from the list and free its memory.
 * @param data_handle: Pointer to the data handle to remove.
 * @retval 1 if removed successfully, 0 otherwise.
 */
uint8_t __SPI_ADC_List_Remove(SPI_ADC_Data_Handle_t* data_handle)
{
    if (__SPI_ADC_List_Inited == 0) {
        return 0;
    }

    SPI_ADC_Node_t* current = __SPI_ADC_List.head;
    SPI_ADC_Node_t* previous = NULL;

    while (current != NULL)
    {
        if (current->data_handle == data_handle)
        {
            if (previous == NULL)
            {
                __SPI_ADC_List.head = current->next;
            } else {
                previous->next = current->next;
            }

            if (current == __SPI_ADC_List.tail)
            {
                __SPI_ADC_List.tail = previous;
            }

            if (current->data_handle->spi_tx_buffer != NULL)
            {
                free(current->data_handle->spi_tx_buffer);
                current->data_handle->spi_tx_buffer = NULL;
            }
            if (current->data_handle->spi_rx_buffer != NULL)
            {
                free(current->data_handle->spi_rx_buffer);
                current->data_handle->spi_rx_buffer = NULL;
            }
            if (current->data_handle != NULL)
            {
                free(current->data_handle);
                current->data_handle = NULL;
            }
            free(current);
            return 1;
        }

        previous = current;
        current = current->next;
    }

    return 0;
}

/**
 * @brief Find a SPI_ADC_Data_Handle_t in the list by SPI handle and ADC reference.
 * @param hspi: SPI handle.
 * @param ref: ADC reference (usually the ADC handle cast to size_t).
 * @param return_handle: Pointer to store the found data handle.
 * @retval 1 if found, 0 otherwise.
 */
uint8_t __SPI_ADC_List_Find(SPI_HandleTypeDef *hspi, size_t ref, SPI_ADC_Data_Handle_t** return_handle)
{
    if (__SPI_ADC_List_Inited == 0)
    {
        return 0;
    }

    SPI_ADC_Node_t* current = __SPI_ADC_List.head;

    while (current != NULL)
    {
        if (current->data_handle->handle == hspi
            && current->data_handle->handle->Instance == hspi->Instance
            && current->data_handle->adc_ref == ref)

        {
            *return_handle = current->data_handle;
            return 1;
        }

        current = current->next;
    }

    return 0;
}

#endif

/**
 * @brief Initialize SPI ADC data handle and add it to the list.
 *        Allocates and initializes a data handle for the given SPI and ADC handles.
 * @param hspi: SPI handle.
 * @param hadc: ADC handle.
 * @retval SPI_ADC_Result: Result of the operation.
 */
SPI_ADC_Result SPI_ADC_Init(SPI_HandleTypeDef *hspi, ADC_HandleTypeDef *hadc)
{
    if (hspi == NULL || hadc == NULL)
    {
        return SPI_ADC_ARGUMENT_OUT_OF_RANGE;
    }

    if (__SPI_ADC_List_Init() == 0)
    {
        return SPI_ADC_OUT_OF_MEMORY;
    }

    uint8_t type_size = 0;
    uint32_t adc_buffer_size = 0;
    if (ADC_Get(hadc, NULL, NULL, &adc_buffer_size, &type_size) != ADC_OK)
    {
        return SPI_ADC_ERROR;
    }

    SPI_ADC_Data_Handle_t* data_handle = (SPI_ADC_Data_Handle_t*)malloc(sizeof(SPI_ADC_Data_Handle_t));
    if (data_handle == NULL)
    {
        return SPI_ADC_OUT_OF_MEMORY;
    }

    data_handle->spi_tx_buffer = NULL;
    data_handle->spi_rx_buffer = NULL;
    data_handle->spi_size = type_size + type_size + adc_buffer_size; // Header + Size + ADC Data
    data_handle->handle = hspi;
    data_handle->adc_ref = (size_t)hadc; // Use ADC handle as reference

    if (__SPI_ADC_List_Add(data_handle) == 0)
    {
        free(data_handle);
        return SPI_ADC_OUT_OF_MEMORY;
    }

    return SPI_ADC_OK;
}

/**
 * @brief Deinitialize SPI ADC data handle and remove it from the list.
 *        Frees all memory associated with the SPI/ADC handle pair.
 * @param hspi: SPI handle.
 * @param hadc: ADC handle.
 * @retval SPI_ADC_Result: Result of the operation.
 */
SPI_ADC_Result SPI_ADC_DeInit(SPI_HandleTypeDef *hspi, ADC_HandleTypeDef *hadc)
{
    if (hspi == NULL || hadc == NULL)
    {
        return SPI_ADC_ARGUMENT_OUT_OF_RANGE;
    }

    SPI_ADC_Data_Handle_t* data_handle = NULL;
    if (__SPI_ADC_List_Find(hspi, (size_t)hadc, &data_handle) == 0)
    {
        return SPI_ADC_ERROR;
    }

    if (__SPI_ADC_List_Remove(data_handle) == 0)
    {
        return SPI_ADC_ERROR;
    }

    free(data_handle);

    return SPI_ADC_OK;
}

/**
 * @brief Get a buffer containing the SPI frame with ADC data.
 *        Allocates and fills a TX buffer with header, size, ADC data, and ACK.
 *        Also allocates an RX buffer if requested.
 * @param hspi: SPI handle.
 * @param hadc: ADC handle.
 * @param ptr_tx_buffer: Pointer to store allocated TX buffer.
 * @param ptr_rx_buffer: Pointer to store allocated RX buffer.
 * @param ptr_size: Pointer to store size of the buffer.
 * @retval SPI_ADC_Result: Result of the operation.
 */
SPI_ADC_Result SPI_ADC_Get(SPI_HandleTypeDef *hspi, ADC_HandleTypeDef *hadc, uint8_t **ptr_tx_buffer, uint8_t **ptr_rx_buffer, uint32_t *ptr_size)
{
    if (hspi == NULL || hadc == NULL)
    {
        return SPI_ADC_ARGUMENT_OUT_OF_RANGE;
    }

    SPI_ADC_Data_Handle_t* data_handle = NULL;
    if (__SPI_ADC_List_Find(hspi, (size_t)hadc, &data_handle) == 0)
    {
        return SPI_ADC_ERROR;
    }

    if (data_handle->spi_tx_buffer != NULL)
    {
        free(data_handle->spi_tx_buffer);
        data_handle->spi_tx_buffer = NULL;
    }

    if (data_handle->spi_rx_buffer != NULL)
    {
        free(data_handle->spi_rx_buffer);
        data_handle->spi_rx_buffer = NULL;
    }

    uint16_t adc_size = 0;
    uint32_t adc_buffer_size = 0;
    uint8_t type_size = 0;
    if (ADC_Get(hadc, NULL, &adc_size, &adc_buffer_size, &type_size) != ADC_OK)
    {
        return SPI_ADC_ERROR;
    }

    if (ptr_size != NULL)
    {
        *ptr_size = data_handle->spi_size;
    }

    if (ptr_tx_buffer != NULL)
    {
        data_handle->spi_tx_buffer = (uint8_t*)malloc(data_handle->spi_size);
        if (data_handle->spi_tx_buffer == NULL)
        {
            return SPI_ADC_OUT_OF_MEMORY;
        }

        uint8_t* tar_buf = data_handle->spi_tx_buffer;

        // Set Header
        *tar_buf++ = FRAME_HEADER_1; // Header
        *tar_buf++ = FRAME_HEADER_2; // Header

        // Set Size
        memcpy(tar_buf, &adc_size, type_size);
        
        tar_buf += type_size;

        // Set ADC Data
        if (ADC_Get(hadc, tar_buf, NULL, NULL, NULL) != ADC_OK)
        {
            free(data_handle->spi_tx_buffer);
            data_handle->spi_tx_buffer = NULL;
            return SPI_ADC_ERROR;
        }
        tar_buf += adc_buffer_size;

        // Set ACK
        *tar_buf++ = 0x00; // ACK

        *ptr_tx_buffer = data_handle->spi_tx_buffer;
    }

    if (ptr_rx_buffer != NULL)
    {
        data_handle->spi_rx_buffer = (uint8_t*)malloc(data_handle->spi_size);
        if (data_handle->spi_rx_buffer == NULL)
        {
            return SPI_ADC_OUT_OF_MEMORY;
        }
        memset(data_handle->spi_rx_buffer, 0, data_handle->spi_size);
        *ptr_rx_buffer = data_handle->spi_rx_buffer;
    }

    return SPI_ADC_OK;
}
