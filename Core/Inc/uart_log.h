#ifndef UART_LOG_H
#define UART_LOG_H

#include "stm32f4xx_hal.h"
#include "control.h"

void UARTLog_Init(UART_HandleTypeDef *huart);
void UARTLog_SendHeader(void);
void UARTLog_SendData(uint32_t tick_ms, ControlState *state);

#endif
