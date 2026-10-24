#include "system.h"
#include "stm32f4xx.h"

void delay_ms(uint32_t ms)//frecuencia del reloj del microcontrolador 16MHz, 1ms = 16000 ciclos
{
    SysTick->LOAD = 16000 - 1; // Carga el valor de recarga para 1 ms
    SysTick->VAL = 0;         // Reinicia el valor actual del contador
    SysTick->CTRL = 5;        // Habilita el temporizador SysTick
    while (ms > 0) {
        while ((SysTick->CTRL & 0x10000) == 0); // Espera hasta que el contador llegue a cero
        ms--;
    }  
    SysTick->CTRL = 0; // Deshabilita el temporizador SysTick
}