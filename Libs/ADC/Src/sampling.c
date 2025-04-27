/*
 * signal.c
 *
 *  Created on: Apr 22, 2025
 *      Author:
 */

#include "sampling.h"

#include <stdlib.h>
#include <string.h>

typedef struct {

    uint8_t* adc_buffer;
    uint16_t adc_size;
    uint32_t adc_buffer_size;
    uint8_t type_size;

    ADC_HandleTypeDef* handle;

} ADC_Data_Handle_t;

typedef struct __ADC_Node_t ADC_Node_t;

struct __ADC_Node_t {
    ADC_Data_Handle_t* data_handle;
    ADC_Node_t* next;
};

typedef struct {
    ADC_Node_t* head;
    ADC_Node_t* tail;
} ADC_List_t;

ADC_List_t __ADC_List = {0};
uint8_t __ADC_List_Inited = 0;

/**
 * @brief  Initialize the ADC data handle list.
 *         This function initializes the linked list for ADC data handles.
 * @retval 1 if initialized, 0 if already initialized.
 */
uint8_t __ADC_List_Init(void)
{
    if (__ADC_List_Inited == 1) {
        return 0;
    }

    __ADC_List.head = NULL;
    __ADC_List.tail = NULL;
    __ADC_List_Inited = 1;

    return 1;
}

/**
 * @brief  Deinitialize the ADC data handle list and free all nodes.
 *         Frees all memory used by the list and resets its state.
 * @retval None
 */
void __ADC_List_DeInit(void)
{
    if (__ADC_List_Inited == 0) {
        return;
    }

    ADC_Node_t* current = __ADC_List.head;
    ADC_Node_t* next_node = NULL;

    while (current != NULL) {
        next_node = current->next;
        free(current);
        current = next_node;
    }

    __ADC_List.head = NULL;
    __ADC_List.tail = NULL;
    __ADC_List_Inited = 0;
}

/**
 * @brief  Add a data handle to the ADC list.
 *         Allocates a new node and adds it to the end of the list.
 * @param  data_handle: Pointer to the data handle to add.
 * @retval 1 if successful, 0 otherwise.
 */
uint8_t __ADC_List_Add(ADC_Data_Handle_t* data_handle)
{
    if (__ADC_List_Inited == 0) {
        return 0;
    }

    ADC_Node_t* new_node = (ADC_Node_t*)malloc(sizeof(ADC_Node_t));
    if (new_node == NULL) {
        return 0;
    }

    new_node->data_handle = data_handle;
    new_node->next = NULL;

    if (__ADC_List.head == NULL) {
        __ADC_List.head = new_node;
        __ADC_List.tail = new_node;
    } else {
        __ADC_List.tail->next = new_node;
        __ADC_List.tail = new_node;
    }

    return 1;
}

/**
 * @brief  Remove a data handle from the ADC list.
 *         Removes the node containing the given data handle and frees its memory.
 * @param  data_handle: Pointer to the data handle to remove.
 * @retval 1 if successful, 0 otherwise.
 */
uint8_t __ADC_List_Remove(ADC_Data_Handle_t* data_handle)
{
    if (__ADC_List_Inited == 0) {
        return 0;
    }

    ADC_Node_t* current = __ADC_List.head;
    ADC_Node_t* previous = NULL;

    while (current != NULL) {
        if (current->data_handle == data_handle) {
            if (previous == NULL) {
                __ADC_List.head = current->next;
            } else {
                previous->next = current->next;
            }

            if (current == __ADC_List.tail) {
                __ADC_List.tail = previous;
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
 * @brief  Find a data handle in the ADC list by ADC handle.
 *         Searches the list for a data handle matching the given ADC handle.
 * @param  hadc: ADC handle to search for.
 * @param  return_handle: Pointer to store the found data handle.
 * @retval 1 if found, 0 otherwise.
 */
uint8_t __ADC_List_Find(ADC_HandleTypeDef *hadc, ADC_Data_Handle_t** return_handle)
{
    if (__ADC_List_Inited == 0)
    {
        return 0;
    }

    ADC_Node_t* current = __ADC_List.head;

    while (current != NULL)
    {
        if (current->data_handle->handle == hadc && current->data_handle->handle->Instance == hadc->Instance)
        {
            *return_handle = current->data_handle;
            return 1;
        }
        current = current->next;
    }

    return 0;
}

/**
 * @brief  Initialize an ADC data handle and allocate buffer.
 *         Allocates and initializes a data handle for the given ADC handle, DMA size, and data type.
 * @param  hadc: ADC handle.
 * @param  dma_size: Size of the DMA buffer.
 * @param  data_type: Data type (byte, half-word, word).
 * @retval ADC_Result status.
 */
ADC_Result ADC_Init(ADC_HandleTypeDef *hadc, const uint16_t dma_size, const ADC_DataType data_type)
{

    if (hadc == NULL)
    {
        return ADC_ARGUMENT_OUT_OF_RANGE;
    }

    if (dma_size == 0)
    {
        return ADC_ERROR;
    }

    if (__ADC_List_Init() == 0)
    {
        return ADC_OUT_OF_MEMORY;
    }

    ADC_Data_Handle_t* data_handle = (ADC_Data_Handle_t*)malloc(sizeof(ADC_Data_Handle_t));
    if (data_handle == NULL)
    {
        return ADC_OUT_OF_MEMORY;
    }

    uint16_t type_size = 0;
    switch (data_type)
    {
        case ADC_BYTE:
            type_size = sizeof(uint8_t);
            break;
        case ADC_HALF_WORD:
            type_size = sizeof(uint16_t);
            break;
        case ADC_WORD:
            type_size = sizeof(uint32_t);
            break;
        default:
            return ADC_ARGUMENT_OUT_OF_RANGE;
    }

    data_handle->adc_size = dma_size;
    data_handle->adc_buffer_size = dma_size * type_size;
    data_handle->type_size = type_size;
    data_handle->adc_buffer = (uint8_t*)malloc(data_handle->adc_buffer_size);

    if (data_handle->adc_buffer == NULL)
    {
        free(data_handle);
        return ADC_OUT_OF_MEMORY;
    }

    data_handle->handle = hadc;

    if (__ADC_List_Add(data_handle) == 0)
    {
        free(data_handle->adc_buffer);
        free(data_handle);
        return ADC_OUT_OF_MEMORY;
    }

    return ADC_OK;
}

/**
 * @brief  Deinitialize an ADC data handle and free resources.
 *         Frees all memory associated with the ADC handle.
 * @param  hadc: ADC handle.
 * @retval ADC_Result status.
 */
ADC_Result ADC_DeInit(ADC_HandleTypeDef *hadc)
{
    if (hadc == NULL)
    {
        return ADC_ARGUMENT_OUT_OF_RANGE;
    }

    ADC_Data_Handle_t* data_handle = NULL;
    if (__ADC_List_Find(hadc, &data_handle) == 0)
    {
        return ADC_ERROR;
    }

    if (data_handle->adc_buffer != NULL)
    {
        free(data_handle->adc_buffer);
        data_handle->adc_buffer = NULL;
    }

    if (__ADC_List_Remove(data_handle) == 0)
    {
        return ADC_ERROR;
    }

    free(data_handle);

    return ADC_OK;
}

/**
 * @brief  Get ADC data buffer and related information.
 *         Copies ADC data to the provided buffer and/or returns buffer size, data size, and type size.
 * @param  hadc: ADC handle.
 * @param  buffer: Destination buffer to copy data (can be NULL).
 * @param  size: Pointer to store buffer size (can be NULL).
 * @param  data_size: Pointer to store number of data elements (can be NULL).
 * @param  type_size: Pointer to store data type size (can be NULL).
 * @retval ADC_Result status.
 */
ADC_Result ADC_Get(ADC_HandleTypeDef *hadc, uint8_t *buffer, uint16_t *size, uint32_t *data_size, uint8_t *type_size)
{
    if (hadc == NULL)
    {
        return ADC_ARGUMENT_OUT_OF_RANGE;
    }

    ADC_Data_Handle_t* data_handle = NULL;
    if (__ADC_List_Find(hadc, &data_handle) == 0)
    {
        return ADC_ERROR;
    }

    if (buffer != NULL)
    {
        memcpy(buffer, data_handle->adc_buffer, data_handle->adc_buffer_size);
    }

    if (size != NULL)
    {
        *size = data_handle->adc_size;
    }

    if (data_size != NULL)
    {
        *data_size = data_handle->adc_buffer_size;
    }

    if (type_size != NULL)
    {
        *type_size = data_handle->type_size;
    }

    return ADC_OK;
}

/**
 * @brief  Start ADC conversion using DMA.
 *         Stops any ongoing DMA, then starts a new DMA transfer for the ADC.
 * @param  hadc: ADC handle.
 * @retval ADC_Result status.
 */
ADC_Result ADC_Start_DMA(ADC_HandleTypeDef *hadc)
{
    if (hadc == NULL)
    {
        return ADC_ARGUMENT_OUT_OF_RANGE;
    }

    ADC_Data_Handle_t* data_handle = NULL;
    if (__ADC_List_Find(hadc, &data_handle) == 0)
    {
        return ADC_ERROR;
    }

    HAL_ADC_Stop_DMA(hadc);

    if (HAL_ADC_Start_DMA(hadc, (uint32_t*)data_handle->adc_buffer, data_handle->adc_size) != HAL_OK)
    {
        return ADC_ERROR;
    }

    return ADC_OK;
}