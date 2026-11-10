#include "tft_text.h"
#include "tft.h"

/*
 * Cómo se dibuja una letra:
 * Cada letra se dibuja como un rectángulo ("celda") de
 *   ancho = xAdvance de la letra   y   alto = yAdvance de la fuente.
 * Se abre esa ventana en la pantalla con tft_ventana() y se envían todos sus
 * píxeles: color de letra donde el bit del molde vale 1 y color de fondo donde
 * vale 0 o donde la celda queda fuera del molde.
 */

// Distancia desde la parte de arriba de la celda hasta la línea base.
// Es el yOffset más negativo de todas las letras (la letra más alta).
static int8_t ascenso(const GFXfont *fuente)
{
    int8_t min = 0;
    for (uint16_t i = 0; i <= fuente->last - fuente->first; i++)
    {
        if (fuente->glyph[i].yOffset < min) min = fuente->glyph[i].yOffset;
    }
    return (int8_t)(-min);
}

// Devuelve la información (glifo) de una letra; si la fuente no la tiene, usa '?'
static const GFXglyph *glifo(char c, const GFXfont *fuente)
{
    if ((uint8_t)c < fuente->first || (uint8_t)c > fuente->last) c = '?';
    return &fuente->glyph[(uint8_t)c - fuente->first];
}

uint16_t tft_letra(uint16_t x, uint16_t y, char c, const GFXfont *fuente,
                   uint16_t color, uint16_t fondo)
{
    const GFXglyph *g = glifo(c, fuente);
    uint8_t ancho_celda = g->xAdvance;      // cuánto avanza el cursor con esta letra
    uint8_t alto_celda  = fuente->yAdvance; // alto de una línea de texto
    int8_t  base        = ascenso(fuente);  // fila de la línea base dentro de la celda

    // Si la letra se sale por la derecha o por abajo, no se dibuja
    if (x + ancho_celda > TFT_ANCHO || y + alto_celda > TFT_ALTO) return 0;

    tft_ventana(x, y, x + ancho_celda - 1, y + alto_celda - 1);

    for (int cy = 0; cy < alto_celda; cy++)
    {
        int gy = cy - (base + g->yOffset);  // fila correspondiente dentro del molde
        for (int cx = 0; cx < ancho_celda; cx++)
        {
            int gx = cx - g->xOffset;       // columna correspondiente dentro del molde
            uint16_t px = fondo;

            if (gx >= 0 && gx < g->width && gy >= 0 && gy < g->height)
            {
                // Los bits del molde van seguidos, fila tras fila, sin relleno:
                // bit número n = inicio de la letra (en bits) + fila * ancho + columna
                uint32_t n = (uint32_t)g->bitmapOffset * 8u
                           + (uint32_t)gy * g->width + (uint32_t)gx;
                if (fuente->bitmap[n >> 3] & (0x80u >> (n & 7u))) px = color; // n>>3 = byte, n&7 = bit
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
        if (avance == 0) break;             // no cupo: se corta el texto
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