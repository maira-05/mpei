#ifndef __TFT_H__
#define __TFT_H__
#include <stdint.h>

// Pines de control (puerto B) 
#define TFT_CS    12   // PB12 
#define TFT_DC    14   // PB14
#define TFT_RST    1   // PB1

#define TFT_ANCHO  240
#define TFT_ALTO   320

// Colores RGB565: 16 bits = RRRRR GGGGGG BBBBB (5 bits rojo, 6 verde, 5 azul)
#define TFT_NEGRO   0x0000
#define TFT_BLANCO  0xFFFF
#define TFT_ROJO    0xF800 // Rojo al máximo = 11111
#define TFT_VERDE   0x07E0 //verde al máximo = 111111
#define TFT_AZUL    0x001F //azul al máximo = 11111 donde el rojo y el verde estan en 0
#define TFT_AMARILLO     0xFFE0   // P12: en RGB565 = RRRRR GGGGGG BBBBB. Amarillo = rojo + verde (11111 111111 00000	)
#define TFT_CIAN         0x07FF // Cian = verde + azul (00000 111111 11111	)
#define TFT_GRIS         0x8410// Gris = rojo + verde + azul (10000 100000 10000	)
#define TFT_NARANJA      0xFD20// Naranja = rojo + verde (11111 101001 00000)
#define TFT_AZUL_OSCURO  0x0010 //Azul oscuro = azul en la mitad (00000 000000 10000)

void tft_init(void); // configura pines, SPI y la pantalla 
void tft_comando(uint8_t cmd);     // envía un comando (DC = 0)
void tft_dato(uint8_t dato);       // envía un dato (DC = 1)
void tft_llenar(uint16_t color);   // pinta toda la pantalla
void tft_ventana(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1); // define la ventana de dibujo
void tft_pixel(uint16_t color);    // envía un píxel a la ventana actual
void tft_rectangulo(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint16_t color); // rectángulo lleno
void tft_encender(void);           // SLPOUT + DISPON
void tft_apagar(void);             // DISPOFF + SLPIN
    
#endif