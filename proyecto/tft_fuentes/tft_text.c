#include "tft_text.h"
#include "tft.h"

static int8_t ascenso(const GFXfont *fuente)
{
    int8_t min = 0;
    for (uint16_t i = 0; i <= fuente->last - fuente->first; i++)
    {
        if (fuente->glyph[i].yOffset < min) min = fuente->glyph[i].yOffset;
    }
    return (int8_t)(-min);
}

static const GFXglyph *glifo(char c, const GFXfont *fuente)
{
    if ((uint8_t)c < fuente->first || (uint8_t)c > fuente->last) c = '?';
    return &fuente->glyph[(uint8_t)c - fuente->first];
}

uint16_t tft_letra(uint16_t x, uint16_t y, char c, const GFXfont *fuente,
                   uint16_t color, uint16_t fondo)
{
    const GFXglyph *g = glifo(c, fuente);
    uint8_t ancho_celda = g->xAdvance;
    uint8_t alto_celda  = fuente->yAdvance;
    int8_t  base        = ascenso(fuente);

    if (x + ancho_celda > TFT_ANCHO || y + alto_celda > TFT_ALTO) return 0;

    tft_ventana(x, y, x + ancho_celda - 1, y + alto_celda - 1);

    for (int cy = 0; cy < alto_celda; cy++)
    {
        int gy = cy - (base + g->yOffset);
        for (int cx = 0; cx < ancho_celda; cx++)
        {
            int gx = cx - g->xOffset;
            uint16_t px = fondo;

            if (gx >= 0 && gx < g->width && gy >= 0 && gy < g->height)
            {
                uint32_t n = (uint32_t)g->bitmapOffset * 8u
                           + (uint32_t)gy * g->width + (uint32_t)gx;
                if (fuente->bitmap[n >> 3] & (0x80u >> (n & 7u))) px = color;
            }
            tft_pixel(px);
        }
    }
    return ancho_celda;
}

uint16_t tft_texto(uint16_t x, uint16_t y, const char *s, const GFXfont *fuente,
                   uint16_t color, uint16_t fondo)
{
    while (*s)
    {
        uint16_t avance = tft_letra(x, y, *s, fuente, color, fondo);
        if (avance == 0) break;
        x += avance;
        s++;
    }
    return x;
}

uint16_t tft_ancho_texto(const char *s, const GFXfont *fuente)
{
    uint16_t ancho = 0;
    while (*s)
    {
        ancho += glifo(*s, fuente)->xAdvance;
        s++;
    }
    return ancho;
}

void tft_texto_centrado(uint16_t y, const char *s, const GFXfont *fuente,
                        uint16_t color, uint16_t fondo)
{
    uint16_t ancho = tft_ancho_texto(s, fuente);
    uint16_t x = (ancho < TFT_ANCHO) ? (TFT_ANCHO - ancho) / 2 : 0;
    tft_texto(x, y, s, fuente, color, fondo);
}
