/*
 * uart_adc.c
 *
 *  Created on: Apr 23, 2025
 *      Author:
 */

#include "uart_adc.h"

#include <stdlib.h>
#include <string.h>

#include "sampling.h"

#define FRAME_HEADER_1   0xAA    // Frame header first byte
#define FRAME_HEADER_2   0x55    // Frame header second byte
#define FRAME_FOOTER_1   0x5A    // Frame footer first byte
#define FRAME_FOOTER_2   0xA5    // Frame footer second byte

typedef struct {
    uint8_t* uart_tx_buffer; // Allocate After
    uint32_t uart_tx_size; // Header + Size + ADC Data + Footer

    uint8_t* uart_rx_buffer; // Allocate After
    uint32_t uart_rx_size;

    UART_HandleTypeDef* handle;
    size_t adc_ref;

} UART_ADC_Data_Handle_t;

#if 1 // List Implementation

typedef struct __UART_ADC_Node_t UART_ADC_Node_t;
typedef struct __UART_ADC_Node_t
{
    UART_ADC_Data_Handle_t* data_handle;
    UART_ADC_Node_t* next;
} UART_ADC_Node_t;

typedef struct
{
    UART_ADC_Node_t* head;
    UART_ADC_Node_t* tail;
} UART_ADC_List_t;

UART_ADC_List_t __UART_ADC_List = {0};
uint8_t __UART_ADC_List_Inited = 0;

/**
 * @brief Initialize the UART ADC list if not already initialized.
 * @retval 1 if initialized, 0 if already initialized.
 */
uint8_t __UART_ADC_List_Init(void)
{
    if (__UART_ADC_List_Inited == 1)
    {
        return 0;
    }

    __UART_ADC_List.head = NULL;
    __UART_ADC_List.tail = NULL;
    __UART_ADC_List_Inited = 1;

    return 1;
}

/**
 * @brief Deinitialize the UART ADC list and free all nodes.
 * @retval None
 */
void __UART_ADC_List_DeInit(void)
{
    if (__UART_ADC_List_Inited == 0) {
        return;
    }

    UART_ADC_Node_t* current = __UART_ADC_List.head;
    UART_ADC_Node_t* next_node = NULL;

    while (current != NULL)
    {
        next_node = current->next;
        if (current->data_handle->uart_tx_buffer != NULL)
        {
            free(current->data_handle->uart_tx_buffer);
            current->data_handle->uart_tx_buffer = NULL;
        }
        if (current->data_handle->uart_rx_buffer != NULL)
        {
            free(current->data_handle->uart_rx_buffer);
            current->data_handle->uart_rx_buffer = NULL;
        }
        if (current->data_handle != NULL)
        {
            free(current->data_handle);
            current->data_handle = NULL;
        }
        free(current);
        current = next_node;
    }

    __UART_ADC_List.head = NULL;
    __UART_ADC_List.tail = NULL;
    __UART_ADC_List_Inited = 0;
}

/**
 * @brief Add a UART_ADC_Data_Handle_t to the list.
 * @param data_handle: Pointer to the data handle to add.
 * @retval 1 if added successfully, 0 otherwise.
 */
uint8_t __UART_ADC_List_Add(UART_ADC_Data_Handle_t* data_handle)
{
    if (__UART_ADC_List_Inited == 0)
    {
        return 0;
    }

    UART_ADC_Node_t* new_node = (UART_ADC_Node_t*)malloc(sizeof(UART_ADC_Node_t));
    if (new_node == NULL)
    {
        return 0;
    }

    new_node->data_handle = data_handle;
    new_node->next = NULL;

    if (__UART_ADC_List.head == NULL)
    {
        __UART_ADC_List.head = new_node;
        __UART_ADC_List.tail = new_node;
    }
    else
    {
        __UART_ADC_List.tail->next = new_node;
        __UART_ADC_List.tail = new_node;
    }

    return 1;
}

/**
 * @brief Remove a UART_ADC_Data_Handle_t from the list and free its memory.
 * @param data_handle: Pointer to the data handle to remove.
 * @retval 1 if removed successfully, 0 otherwise.
 */
uint8_t __UART_ADC_List_Remove(UART_ADC_Data_Handle_t* data_handle)
{
    if (__UART_ADC_List_Inited == 0) {
        return 0;
    }

    UART_ADC_Node_t* current = __UART_ADC_List.head;
    UART_ADC_Node_t* previous = NULL;

    while (current != NULL) {
        if (current->data_handle == data_handle) {
            if (previous == NULL) {
                __UART_ADC_List.head = current->next;
            } else {
                previous->next = current->next;
            }

            if (current == __UART_ADC_List.tail) {
                __UART_ADC_List.tail = previous;
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
 * @brief Find a UART_ADC_Data_Handle_t in the list by UART handle and ADC reference.
 * @param huart: UART handle.
 * @param ref: ADC reference (usually the ADC handle cast to size_t).
 * @param return_handle: Pointer to store the found data handle.
 * @retval 1 if found, 0 otherwise.
 */
uint8_t __UART_ADC_List_Find(UART_HandleTypeDef *huart, size_t ref, UART_ADC_Data_Handle_t** return_handle)
{
    if (__UART_ADC_List_Inited == 0)
    {
        return 0;
    }

    UART_ADC_Node_t* current = __UART_ADC_List.head;

    while (current != NULL)
    {
        if (current->data_handle->handle == huart
            && current->data_handle->handle->Instance == huart->Instance
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
 * @brief Initialize UART ADC data handle and add it to the list.
 *        Allocates and initializes a data handle for the given UART and ADC handles.
 * @param huart: UART handle.
 * @param hadc: ADC handle.
 * @retval UART_ADC_Result: Result of the operation.
 */
UART_ADC_Result UART_ADC_Init(UART_HandleTypeDef *huart, ADC_HandleTypeDef *hadc, const uint32_t rx_buffer)
{
    if (huart == NULL || hadc == NULL)
    {
        return UART_ADC_ARGUMENT_OUT_OF_RANGE;
    }

    if (__UART_ADC_List_Init() == 0)
    {
        return UART_ADC_OUT_OF_MEMORY;
    }

    // uint8_t type_size = 0;
    uint32_t adc_buffer_size = 0;
    if (ADC_Get(hadc, NULL, NULL, &adc_buffer_size, NULL) != ADC_OK)
    {
        return UART_ADC_ERROR;
    }

    UART_ADC_Data_Handle_t* data_handle = (UART_ADC_Data_Handle_t*)malloc(sizeof(UART_ADC_Data_Handle_t));
    if (data_handle == NULL)
    {
        return UART_ADC_OUT_OF_MEMORY;
    }

    data_handle->uart_tx_buffer = NULL;
    data_handle->uart_rx_buffer = NULL;
    data_handle->uart_tx_size = 2 + 2 + adc_buffer_size + 2; // Header + Size + ADC Data + Footer
    data_handle->uart_rx_size = rx_buffer; // Set RX buffer size
    data_handle->handle = huart;
    data_handle->adc_ref = (size_t)hadc; // Use ADC handle as reference

    if (__UART_ADC_List_Add(data_handle) == 0)
    {
        free(data_handle);
        return UART_ADC_OUT_OF_MEMORY;
    }

    return UART_ADC_OK;
}

/**
 * @brief Deinitialize a UART ADC data handle and free resources.
 *        Frees all memory associated with the UART ADC handle.
 * @param huart: UART handle.
 * @param hadc: ADC handle.
 * @retval UART_ADC_Result: Result of the operation.
 */
UART_ADC_Result UART_ADC_DeInit(UART_HandleTypeDef *huart, ADC_HandleTypeDef *hadc)
{
    if (huart == NULL || hadc == NULL)
    {
        return UART_ADC_ARGUMENT_OUT_OF_RANGE;
    }

    UART_ADC_Data_Handle_t* data_handle = NULL;
    if (__UART_ADC_List_Find(huart, (size_t)hadc, &data_handle) == 0)
    {
        return UART_ADC_ERROR;
    }

    if (data_handle->uart_tx_buffer != NULL)
    {
        free(data_handle->uart_tx_buffer);
        data_handle->uart_tx_buffer = NULL;
    }

    if (data_handle->uart_rx_buffer != NULL)
    {
        free(data_handle->uart_rx_buffer);
        data_handle->uart_rx_buffer = NULL;
    }

    if (__UART_ADC_List_Remove(data_handle) == 0)
    {
        return UART_ADC_ERROR;
    }

    free(data_handle);

    return UART_ADC_OK;
}


UART_ADC_Result UART_ADC_Get(UART_HandleTypeDef *huart, ADC_HandleTypeDef *hadc, uint8_t **ptr_tx_buffer, uint32_t *ptr_tx_size, uint8_t **ptr_rx_buffer, uint32_t *ptr_rx_size)
{
    if (huart == NULL || hadc == NULL)
    {
        return UART_ADC_ARGUMENT_OUT_OF_RANGE;
    }

    UART_ADC_Data_Handle_t* data_handle = NULL;
    if (__UART_ADC_List_Find(huart, (size_t)hadc, &data_handle) == 0)
    {
        return UART_ADC_ERROR;
    }

    if (data_handle->uart_tx_buffer != NULL)
    {
        free(data_handle->uart_tx_buffer);
        data_handle->uart_tx_buffer = NULL;
    }
    
    if (data_handle->uart_rx_buffer != NULL)
    {
        free(data_handle->uart_rx_buffer);
        data_handle->uart_rx_buffer = NULL;
    }

    uint16_t adc_size = 0;
    uint32_t adc_buffer_size = 0;
    // uint8_t type_size = 0;
    if (ADC_Get(hadc, NULL, &adc_size, &adc_buffer_size, NULL) != ADC_OK)
    {
        return UART_ADC_ERROR;
    }

    if (ptr_tx_size != NULL)
    {
        *ptr_tx_size = data_handle->uart_tx_size;
    }

    if (ptr_rx_size != NULL)
    {
        *ptr_rx_size = data_handle->uart_rx_size;
    }

    if (ptr_tx_buffer != NULL)
    {
        data_handle->uart_tx_buffer = (uint8_t*)malloc(data_handle->uart_tx_size);
        if (data_handle->uart_tx_buffer == NULL)
        {
            return UART_ADC_OUT_OF_MEMORY;
        }

        uint8_t* tar_buf = data_handle->uart_tx_buffer;

        // Set Header
        *tar_buf++ = FRAME_HEADER_1; // Header
        *tar_buf++ = FRAME_HEADER_2; // Header

        // Set Size
        memcpy(tar_buf, &adc_size, 2);
        
        tar_buf += 2;

        // Set ADC Data
        if (ADC_Get(hadc, tar_buf, NULL, NULL, NULL) != ADC_OK)
        {
            return UART_ADC_ERROR;
        }

        tar_buf += adc_buffer_size;

        // Set Footer
        *tar_buf++ = FRAME_FOOTER_1; // Footer
        *tar_buf++ = FRAME_FOOTER_2; // Footer

        *ptr_tx_buffer = data_handle->uart_tx_buffer;
    }

    if (ptr_rx_buffer != NULL)
    {
        data_handle->uart_rx_buffer = (uint8_t*)malloc(data_handle->uart_rx_size);
        if (data_handle->uart_rx_buffer == NULL)
        {
            return UART_ADC_OUT_OF_MEMORY;
        }

        memset(data_handle->uart_rx_buffer, 0, data_handle->uart_rx_size); // Initialize RX buffer to zero
        *ptr_rx_buffer = data_handle->uart_rx_buffer;
    }

    return UART_ADC_OK;
}
