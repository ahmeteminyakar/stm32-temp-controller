#ifndef DHT22_H
#define DHT22_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct {
    float temperature;
    float humidity;
} DHT22_Data;

void    DHT22_Init(void);
uint8_t DHT22_Read(DHT22_Data *data);

#endif
