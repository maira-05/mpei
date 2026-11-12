#include "tft.h"
#include "gpio.h"
#include "spi.h"
#include "system.h"
#include "stm32f4xx.h"

void tft_comando(uint8_t cmd)
{
    spi2_esperar();
    escribir_pin_state(GPIOB, TFT_DC, 0);
    spi2_enviar(cmd);
    spi2_esperar();
    escribir_pin_state(GPIOB, TFT_DC, 1);
}

void tft_dato(uint8_t dato)
{
    spi2_enviar(dato);
}

void tft_init(void)
{

    GPIO_InitTypeDef pin_cs  = {GPIO_MODO_SALIDA, TFT_CS,  GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0};
    GPIO_InitTypeDef pin_dc  = {GPIO_MODO_SALIDA, TFT_DC,  GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0};
    GPIO_InitTypeDef pin_rst = {GPIO_MODO_SALIDA, TFT_RST, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0};
    GPIO_Config(GPIOB, pin_cs);
    GPIO_Config(GPIOB, pin_dc);
    GPIO_Config(GPIOB, pin_rst);

    spi2_init();

    escribir_pin_state(GPIOB, TFT_RST, 0);
    delay_ms(100);
    escribir_pin_state(GPIOB, TFT_RST, 1);
    delay_ms(100);
    escribir_pin_state(GPIOB, TFT_CS, 0);

    tft_comando(0x01);
    delay_ms(150);
    tft_comando(0x11);
    delay_ms(500);
    tft_comando(0x3A);
    tft_dato(0x55);
    tft_comando(0x36);
    tft_dato(0x00);
    tft_comando(0x20);
    tft_comando(0x29);
}

void tft_ventana(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    tft_comando(0x2A);
    tft_dato(x0 >> 8); tft_dato(x0 & 0xFF);
    tft_dato(x1 >> 8); tft_dato(x1 & 0xFF);

    tft_comando(0x2B);
    tft_dato(y0 >> 8); tft_dato(y0 & 0xFF);
    tft_dato(y1 >> 8); tft_dato(y1 & 0xFF);

    tft_comando(0x2C);
}

void tft_pixel(uint16_t color)
{
    tft_dato(color >> 8);
    tft_dato(color & 0xFF);
}

void tft_rectangulo(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint16_t color)
{
    tft_ventana(x, y, x + ancho - 1, y + alto - 1);
    for (uint32_t i = 0; i < (uint32_t)ancho * alto; i++)
    {
        tft_pixel(color);
    }
}

void tft_llenar(uint16_t color)
{
    tft_rectangulo(0, 0, TFT_ANCHO, TFT_ALTO, color);
}

void tft_apagar(void)
{
    tft_comando(0x28);
    delay_ms(10);
    tft_comando(0x10);
}

void tft_encender(void)
{
    tft_comando(0x11);
    delay_ms(120);
    tft_comando(0x29);
}
