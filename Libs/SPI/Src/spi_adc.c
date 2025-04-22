/*
 * spi_adc.c
 *
 *  Created on: Apr 22, 2025
 *      Author:
 */

#include "spi_adc.h"

#include <stdlib.h>
#include <string.h>

#include "signal.h"

#define FRAME_HEADER_1   0xAA    // Frame header first byte
#define FRAME_HEADER_2   0x55    // Frame header second byte
#define FRAME_FOOTER_1   0x5A    // Frame footer first byte
#define FRAME_FOOTER_2   0xA5    // Frame footer second byte

typedef struct {
    uint8_t* spi_buffer; // Allocate After
    uint32_t spi_size; // Header + Size + ADC Data + Footer

    SPI_HandleTypeDef* handle;

} SPI_ADC_Data_Handle_t;

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
        if (current->data_handle->spi_buffer != NULL)
        {
            free(current->data_handle->spi_buffer);
            current->data_handle->spi_buffer = NULL;
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

            if (current->data_handle->spi_buffer != NULL)
            {
                free(current->data_handle->spi_buffer);
                current->data_handle->spi_buffer = NULL;
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

uint8_t __SPI_ADC_List_Find(SPI_HandleTypeDef *hspi, SPI_ADC_Data_Handle_t** return_handle)
{
    if (__SPI_ADC_List_Inited == 0)
    {
        return 0;
    }

    SPI_ADC_Node_t* current = __SPI_ADC_List.head;

    while (current != NULL)
    {
        if (current->data_handle->handle == hspi && current->data_handle->handle->Instance == hspi->Instance)
        {
            *return_handle = current->data_handle;
            return 1;
        }
        current = current->next;
    }

    return 0;
}

/**
 * @brief SPI_ADC_Init
 * @param hspi: SPI handle
 * @param hadc: ADC handle
 * @return SPI_ADC_Result: Result of the operation
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
    if (AnaRP_GetData(hadc, NULL, NULL, &adc_buffer_size, &type_size) != ANA_RP_OK)
    {
        return SPI_ADC_ERROR;
    }

    SPI_ADC_Data_Handle_t* data_handle = (SPI_ADC_Data_Handle_t*)malloc(sizeof(SPI_ADC_Data_Handle_t));
    if (data_handle == NULL)
    {
        return SPI_ADC_OUT_OF_MEMORY;
    }

    data_handle->spi_buffer = NULL;
    data_handle->spi_size = type_size + type_size + adc_buffer_size + type_size; // Header + Size + ADC Data + Footer
    data_handle->handle = hspi;

    if (__SPI_ADC_List_Add(data_handle) == 0)
    {
        free(data_handle);
        return SPI_ADC_OUT_OF_MEMORY;
    }

    return SPI_ADC_OK;
}

/**
 * @brief SPI_ADC_DeInit
 * @param hspi: SPI handle
 * @return SPI_ADC_Result: Result of the operation
 */
SPI_ADC_Result SPI_ADC_DeInit(SPI_HandleTypeDef *hspi)
{
    if (hspi == NULL)
    {
        return SPI_ADC_ARGUMENT_OUT_OF_RANGE;
    }

    SPI_ADC_Data_Handle_t* data_handle = NULL;
    if (__SPI_ADC_List_Find(hspi, &data_handle) == 0)
    {
        return SPI_ADC_ERROR;
    }

    if (__SPI_ADC_List_Remove(data_handle) == 0)
    {
        return SPI_ADC_FAILED;
    }

    free(data_handle);

    return SPI_ADC_OK;
}

/**
 * @brief SPI_ADC_Memcpy
 * @param hspi: SPI handle, Required
 * @param hadc: ADC handle, Required
 * @param pbuffer: Pointer to the buffer to store the data and the buffer is auto release, NULL to skip
 * @param size: Pointer to the size of the data, NULL to skip
 * @return SPI_ADC_Result: Result of the operation
 */
SPI_ADC_Result SPI_ADC_Memcpy(SPI_HandleTypeDef *hspi, ADC_HandleTypeDef *hadc, uint8_t **pbuffer, uint32_t *size)
{
    if (hspi == NULL || hadc == NULL)
    {
        return SPI_ADC_ARGUMENT_OUT_OF_RANGE;
    }

    SPI_ADC_Data_Handle_t* data_handle = NULL;
    if (__SPI_ADC_List_Find(hspi, &data_handle) == 0)
    {
        return SPI_ADC_ERROR;
    }

    if (data_handle->spi_buffer != NULL)
    {
        free(data_handle->spi_buffer);
        data_handle->spi_buffer = NULL;
    }

    uint16_t adc_size = 0;
    uint32_t adc_buffer_size = 0;
    uint8_t type_size = 0;
    if (AnaRP_GetData(hadc, NULL, &adc_size, &adc_buffer_size, &type_size) != ANA_RP_OK)
    {
        return SPI_ADC_ERROR;
    }

    if (size != NULL)
    {
        *size = data_handle->spi_size;
    }

    if (pbuffer != NULL)
    {
        data_handle->spi_buffer = (uint8_t*)malloc(data_handle->spi_size);
        if (data_handle->spi_buffer == NULL)
        {
            return SPI_ADC_OUT_OF_MEMORY;
        }

        uint8_t* tar_buf = data_handle->spi_buffer;

        // Set Header
        *tar_buf++ = FRAME_HEADER_1; // Header
        *tar_buf++ = FRAME_HEADER_2; // Header

        // Set Size
        memcpy(tar_buf, &adc_size, type_size);
        tar_buf += type_size;

        // Set ADC Data
        if (AnaRP_GetData(hadc, tar_buf, NULL, NULL, NULL) != ANA_RP_OK)
        {
            free(data_handle->spi_buffer);
            data_handle->spi_buffer = NULL;
            return SPI_ADC_ERROR;
        }
        tar_buf += adc_buffer_size;

        // Set Footer
        *tar_buf++ = FRAME_FOOTER_1; // Footer
        *tar_buf++ = FRAME_FOOTER_2; // Footer

        *pbuffer = data_handle->spi_buffer;
    }

    return SPI_ADC_OK;
}