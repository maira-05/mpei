/*
 * gfxfont.h
 * Estructuras que usan los archivos de fuente de Adafruit GFX (carpeta Fonts/).
 * Es lo único que necesitamos de esa librería para poder leer sus fuentes.
 */
#ifndef GFXFONT_H
#define GFXFONT_H

#include <stdint.h>

/* En Arduino AVR, PROGMEM pone los datos en Flash. En el STM32 todo lo
 * que es "const" ya queda en Flash, así que lo definimos vacío. */
#ifndef PROGMEM
#define PROGMEM
#endif

/* Información de UN carácter (glifo) */
typedef struct {
    uint16_t bitmapOffset; /* Dónde empieza su dibujo dentro del arreglo de bits */
    uint8_t  width;        /* Ancho del dibujo en píxeles */
    uint8_t  height;       /* Alto del dibujo en píxeles */
    uint8_t  xAdvance;     /* Cuánto avanza el cursor después de este carácter */
    int8_t   xOffset;      /* Corrimiento en x desde el cursor hasta el dibujo */
    int8_t   yOffset;      /* Corrimiento en y desde la línea base (negativo = hacia arriba) */
} GFXglyph;

/* Información de la fuente completa */
typedef struct {
    uint8_t  *bitmap;   /* Todos los bits de todos los caracteres, seguidos */
    GFXglyph *glyph;    /* Tabla de glifos */
    uint16_t  first;    /* Primer carácter ASCII incluido (normalmente 0x20 = ' ') */
    uint16_t  last;     /* Último carácter ASCII incluido (normalmente 0x7E = '~') */
    uint8_t   yAdvance; /* Alto de una línea de texto (salto de línea) */
} GFXfont;

#endif
