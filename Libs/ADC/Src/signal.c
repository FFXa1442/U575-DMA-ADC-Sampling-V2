/*
 * signal.c
 *
 *  Created on: Apr 22, 2025
 *      Author:
 */

#include "signal.h"

#include <stdlib.h>
#include <string.h>

typedef struct {

    uint8_t* adc_buffer;
    uint16_t adc_size;
    uint32_t adc_buffer_size;
    uint8_t type_size;

    ADC_HandleTypeDef* handle;

} AnaRP_Data_Handle_t;

typedef struct __AnaPR_Node_t AnaPR_Node_t;

struct __AnaPR_Node_t {
    AnaRP_Data_Handle_t* data_handle;
    AnaPR_Node_t* next;
};

typedef struct {
    AnaPR_Node_t* head;
    AnaPR_Node_t* tail;
} AnaRP_List_t;

AnaRP_List_t __ADC_List = {0};
uint8_t __ADC_List_Inited = 0;

uint8_t __AnaRP_List_Init(void)
{
    if (__ADC_List_Inited == 1) {
        return 0;
    }

    __ADC_List.head = NULL;
    __ADC_List.tail = NULL;
    __ADC_List_Inited = 1;

    return 1;
}

void __AnaRP_List_DeInit(void)
{
    if (__ADC_List_Inited == 0) {
        return;
    }

    AnaPR_Node_t* current = __ADC_List.head;
    AnaPR_Node_t* next_node = NULL;

    while (current != NULL) {
        next_node = current->next;
        free(current);
        current = next_node;
    }

    __ADC_List.head = NULL;
    __ADC_List.tail = NULL;
    __ADC_List_Inited = 0;
}

uint8_t __AnaRP_List_Add(AnaRP_Data_Handle_t* data_handle)
{
    if (__ADC_List_Inited == 0) {
        return 0;
    }

    AnaPR_Node_t* new_node = (AnaPR_Node_t*)malloc(sizeof(AnaPR_Node_t));
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

uint8_t __AnaRP_List_Remove(AnaRP_Data_Handle_t* data_handle)
{
    if (__ADC_List_Inited == 0) {
        return 0;
    }

    AnaPR_Node_t* current = __ADC_List.head;
    AnaPR_Node_t* previous = NULL;

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

uint8_t __AnaRP_List_Find(ADC_HandleTypeDef *hadc, AnaRP_Data_Handle_t** return_handle)
{
    if (__ADC_List_Inited == 0)
    {
        return 0;
    }

    AnaPR_Node_t* current = __ADC_List.head;

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

AnaRP_Result AnaRP_Init(ADC_HandleTypeDef *hadc, const uint16_t dma_size, const AnaRP_DataType data_type)
{

    if (hadc == NULL)
    {
        return ANA_RP_ARGUMENT_OUT_OF_RANGE;
    }

    if (dma_size == 0)
    {
        return ANA_RP_ERROR;
    }

    if (__AnaRP_List_Init() == 0)
    {
        return ANA_RP_OUT_OF_MEMORY;
    }

    AnaRP_Data_Handle_t* data_handle = (AnaRP_Data_Handle_t*)malloc(sizeof(AnaRP_Data_Handle_t));
    if (data_handle == NULL)
    {
        return ANA_RP_OUT_OF_MEMORY;
    }

    uint16_t type_size = 0;
    switch (data_type)
    {
        case ANA_RP_BYTE:
            type_size = sizeof(uint8_t);
            break;
        case ANA_RP_HALF_WORD:
            type_size = sizeof(uint16_t);
            break;
        case ANA_RP_WORD:
            type_size = sizeof(uint32_t);
            break;
        default:
            return ANA_RP_ARGUMENT_OUT_OF_RANGE;
    }

    data_handle->adc_size = dma_size;
    data_handle->adc_buffer_size = dma_size * type_size;
    data_handle->type_size = type_size;
    data_handle->adc_buffer = (uint8_t*)malloc(data_handle->adc_buffer_size);

    if (data_handle->adc_buffer == NULL)
    {
        free(data_handle);
        return ANA_RP_OUT_OF_MEMORY;
    }

    data_handle->handle = hadc;

    if (__AnaRP_List_Add(data_handle) == 0)
    {
        free(data_handle->adc_buffer);
        free(data_handle);
        return ANA_RP_OUT_OF_MEMORY;
    }

    return ANA_RP_OK;
}

AnaRP_Result AnaRP_DeInit(ADC_HandleTypeDef *hadc)
{
    if (hadc == NULL)
    {
        return ANA_RP_ARGUMENT_OUT_OF_RANGE;
    }

    AnaRP_Data_Handle_t* data_handle = NULL;
    if (__AnaRP_List_Find(hadc, &data_handle) == 0)
    {
        return ANA_RP_ERROR;
    }

    if (data_handle->adc_buffer != NULL)
    {
        free(data_handle->adc_buffer);
        data_handle->adc_buffer = NULL;
    }

    if (__AnaRP_List_Remove(data_handle) == 0)
    {
        return ANA_RP_ERROR;
    }

    free(data_handle);

    return ANA_RP_OK;
}

AnaRP_Result AnaRP_GetData(ADC_HandleTypeDef *hadc, uint8_t *buffer, uint16_t *size, uint32_t *data_size, uint8_t *type_size)
{
    if (hadc == NULL)
    {
        return ANA_RP_ARGUMENT_OUT_OF_RANGE;
    }

    AnaRP_Data_Handle_t* data_handle = NULL;
    if (__AnaRP_List_Find(hadc, &data_handle) == 0)
    {
        return ANA_RP_ERROR;
    }

    if (buffer != NULL)
    {
        memcpy(buffer, data_handle->adc_buffer, data_handle->adc_buffer_size);
    }

    if (size != NULL)
    {
        *size = data_handle->adc_buffer_size;
    }

    if (data_size != NULL)
    {
        *data_size = data_handle->adc_size;
    }

    if (type_size != NULL)
    {
        *type_size = data_handle->type_size;
    }

    return ANA_RP_OK;
}

AnaRP_Result AnaRP_Start_DMA(ADC_HandleTypeDef *hadc)
{
    if (hadc == NULL)
    {
        return ANA_RP_ARGUMENT_OUT_OF_RANGE;
    }

    AnaRP_Data_Handle_t* data_handle = NULL;
    if (__AnaRP_List_Find(hadc, &data_handle) == 0)
    {
        return ANA_RP_ERROR;
    }

    if (HAL_ADC_Start_DMA(hadc, (uint32_t*)data_handle->adc_buffer, data_handle->adc_size) != HAL_OK)
    {
        return ANA_RP_ERROR;
    }

    return ANA_RP_OK;
}


AnaRP_Result AnaRP_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc, uint8_t *dst, uint16_t *size)
{
    if (hadc == NULL)
    {
        return ANA_RP_ARGUMENT_OUT_OF_RANGE;
    }

    AnaRP_Data_Handle_t* data_handle = NULL;
    if (__AnaRP_List_Find(hadc, &data_handle) == 0)
    {
        return ANA_RP_ERROR;
    }

    HAL_ADC_Stop_DMA(hadc);

    if (dst != NULL)
    {
        memcpy(dst, data_handle->adc_buffer, data_handle->adc_buffer_size);
    }

    if (size != NULL)
    {
        *size = data_handle->adc_size;
    }

    return ANA_RP_OK;
}