#include "dht22.h"
#include "main.h"

static void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

static void set_output(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin   = DHT22_PIN;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT22_PORT, &g);
}

static void set_input(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin  = DHT22_PIN;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT22_PORT, &g);
}

static uint8_t wait_for(GPIO_PinState state, uint32_t timeout_us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t limit = timeout_us * (SystemCoreClock / 1000000);
    while (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) != state) {
        if ((DWT->CYCCNT - start) > limit) return 0;
    }
    return 1;
}

void DHT22_Init(void)
{
}

uint8_t DHT22_Read(DHT22_Data *data)
{
    uint8_t bytes[5] = {0};

    set_output();
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_RESET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);
    delay_us(30);
    set_input();

    if (!wait_for(GPIO_PIN_RESET, 100)) return 0;
    if (!wait_for(GPIO_PIN_SET,   100)) return 0;
    if (!wait_for(GPIO_PIN_RESET, 100)) return 0;

    for (int i = 0; i < 40; i++) {
        if (!wait_for(GPIO_PIN_SET, 100)) return 0;

        uint32_t start = DWT->CYCCNT;
        if (!wait_for(GPIO_PIN_RESET, 100)) return 0;
        uint32_t elapsed_us = (DWT->CYCCNT - start) / (SystemCoreClock / 1000000);

        bytes[i / 8] <<= 1;
        if (elapsed_us > 40)
            bytes[i / 8] |= 1;
    }

    uint8_t checksum = bytes[0] + bytes[1] + bytes[2] + bytes[3];
    if (checksum != bytes[4]) return 0;

    uint16_t raw_hum = ((uint16_t)bytes[0] << 8) | bytes[1];
    data->humidity = (float)raw_hum / 10.0f;

    uint16_t raw_temp = ((uint16_t)bytes[2] << 8) | bytes[3];
    if (raw_temp & 0x8000) {
        raw_temp &= 0x7FFF;
        data->temperature = -(float)raw_temp / 10.0f;
    } else {
        data->temperature = (float)raw_temp / 10.0f;
    }

    return 1;
}
