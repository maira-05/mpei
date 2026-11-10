#ifndef __TFT_TEXT_H__
#define __TFT_TEXT_H__
#include <stdint.h>
#include "gfxfont.h"

/* Dibujo de texto en la pantalla con fuentes en formato Adafruit GFX (GFXfont).
 * (x, y) es la esquina SUPERIOR izquierda del texto.
 * Se pinta también el fondo de cada letra, así que no hay que borrar antes.
 * Los colores están en tft.h (TFT_NEGRO, TFT_BLANCO, ...). */

uint16_t tft_letra(uint16_t x, uint16_t y, char c, const GFXfont *fuente,
                   uint16_t color, uint16_t fondo);      /* devuelve el ancho que ocupó */

uint16_t tft_texto(uint16_t x, uint16_t y, const char *s, const GFXfont *fuente,
                   uint16_t color, uint16_t fondo);      /* devuelve la x donde terminó */

/* T1: ¿qué devuelve esta función y qué parámetros necesita?
 *     Pista: devuelve un ancho en píxeles (mismo tipo que tft_letra) y solo
 *     necesita el texto y la fuente. Miren la función en tft_text.c. */
uint16_t tft_ancho_texto(const char *s, const GFXfont *fuente); /* ancho en píxeles, para centrar */

/* T2: igual que T1. Pista: no devuelve nada; no recibe x porque la calcula ella. */
void tft_texto_centrado(uint16_t y, const char *s, const GFXfont *fuente,
uint16_t color, uint16_t fondo);      /* centrado horizontalmente */

#endif