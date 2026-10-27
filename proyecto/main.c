#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio.h"
#include "system.h"
#include "tft.h"
#include "uart.h"
#include "sd.h"

#define BOTON_PIN 13    // PC13: boton azul B1 de la Nucleo (da 0 al presionarlo)

int main(void)
{
    // UART para ver mensajes en el PC
    uart_init(115200);
    uart_enviar_texto("Prueba de pines del modulo SD\r\n");

    // ===== PRUEBA TEMPORAL: pines del SPI1 fijos (sin SPI) =====
    // PB3 (SCK), PB5 (MOSI) y PB6 (CS) como salidas normales.
    // Cada 5 segundos cambian entre ALTO (3.3 V) y BAJO (0 V).
    GPIO_InitTypeDef p = {GPIO_MODO_SALIDA, 3, GPIO_SIN_PULL, GPIO_BAJA_VELOCIDAD, 0};
    GPIO_Config(GPIOB, p);              // PB3 (SCK)
    p.Pin = 5;
    GPIO_Config(GPIOB, p);              // PB5 (MOSI)
    p.Pin = 6;
    GPIO_Config(GPIOB, p);              // PB6 (CS)

    while (1)
    {
        escribir_pin_state(GPIOB, 3, 1);
        escribir_pin_state(GPIOB, 5, 1);
        escribir_pin_state(GPIOB, 6, 1);
        uart_enviar_texto("ALTO: SCK, MOSI y CS en 3.3 V\r\n");
        delay_ms(5000);

        escribir_pin_state(GPIOB, 3, 0);
        escribir_pin_state(GPIOB, 5, 0);
        escribir_pin_state(GPIOB, 6, 0);
        uart_enviar_texto("BAJO: SCK, MOSI y CS en 0 V\r\n");
        delay_ms(5000);
    }
}