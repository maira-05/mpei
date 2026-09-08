#include <stdint.h>
#include "stm32f401.h"
#include "gpio_config.h"
#include "display.h"

uint8_t leer_dip_switch(void)
{
    return (uint8_t)(GPIOC->IDR & 0xFF); 
}

int main(void)
{
    GPIO_Config();     
    Display_Init();    

    while(1)
    {
        uint8_t valor = leer_dip_switch();

        uint8_t centenas = valor / 100;
        uint8_t decenas  = (valor / 10) % 10;
        uint8_t unidades = valor % 10;

        Display_Refresh(centenas, decenas, unidades);
    }
}