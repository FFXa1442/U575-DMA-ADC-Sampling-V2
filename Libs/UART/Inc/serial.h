/*
 * serial.h
 *
 *  Created on: Apr 22, 2025
 *      Author: 
 */

#ifndef __UART_SERIAL_H
#define __UART_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stm32u5xx_hal.h"

extern void Serial_Init(UART_HandleTypeDef * huart);

#ifdef __cplusplus
}
#endif

#endif /* __UART_SERIAL_H */
