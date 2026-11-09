#include "tft.h"
#include "gpio.h"
#include "spi.h"
#include "system.h"
#include "stm32f4xx.h"

void tft_comando(uint8_t cmd)
{
    spi2_esperar(); // Asegurarse de que la transmisión anterior haya terminado
    escribir_pin_state(GPIOB, TFT_DC, 0); // DC = 0 para comando
    spi2_enviar(cmd); // Enviar el comando
    spi2_esperar(); // Asegurarse de que la transmisión haya terminado antes de cambiar el estado del pin
    escribir_pin_state(GPIOB, TFT_DC, 1); // CS = 1 para deseleccionar la pantalla
}

void tft_dato(uint8_t dato)
{
    spi2_enviar(dato);// Enviar el dato
}

void tft_init(void)
{
    // Configuración de pines para los pines de la pantalla TFT (CS, DC, RST) y a que 
    GPIO_InitTypeDef pin_cs = {GPIO_MODO_SALIDA, TFT_CS, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0}; // salida, sin pull, alta velocidad, sin función alternativa
    GPIO_InitTypeDef pin_dc = {GPIO_MODO_SALIDA, TFT_DC, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0}; // salida, sin pull, alta velocidad, sin función alternativa
    GPIO_InitTypeDef pin_rst = {GPIO_MODO_SALIDA, TFT_RST, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0}; // salida, sin pull, alta velocidad, sin función alternativa
    
    //Configuro los pines del puerto GPIOB con la estructura de configuración
    GPIO_Config(GPIOB, pin_cs); // Configurar el pin CS, para seleccionar la pantalla
    GPIO_Config(GPIOB, pin_dc);// Configurar el pin DC, para indicar si es comando o dato
    GPIO_Config(GPIOB, pin_rst);// Configurar el pin RST, para resetear la pantalla

    // Inicialización de SPI2
    spi2_init(); // Inicializar SPI2, para comunicarme con la pantalla

    // Reset de la pantalla
    escribir_pin_state(GPIOB, TFT_RST, 0);
    delay_ms(100);
    escribir_pin_state(GPIOB, TFT_RST, 1);
    delay_ms(100);
    escribir_pin_state(GPIOB, TFT_CS, 0); // CS = 0 para seleccionar la pantalla

    // Secuencia de inicialización de la pantalla (según el datasheet)
    tft_comando(0x01); // Software reset
    delay_ms(150);
    tft_comando(0x11); // Sleep out
    delay_ms(500);
    tft_comando(0x3A); // COLMOD: Pixel Format Set
    tft_dato(0x55);    // 16 bits/pixel   
    tft_comando(0x36); // MADCTL: Memory Data Access Control
    tft_dato(0x00);    
    tft_comando(0x20);
    tft_comando(0x29); // Display on

}

void tft_llenar(uint16_t color)
{
    // Columnas 0 a 239 (CASET)
    tft_comando(0x2A);
    tft_dato(0); tft_dato(0);                           // inicio = 0
    tft_dato((TFT_ANCHO - 1) >> 8); tft_dato((TFT_ANCHO - 1) & 0xFF);   // fin = 239

    // Filas 0 a 319 (RASET): mismo formato que CASET
    tft_comando(0x2B);
    tft_dato(0); tft_dato(0);                           // inicio = 0
    tft_dato((TFT_ALTO - 1) >> 8); tft_dato((TFT_ALTO - 1) & 0xFF);    // fin = 319

    // Escribir pixeles (RAMWR)
    tft_comando(0x2C);
    for (uint32_t i = 0; i < TFT_ANCHO * TFT_ALTO; i++)       // ¿cuántos pixeles tiene la pantalla?
    {
        tft_dato(color >> 8);                // byte alto del color
        tft_dato(color & 0xFF);                       // byte bajo del color
    }
}

void tft_apagar(void)
{
    tft_comando(0x28); // Display off
    delay_ms(10);
    tft_comando(0x10); // Enter sleep mode
}

void tft_encender(void)
{
    tft_comando(0x11); // Sleep out
    delay_ms(120);
    tft_comando(0x29); // Display on
}

