#include "main.h"
#include "pid.h"
#include "ds18b20.h"
#include "dht22.h"
#include "oled.h"
#include "control.h"
#include "uart_log.h"
#include <stdio.h>
#include <string.h>

TIM_HandleTypeDef  htim3;
I2C_HandleTypeDef  hi2c1;
UART_HandleTypeDef huart2;
ADC_HandleTypeDef  hadc1;

PID_Controller   pid;
ControlState     ctrl;
volatile uint8_t btn_pressed = 0;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);

static float ReadSetpointFromADC(void);
static void  UpdateDisplay(ControlState *state);

void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    if (pin == BTN_MODE_PIN)
        btn_pressed = 1;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_TIM3_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();
    MX_ADC1_Init();

    DS18B20_Init();
    DHT22_Init();
    OLED_Init(&hi2c1);
    UARTLog_Init(&huart2);

    PID_Init(&pid, DEFAULT_KP, DEFAULT_KI, DEFAULT_KD,
             CONTROL_DT, -100.0f, 100.0f);

    Control_Init(&ctrl);

    HAL_TIM_PWM_Start(&htim3, HEATER_TIM_CHANNEL);
    HAL_TIM_PWM_Start(&htim3, FAN_TIM_CHANNEL);

    UARTLog_SendHeader();

    OLED_Clear();
    OLED_SetCursor(10, 2);
    OLED_WriteString("TEMP CONTROLLER");
    OLED_SetCursor(30, 4);
    OLED_WriteString("Starting...");
    HAL_Delay(1500);

    uint32_t last_tick = HAL_GetTick();
    uint32_t last_dht_tick = 0;
    DHT22_Data dht_data = {0};

    while (1) {
        uint32_t now = HAL_GetTick();

        if ((now - last_tick) >= (1000 / CONTROL_LOOP_HZ)) {
            last_tick = now;

            ctrl.setpoint = ReadSetpointFromADC();

            float ds_temp = DS18B20_ReadTemp();

            /* DHT22 needs >= 2s between reads */
            if ((now - last_dht_tick) >= 2000) {
                last_dht_tick = now;
                if (DHT22_Read(&dht_data))
                    ctrl.humidity = dht_data.humidity;
            }

            /* Average both sensors if both are valid */
            float temp;
            if (ds_temp > -100.0f && dht_data.temperature > 0.1f)
                temp = (ds_temp + dht_data.temperature) / 2.0f;
            else if (ds_temp > -100.0f)
                temp = ds_temp;
            else
                temp = dht_data.temperature;

            if (btn_pressed) {
                btn_pressed = 0;
                Control_CycleMode(&ctrl);
                PID_Reset(&pid);
            }

            Control_Update(&ctrl, &pid, temp);
            Control_ApplyOutputs(&ctrl, &htim3);
            UpdateDisplay(&ctrl);
            UARTLog_SendData(now, &ctrl);
        }
    }
}

static float ReadSetpointFromADC(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint32_t raw = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    return SETPOINT_MIN + ((float)raw / 4095.0f) * (SETPOINT_MAX - SETPOINT_MIN);
}

static void UpdateDisplay(ControlState *state)
{
    char line[22];

    OLED_SetCursor(0, 0);
    snprintf(line, sizeof(line), "Mode: %s", Control_ModeString(state->mode));
    OLED_WriteString(line);

    OLED_SetCursor(0, 2);
    snprintf(line, sizeof(line), "Temp: %.1f C", state->temperature);
    OLED_WriteString(line);

    OLED_SetCursor(0, 3);
    snprintf(line, sizeof(line), "Set:  %.1f C", state->setpoint);
    OLED_WriteString(line);

    OLED_SetCursor(0, 4);
    snprintf(line, sizeof(line), "Hum:  %.0f %%", state->humidity);
    OLED_WriteString(line);

    OLED_SetCursor(0, 6);
    snprintf(line, sizeof(line), "H:%3.0f%%  F:%3.0f%%",
             state->heater_duty, state->fan_duty);
    OLED_WriteString(line);
}

/* Peripheral init — CubeMX generates these, shown here as reference */

void SystemClock_Config(void)
{
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef g = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    g.Pin  = BTN_MODE_PIN;
    g.Mode = GPIO_MODE_IT_FALLING;
    g.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BTN_MODE_PORT, &g);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void MX_TIM3_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};

    __HAL_RCC_TIM3_CLK_ENABLE();

    htim3.Instance               = TIM3;
    htim3.Init.Prescaler         = 89;
    htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim3.Init.Period            = 999;     /* 1 kHz PWM */
    htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    HAL_TIM_PWM_Init(&htim3);

    sConfigOC.OCMode     = TIM_OCMODE_PWM1;
    sConfigOC.Pulse      = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, HEATER_TIM_CHANNEL);
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, FAN_TIM_CHANNEL);
}

static void MX_I2C1_Init(void)
{
    __HAL_RCC_I2C1_CLK_ENABLE();

    hi2c1.Instance             = I2C1;
    hi2c1.Init.ClockSpeed      = 400000;
    hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1     = 0;
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&hi2c1);
}

static void MX_USART2_UART_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();

    huart2.Instance          = USART2;
    huart2.Init.BaudRate     = 115200;
    huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    huart2.Init.StopBits     = UART_STOPBITS_1;
    huart2.Init.Parity       = UART_PARITY_NONE;
    huart2.Init.Mode         = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&huart2);
}

static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance                   = ADC1;
    hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode          = DISABLE;
    hadc1.Init.ContinuousConvMode    = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion       = 1;

    HAL_ADC_Init(&hadc1);

    sConfig.Channel      = SETPOINT_ADC_CHANNEL;
    sConfig.Rank         = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;

    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(BTN_MODE_PIN);
}
