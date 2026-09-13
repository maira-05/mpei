#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__
#include "stm32f401.h"
#include "stdint.h"


#define NUM_CELLS 9

extern const uint8_t button_pins[NUM_CELLS];
extern const uint8_t red_pins[NUM_CELLS];
extern const uint8_t green_pins[NUM_CELLS];


void GPIO_Config_Triqui(void);
void GPIO_Config(void);

uint8_t read_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin);
void write_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state);

#endif // __GPIO_CONFIG_H__