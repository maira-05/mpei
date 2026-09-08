#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void Display_Init(void);
void Display_SetDigit(uint8_t display_index, uint8_t digit);
void Display_Refresh(uint8_t centenas, uint8_t decenas, uint8_t unidades);

#endif