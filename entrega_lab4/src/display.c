#include "display.h"
#include "stm32f401.h"
#include "gpio_config.h"


static const uint8_t tabla_7seg[10] = {
    0b0111111, // 0
    0b0000110, // 1
    0b1011011, // 2
    0b1001111, // 3
    0b1100110, // 4
    0b1101101, // 5
    0b1111101, // 6
    0b0000111, // 7
    0b1111111, // 8
    0b1101111  // 9
};

void Display_Init(void)
{
    // Configura PB0-PB6 como salida (segmentos)
    // Configura PB8, PB9, PB10 como salida (selectores de dígito)
    // (llamadas a write_pin_state / configuración de MODER van aquí)
}

// Enciende un solo display con un dígito específico
void Display_SetDigit(uint8_t display_index, uint8_t digit)
{
    uint8_t patron = tabla_7seg[digit];

    
    for (int i = 0; i < 7; i++) {
        write_pin_state(GPIOB, i, (patron >> i) & 0x01);
    }

    
    write_pin_state(GPIOB, 8, 0);
    write_pin_state(GPIOB, 9, 0);
    write_pin_state(GPIOB, 10, 0);

    write_pin_state(GPIOB, 8 + display_index, 1);
}

void Display_Refresh(uint8_t centenas, uint8_t decenas, uint8_t unidades)
{
    Display_SetDigit(0, unidades);
    for (volatile uint32_t i = 0; i < 3000; i++); 

    Display_SetDigit(1, decenas);
    for (volatile uint32_t i = 0; i < 3000; i++);

    Display_SetDigit(2, centenas);
    for (volatile uint32_t i = 0; i < 3000; i++);
}