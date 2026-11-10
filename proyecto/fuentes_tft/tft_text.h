/*
 * tft_text.h
 * Dibujo de texto en la pantalla ST7789 usando fuentes en formato Adafruit GFX.
 */
#ifndef TFT_TEXT_H
#define TFT_TEXT_H

#include <stdint.h>
#include "gfxfont.h"

/* Colores RGB565 útiles */
#define COLOR_NEGRO     0x0000
#define COLOR_BLANCO    0xFFFF
#define COLOR_ROJO      0xF800
#define COLOR_VERDE     0x07E0
#define COLOR_AZUL      0x001F
#define COLOR_AMARILLO  0xFFE0
#define COLOR_GRIS      0x8410

/* Dibuja un carácter. (x, y) es la esquina SUPERIOR izquierda de la celda.
 * Pinta también el fondo, así que no hay que borrar antes.
 * Devuelve el ancho que ocupó (xAdvance) para saber dónde va el siguiente. */
uint16_t tft_draw_char(uint16_t x, uint16_t y, char c, const GFXfont *f,
                       uint16_t color, uint16_t fondo);

/* Dibuja una cadena completa empezando en (x, y). Devuelve el x final. */
uint16_t tft_draw_text(uint16_t x, uint16_t y, const char *s, const GFXfont *f,
                       uint16_t color, uint16_t fondo);

/* Ancho en píxeles que ocuparía una cadena (para centrar texto) */
uint16_t tft_text_width(const char *s, const GFXfont *f);

#endif
