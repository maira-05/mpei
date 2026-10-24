#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio.h"
#include "system.h"
#include "tft.h"

#define BOTON_PIN 13    // PC13: boton azul B1 de la Nucleo (da 0 al presionarlo)

int main(void)
{
    // Configuracion del boton
    GPIO_InitTypeDef boton = {GPIO_MODO_ENTRADA, BOTON_PIN, GPIO_SIN_PULL, GPIO_BAJA_VELOCIDAD, 0};
    GPIO_Config(GPIOC, boton);

    // Iniciar la pantalla, pintarla y dejarla apagada
    tft_init();
    tft_llenar(TFT_AZUL);     // color que se vera al encender
    tft_apagar();

    uint32_t encendido = 0;   // 0 = pantalla apagada, 1 = encendida
    uint32_t anterior  = 1;   // 1 = boton suelto

    while (1)
    {
        uint32_t actual = leer_pin_state(GPIOC, BOTON_PIN);

        if (anterior == 1 && actual == 0)   // se acaba de presionar
        {
            if (encendido == 1) {
                tft_apagar();
                encendido = 0;
            } else {
                tft_encender();
                encendido = 1;
            }
            delay_ms(50);                   // antirrebote
        }

        anterior = actual;                  // recordar para la proxima vuelta
    }
}