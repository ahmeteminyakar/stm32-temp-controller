#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx_hal.h"

#define DS18B20_PORT        GPIOA
#define DS18B20_PIN         GPIO_PIN_8

#define DHT22_PORT          GPIOA
#define DHT22_PIN           GPIO_PIN_9

#define OLED_I2C_ADDR       0x78

#define HEATER_TIM_CHANNEL  TIM_CHANNEL_1
#define FAN_TIM_CHANNEL     TIM_CHANNEL_2

#define BTN_MODE_PORT       GPIOC
#define BTN_MODE_PIN        GPIO_PIN_13

#define SETPOINT_ADC_CHANNEL ADC_CHANNEL_0

#define LOG_UART            USART2

typedef enum {
    MODE_HEAT = 0,
    MODE_COOL,
    MODE_AUTO
} ControlMode;

#define CONTROL_LOOP_HZ     10
#define CONTROL_DT          (1.0f / CONTROL_LOOP_HZ)

#define SETPOINT_MIN        15.0f
#define SETPOINT_MAX        50.0f

#define DEFAULT_KP          5.0f
#define DEFAULT_KI          0.3f
#define DEFAULT_KD          1.0f

#endif
