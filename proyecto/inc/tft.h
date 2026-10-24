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

void tft_init(void);               /* configura pines, SPI y la pantalla */
void tft_comando(uint8_t cmd);     /* envía un comando (DC = 0) */
void tft_dato(uint8_t dato);       /* envía un dato (DC = 1) */
void tft_llenar(uint16_t color);   /* pinta toda la pantalla */
void tft_encender(void);           /* SLPOUT + DISPON */
void tft_apagar(void);             /* DISPOFF + SLPIN */

#endif