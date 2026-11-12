#ifndef __TFT_TEXT_H__
#define __TFT_TEXT_H__
#include <stdint.h>
#include "gfxfont.h"

uint16_t tft_letra(uint16_t x, uint16_t y, char c, const GFXfont *fuente,
                   uint16_t color, uint16_t fondo);
uint16_t tft_texto(uint16_t x, uint16_t y, const char *s, const GFXfont *fuente,
                   uint16_t color, uint16_t fondo);
uint16_t tft_ancho_texto(const char *s, const GFXfont *fuente);
void     tft_texto_centrado(uint16_t y, const char *s, const GFXfont *fuente,
                            uint16_t color, uint16_t fondo);

#endif
