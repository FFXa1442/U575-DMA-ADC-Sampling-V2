/*
 * serial.c
 *
 *  Created on: Apr 22, 2025
 *      Author:
 */


#include "serial.h"

#include <stdio.h>

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

UART_HandleTypeDef *ptr_huart = NULL;

PUTCHAR_PROTOTYPE
{
	if (ptr_huart)
	{
		HAL_UART_Transmit(ptr_huart, (uint8_t *)&ch, 1, 0xFFFF);
	}
    return ch;
}

/**
 * @brief  Initializes the UART peripheral for serial communication. 'printf' will use this UART.
 * @param  huart: Pointer to the UART_HandleTypeDef structure that contains the configuration information for the specified UART.
 * @retval None
 */
void Serial_Init(UART_HandleTypeDef * huart)
{
	ptr_huart = huart;
}