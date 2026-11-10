/*
 * tft_text.c
 * Dibuja texto en la ST7789 con fuentes en formato Adafruit GFX.
 *
 * Idea: cada carácter se dibuja como un rectángulo de
 *   ancho = xAdvance  y  alto = yAdvance (alto de línea).
 * Se abre esa ventana en la pantalla (CASET/RASET + RAMWR) y se envían
 * todos sus píxeles: color de letra donde el bit vale 1 y color de fondo
 * donde vale 0. Así el fondo queda pintado y no hace falta borrar antes
 * (esto evita el parpadeo que menciona la guía de Adafruit).
 */
#include "tft_text.h"

/* ---- Funciones que ya deberían existir en tu tft.c ----
 * Cámbialas por los nombres que tú usaste en tft_fill():
 *   tft_set_window(x0, y0, x1, y1): envía CASET, RASET y RAMWR
 *   tft_push_color(color): envía un píxel RGB565 (byte alto primero)
 */
extern void tft_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
extern void tft_push_color(uint16_t color);

/* Distancia desde la parte de arriba de la celda hasta la línea base.
 * Es el yOffset más negativo de todos los glifos (el carácter más alto). */
static int8_t ascenso(const GFXfont *f)
{
    int8_t min = 0;
    for (uint16_t i = 0; i <= f->last - f->first; i++) {
        if (f->glyph[i].yOffset < min) min = f->glyph[i].yOffset;
    }
    return (int8_t)(-min);
}

uint16_t tft_draw_char(uint16_t x, uint16_t y, char c, const GFXfont *f,
                       uint16_t color, uint16_t fondo)
{
    if ((uint8_t)c < f->first || (uint8_t)c > f->last) c = '?';

    const GFXglyph *g = &f->glyph[(uint8_t)c - f->first];
    const uint8_t  *bits = f->bitmap;

    uint8_t ancho_celda = g->xAdvance;
    uint8_t alto_celda  = f->yAdvance;
    int8_t  base        = ascenso(f);   /* fila de la línea base dentro de la celda */

    tft_set_window(x, y, x + ancho_celda - 1, y + alto_celda - 1);

    for (int cy = 0; cy < alto_celda; cy++) {
        /* Fila correspondiente dentro del dibujo del glifo */
        int gy = cy - (base + g->yOffset);

        for (int cx = 0; cx < ancho_celda; cx++) {
            /* Columna correspondiente dentro del dibujo del glifo */
            int gx = cx - g->xOffset;
            uint16_t px = fondo;

            if (gx >= 0 && gx < g->width && gy >= 0 && gy < g->height) {
                /* Los bits del glifo están seguidos, fila tras fila, sin relleno */
                uint32_t n = (uint32_t)g->bitmapOffset * 8u
                           + (uint32_t)gy * g->width + (uint32_t)gx;
                if (bits[n >> 3] & (0x80u >> (n & 7u))) px = color;
            }
            tft_push_color(px);
        }
    }
    return ancho_celda;
}

uint16_t tft_draw_text(uint16_t x, uint16_t y, const char *s, const GFXfont *f,
                       uint16_t color, uint16_t fondo)
{
    while (*s) {
        x += tft_draw_char(x, y, *s++, f, color, fondo);
    }
    return x;
}

uint16_t tft_text_width(const char *s, const GFXfont *f)
{
    uint16_t w = 0;
    while (*s) {
        char c = *s++;
        if ((uint8_t)c < f->first || (uint8_t)c > f->last) c = '?';
        w += f->glyph[(uint8_t)c - f->first].xAdvance;
    }
    return w;
}
