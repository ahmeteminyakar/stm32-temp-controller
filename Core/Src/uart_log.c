#include "uart_log.h"
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef *log_huart;

void UARTLog_Init(UART_HandleTypeDef *huart)
{
    log_huart = huart;
}

void UARTLog_SendHeader(void)
{
    const char *hdr = "time_ms,mode,setpoint,temp,humidity,heater%,fan%\r\n";
    HAL_UART_Transmit(log_huart, (uint8_t *)hdr, strlen(hdr), 100);
}

void UARTLog_SendData(uint32_t tick_ms, ControlState *state)
{
    char buf[128];
    int len = snprintf(buf, sizeof(buf),
                       "%lu,%s,%.1f,%.1f,%.1f,%.0f,%.0f\r\n",
                       (unsigned long)tick_ms,
                       Control_ModeString(state->mode),
                       state->setpoint,
                       state->temperature,
                       state->humidity,
                       state->heater_duty,
                       state->fan_duty);
    HAL_UART_Transmit(log_huart, (uint8_t *)buf, len, 50);
}
