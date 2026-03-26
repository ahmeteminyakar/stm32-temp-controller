#ifndef DS18B20_H
#define DS18B20_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

void    DS18B20_Init(void);
uint8_t DS18B20_Start(void);
float   DS18B20_ReadTemp(void);

#endif
