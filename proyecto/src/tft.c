#include "tft.h"
#include "gpio.h"
#include "spi.h"
#include "system.h"
#include "stm32f4xx.h"
//Funciones para controlar la pantalla TFT ST7789 de 240x320 píxeles, con interfaz SPI2 (PB13 SCK, PB15 MOSI, PB14 DC, PB12 CS, PB1 RST)
void tft_comando(uint8_t cmd)
{
    spi2_esperar(); // esperar a que termine de enviar el byte anterior
    escribir_pin_state(GPIOB, TFT_DC, 0);// DC=0 para indicar que se va a enviar un comando
    spi2_enviar(cmd);// enviar el comando
    spi2_esperar();
    escribir_pin_state(GPIOB, TFT_DC, 1); // DC=1 para indicar que se va a enviar un dato
}

void tft_dato(uint8_t dato)
{
    spi2_enviar(dato); // enviar el dato (DC=1)
}

void tft_init(void)
{

    GPIO_InitTypeDef pin_cs  = {GPIO_MODO_SALIDA, TFT_CS,  GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0}; // CS
    GPIO_InitTypeDef pin_dc  = {GPIO_MODO_SALIDA, TFT_DC,  GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0}; // DC
    GPIO_InitTypeDef pin_rst = {GPIO_MODO_SALIDA, TFT_RST, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0}; // RST
    GPIO_Config(GPIOB, pin_cs);
    GPIO_Config(GPIOB, pin_dc);
    GPIO_Config(GPIOB, pin_rst);

    spi2_init(); // inicializar SPI2 en modo maestro, CPOL=1, CPHA=1 (modo 3), 8 bits, pines PB13/PB15

    escribir_pin_state(GPIOB, TFT_RST, 0);
    delay_ms(100);
    escribir_pin_state(GPIOB, TFT_RST, 1);
    delay_ms(100);
    escribir_pin_state(GPIOB, TFT_CS, 0); //CS en bajo ya que la pantalla es el unico que es de SPI2 y no hay conflicto con otros dispositivos SPI

    tft_comando(0x01); // software reset: comando que reinicia los registros internos de la pantalla 
    delay_ms(150);
    tft_comando(0x11);// sleep out
    delay_ms(500);
    tft_comando(0x3A);// COLMOD: pixel format
    tft_dato(0x55); // 16 bits/pixel (RGB565)
    tft_comando(0x36);// MADCTL: memory data access control
    tft_dato(0x00); // configuración estándar es la orientación vertical (portrait) con el origen en la esquina superior izquierda
    tft_comando(0x20);// inversion off, antes estaba con 0x21 pero los colores se veian raros, con 0x20 se ve bien
    tft_comando(0x29);// display on
}

void tft_ventana(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    tft_comando(0x2A);// CASET: column address set
    tft_dato(x0 >> 8); tft_dato(x0 & 0xFF);// enviar los 2 bytes de x0
    tft_dato(x1 >> 8); tft_dato(x1 & 0xFF);// enviar los 2 bytes de x1

    tft_comando(0x2B);// RASET: row address set
    tft_dato(y0 >> 8); tft_dato(y0 & 0xFF);// enviar los 2 bytes de y0
    tft_dato(y1 >> 8); tft_dato(y1 & 0xFF);// enviar los 2 bytes de y1

    tft_comando(0x2C);// RAMWR: memory write
}

void tft_pixel(uint16_t color)
{
    tft_dato(color >> 8);// enviar el byte alto del color
    tft_dato(color & 0xFF);// enviar el byte bajo del color
}

void tft_rectangulo(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint16_t color)
{
    tft_ventana(x, y, x + ancho - 1, y + alto - 1); // define la ventana de dibujo
    for (uint32_t i = 0; i < (uint32_t)ancho * alto; i++)// enviar todos los píxeles del rectángulo
    {
        tft_pixel(color);// enviar el color del píxel
    }
}

void tft_llenar(uint16_t color)
{
    tft_rectangulo(0, 0, TFT_ANCHO, TFT_ALTO, color);// llenar toda la pantalla con el color especificado
}

void tft_apagar(void)
{
    tft_comando(0x28);// display off: deja de mostrar la imagen (la luz de fondo depende de su propio pin, no de este comando)
    delay_ms(10);
    tft_comando(0x10);// sleep in: bajo consumo de energia, pero no se apaga la alimentacion de la pantalla
}

void tft_encender(void)
{
    tft_comando(0x11);// sleep out
    delay_ms(120);
    tft_comando(0x29);// display on
}
