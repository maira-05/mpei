/* =====================================================================
 * PRUEBA DE PINES de la pantalla (sin SPI)
 * Pone en ALTO (3.3 V) un pin a la vez, 2 segundos cada uno, en este
 * orden:  PB12 (CS) -> PB13 (SCK) -> PB15 (SDA) -> PB14 (DC) -> PB1 (RST)
 * El LED verde (LD2) se enciende SOLO durante el primer pin (PB12),
 * para saber donde empieza la secuencia.
 *
 * Como medir: multimetro en voltaje DC, punta negra en GND y punta roja
 * en el pin de la PANTALLA (no en la Nucleo). Cada pin debe marcar
 * ~3.3 V durante sus 2 s y 0 V el resto del tiempo.
 * ===================================================================== */
#include <stdint.h>
#include "stm32f4xx.h"

static void delay_ms(uint32_t ms)
{
    SysTick->LOAD = 16000 - 1;                 /* 1 ms con HSI de 16 MHz */
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    while (ms--) {
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) { }
    }
    SysTick->CTRL = 0;
}

static void pin_salida(GPIO_TypeDef *port, uint32_t pin)
{
    port->MODER &= ~(3U << (pin * 2));
    port->MODER |=  (1U << (pin * 2));         /* 01 = salida */
}

int main(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

    const uint32_t pines[5] = { 12, 13, 15, 14, 1 };   /* CS, SCK, SDA, DC, RST */

    pin_salida(GPIOA, 5);                      /* LED LD2 */
    for (int i = 0; i < 5; i++) pin_salida(GPIOB, pines[i]);

    while (1) {
        for (int i = 0; i < 5; i++) {
            GPIOB->ODR &= ~((1U << 12) | (1U << 13) | (1U << 14) | (1U << 15) | (1U << 1));
            GPIOB->ODR |=  (1U << pines[i]);   /* solo este pin en alto */
            if (i == 0) GPIOA->BSRR = (1U << 5);          /* LED on  */
            else        GPIOA->BSRR = (1U << (5 + 16));   /* LED off */
            delay_ms(2000);
        }
    }
}
