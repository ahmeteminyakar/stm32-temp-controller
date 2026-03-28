#include "ds18b20.h"
#include "main.h"

static void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

static void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void set_pin_output(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin   = DS18B20_PIN;
    g.Mode  = GPIO_MODE_OUTPUT_OD;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DS18B20_PORT, &g);
}

static void set_pin_input(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin  = DS18B20_PIN;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DS18B20_PORT, &g);
}

static uint8_t ow_reset(void)
{
    uint8_t presence;
    set_pin_output();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    delay_us(480);
    set_pin_input();
    delay_us(70);
    presence = (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN) == GPIO_PIN_RESET) ? 1 : 0;
    delay_us(410);
    return presence;
}

static void ow_write_bit(uint8_t bit)
{
    set_pin_output();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    if (bit) {
        delay_us(6);
        set_pin_input();
        delay_us(64);
    } else {
        delay_us(60);
        set_pin_input();
        delay_us(10);
    }
}

static uint8_t ow_read_bit(void)
{
    uint8_t val;
    set_pin_output();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    delay_us(6);
    set_pin_input();
    delay_us(9);
    val = HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN);
    delay_us(55);
    return val;
}

static void ow_write_byte(uint8_t byte)
{
    for (int i = 0; i < 8; i++) {
        ow_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

static uint8_t ow_read_byte(void)
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte >>= 1;
        if (ow_read_bit()) byte |= 0x80;
    }
    return byte;
}

void DS18B20_Init(void)
{
    DWT_Init();
}

uint8_t DS18B20_Start(void)
{
    return ow_reset();
}

float DS18B20_ReadTemp(void)
{
    if (!ow_reset()) return -999.0f;
    ow_write_byte(0xCC);
    ow_write_byte(0x44);

    HAL_Delay(750);

    if (!ow_reset()) return -999.0f;
    ow_write_byte(0xCC);
    ow_write_byte(0xBE);

    uint8_t lsb = ow_read_byte();
    uint8_t msb = ow_read_byte();

    int16_t raw = ((int16_t)msb << 8) | lsb;
    return (float)raw / 16.0f;
}
