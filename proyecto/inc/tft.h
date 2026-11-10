#ifndef __TFT_H__
#define __TFT_H__
#include <stdint.h>

/* Pines de control (puerto B) */
#define TFT_CS    12   /* PB12 */
#define TFT_DC    14   /* PB14 */
#define TFT_RST    1   /* PB1  */

#define TFT_ANCHO  240
#define TFT_ALTO   320

/* Colores RGB565 */
#define TFT_NEGRO   0x0000
#define TFT_BLANCO  0xFFFF
#define TFT_ROJO    0xF800
#define TFT_VERDE   0x07E0
#define TFT_AZUL    0x001F
#define TFT_AMARILLO     0xFFE0   /* P12: en RGB565 = RRRRR GGGGGG BBBBB. Amarillo = rojo + verde. ¿Por qué da 0xFFE0?
                                     Pista: rojo al máximo = 11111, verde al máximo = 111111, azul = 00000 */
#define TFT_CIAN         0x07FF
#define TFT_GRIS         0x8410
#define TFT_NARANJA      0xFD20
#define TFT_AZUL_OSCURO  0x0010

void tft_init(void);               /* configura pines, SPI y la pantalla */
void tft_comando(uint8_t cmd);     /* envía un comando (DC = 0) */
void tft_dato(uint8_t dato);       /* envía un dato (DC = 1) */
void tft_llenar(uint16_t color);   /* pinta toda la pantalla */
void tft_ventana(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto); // define la ventana de dibujo
void tft_pixel(uint16_t color);    /* envía un píxel a la ventana actual */
void tft_rectangulo(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint16_t color); /* rectángulo lleno */
void tft_encender(void);           /* SLPOUT + DISPON */
void tft_apagar(void);             /* DISPOFF + SLPIN */

#endif