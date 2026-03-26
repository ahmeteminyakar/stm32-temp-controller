#ifndef OLED_H
#define OLED_H

#include "stm32f4xx_hal.h"

void OLED_Init(I2C_HandleTypeDef *hi2c);
void OLED_Clear(void);
void OLED_SetCursor(uint8_t col, uint8_t row);
void OLED_WriteString(const char *str);
void OLED_Update(void);

#endif
