#include "pantalla.h"
#include "tft.h"
#include "tft_text.h"

#include "Fonts/FreeSansBold9pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "Fonts/FreeSansBold18pt7b.h"

#define BARRA_ALTO       30
#define Y_NOMBRE         90
#define MAX_LINEAS        3
#define ANCHO_UTIL      224
#define Y_CONTADOR      200
#define X_CONTADOR       83
#define MAX_TEXTO        40

static void texto_linea(uint16_t x, uint16_t y, const char *s, const GFXfont *fuente,
                        uint16_t color, uint16_t fondo)
{
    uint16_t fin = tft_texto(x, y, s, fuente, color, fondo);
    if (fin < TFT_ANCHO)
        tft_rectangulo(fin, y, TFT_ANCHO - fin, fuente->yAdvance, fondo);
}

static void linea_centrada(uint16_t y, const char *s, const GFXfont *fuente,
                           uint16_t color, uint16_t fondo)
{
    uint16_t ancho = tft_ancho_texto(s, fuente);
    uint16_t x = (ancho < TFT_ANCHO) ? (TFT_ANCHO - ancho) / 2 : 0;
    if (x > 0) tft_rectangulo(0, y, x, fuente->yAdvance, fondo);
    texto_linea(x, y, s, fuente, color, fondo);
}

static void nombre_en_lineas(const char *nombre)
{
    const GFXfont *f = &FreeSansBold12pt7b;
    char linea[MAX_TEXTO];
    char prueba[MAX_TEXTO];
    uint8_t largo = 0;
    uint8_t num_linea = 0;
    const char *p = nombre;

    while (*p && num_linea < MAX_LINEAS)
    {
        while (*p == ' ') p++;
        if (*p == '\0') break;

        uint8_t i = 0;
        if (largo > 0)
        {
            for (i = 0; i < largo; i++) prueba[i] = linea[i];
            prueba[i++] = ' ';
        }
        const char *q = p;
        while (*q && *q != ' ' && i < MAX_TEXTO - 1) prueba[i++] = *q++;
        prueba[i] = '\0';

        if (largo == 0 || tft_ancho_texto(prueba, f) <= ANCHO_UTIL)
        {
            for (uint8_t k = 0; k <= i; k++) linea[k] = prueba[k];
            largo = i;
            p = q;
        }
        else
        {
            linea_centrada(Y_NOMBRE + num_linea * f->yAdvance, linea, f, TFT_AMARILLO, TFT_NEGRO);
            num_linea++;
            largo = 0;
        }
    }

    if (largo > 0 && num_linea < MAX_LINEAS)
    {
        linea_centrada(Y_NOMBRE + num_linea * f->yAdvance, linea, f, TFT_AMARILLO, TFT_NEGRO);
        num_linea++;
    }

    for (; num_linea < MAX_LINEAS; num_linea++)
        tft_rectangulo(0, Y_NOMBRE + num_linea * f->yAdvance, TFT_ANCHO, f->yAdvance, TFT_NEGRO);
}

void pantalla_inicio(void)
{
    tft_llenar(TFT_NEGRO);
    tft_rectangulo(0, 0, TFT_ANCHO, BARRA_ALTO, TFT_AZUL_OSCURO);
    tft_rectangulo(0, TFT_ALTO - BARRA_ALTO, TFT_ANCHO, BARRA_ALTO, TFT_AZUL_OSCURO);
    tft_texto_centrado(TFT_ALTO - BARRA_ALTO + 4, "Prev  Pausa  Stop  Next",
                       &FreeSansBold9pt7b, TFT_GRIS, TFT_AZUL_OSCURO);
    tft_texto_centrado(45, "EPIC", &FreeSansBold18pt7b, TFT_NARANJA, TFT_NEGRO);
}

void pantalla_estado(const char *texto)
{
    texto_linea(8, 4, texto, &FreeSansBold9pt7b, TFT_BLANCO, TFT_AZUL_OSCURO);
}

void pantalla_cancion(uint8_t numero, const char *nombre)
{
    nombre_en_lineas(nombre);

    char contador[] = "00 / 40";
    contador[0] = '0' + (numero / 10);
    contador[1] = '0' + (numero % 10);
    texto_linea(X_CONTADOR, Y_CONTADOR, contador, &FreeSansBold12pt7b, TFT_BLANCO, TFT_NEGRO);
}
