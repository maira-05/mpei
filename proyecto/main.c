#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio.h"
#include "system.h"
#include "tft.h"
#include "uart.h"
#include "sd.h"

#define BOTON_PIN 13    // PC13: boton azul B1 de la Nucleo (da 0 al presionarlo)

static uint8_t sector[512];   // buffer de un sector (va en .bss, no en la pila)

int main(void)
{
    // Configuracion del boton
    GPIO_InitTypeDef boton = {GPIO_MODO_ENTRADA, BOTON_PIN, GPIO_SIN_PULL, GPIO_BAJA_VELOCIDAD, 0};
    GPIO_Config(GPIOC, boton);

    // Iniciar la pantalla, pintarla y dejarla apagada
    tft_init();
    tft_llenar(TFT_AZUL);     // color que se vera al encender
    tft_apagar();

    // UART para ver mensajes en el PC
    uart_init(115200);
    uart_enviar_texto("\r\nHola desde la Nucleo\r\n");

    // Iniciar la microSD (imprime la respuesta de cada comando)
    sd_init();

    // Leer el sector 0 y mostrar los primeros 16 bytes y la firma
    uint8_t res = sd_leer_sector(0, sector);
    uart_enviar_texto("Leer sector 0: ");
    uart_hexadecimal(res);
    uart_enviar_texto("\r\nPrimeros 16 bytes: ");
    for (int i = 0; i < 16; i++) { uart_hexadecimal(sector[i]); uart_enviar_texto(" "); }
    uart_enviar_texto("\r\nUltimos 2 bytes (deben ser 55 AA): ");
    uart_hexadecimal(sector[510]); uart_enviar_texto(" ");
    uart_hexadecimal(sector[511]);
    uart_enviar_texto("\r\n");

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
                uart_enviar_texto("Pantalla apagada\r\n");
            } else {
                tft_encender();
                encendido = 1;
                uart_enviar_texto("Pantalla encendida\r\n");
            }
            delay_ms(50);                   // antirrebote
        }

        anterior = actual;                  // recordar para la proxima vuelta
    }
}