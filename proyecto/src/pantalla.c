// funciones que maneja la pantalla, se llaman desde el main y desde la interrupción del timer
#include "pantalla.h"
#include "tft.h"
#include "tft_text.h"
#include "Fonts/FreeSansBold9pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "Fonts/FreeSansBold18pt7b.h"

// definiciones de constantes para dibujar en la pantalla, se usan en varias funciones de este archivo


#define BARRA_ALTO       30     // bit hasta donde va la barra de estado
#define Y_NOMBRE         90     // bit desde donde empieza a dibujar el nombre de la canción
#define MAX_LINEAS        3     // número máximo de líneas para mostrar el nombre
#define ANCHO_UTIL      224     // 224 para dejar un margen de 8 bits a cada lado, para que no se corte el texto
#define Y_CONTADOR      200     // bit desde donde empieza a dibujar el contador en y
#define X_CONTADOR       83     // bit desde donde empieza a dibujar el contador en x
#define MAX_TEXTO        40     // maximo de caracteres que puede tener el nombre de la canción, para no desbordar el buffer

// crea un texto en la pantalla, y borra el resto de la línea con el color de fondo
static void texto_linea(uint16_t x, uint16_t y, const char *s, const GFXfont *fuente,
                        uint16_t color, uint16_t fondo)
{
    uint16_t fin = tft_texto(x, y, s, fuente, color, fondo);                 // bit final del texto dibujado, se usa para borrar el resto de la línea
    if (fin < TFT_ANCHO)
        tft_rectangulo(fin, y, TFT_ANCHO - fin, fuente->yAdvance, fondo);    // borra el resto de la línea
}

// dibuja una línea de texto centrada en la pantalla, y borra el resto de la línea con el color de fondo
static void linea_centrada(uint16_t y, const char *s, const GFXfont *fuente,
                           uint16_t color, uint16_t fondo)
{
    uint16_t ancho = tft_ancho_texto(s, fuente);                             // coje la fuente y calcula el ancho del texto en bits
    uint16_t x = (ancho < TFT_ANCHO) ? (TFT_ANCHO - ancho) / 2 : 0;          // si el texto es más ancho que la pantalla, se dibuja desde el borde izquierdo
    if (x > 0) tft_rectangulo(0, y, x, fuente->yAdvance, fondo);             // borra el espacio a la izquierda del texto
    texto_linea(x, y, s, fuente, color, fondo);                              // dibuja el texto
}

// divide un nombre largo en varias líneas y las dibuja en la pantalla
static void nombre_en_lineas(const char *nombre)
{
    const GFXfont *f = &FreeSansBold12pt7b;  // 12 pt es el tamaño de fuente que se usa para dibujar el nombre de la canción
    char linea[MAX_TEXTO];                   // almacena la línea actual que se está dibujando
    char prueba[MAX_TEXTO];                  // almacena la línea que se está probando antes de decidir si se dibuja
    uint8_t largo = 0;                       // longitud de la línea actual
    uint8_t num_linea = 0;                   // número de la línea actual
    const char *p = nombre;                  // puntero al nombre de la canción

    while (*p && num_linea < MAX_LINEAS)     // mientras haya caracteres en el nombre y no se hayan dibujado todas las líneas
    {
        while (*p == ' ') p++;               // salta los espacios en blanco
        if (*p == '\0') break;               // si se llega al final del string, se detiene

        // construye la línea de prueba que se va a dibujar, añadiendo palabras hasta que se alcance el ancho máximo de la pantalla
        uint8_t i = 0;
        if (largo > 0)
        {
            for (i = 0; i < largo; i++) prueba[i] = linea[i];   // se copia la línea actual en la línea de prueba para añadirle la siguiente palabra
            prueba[i++] = ' ';                                  // se añade un espacio entre la línea actual y la siguiente palabra
        }
        const char *q = p;
        while (*q && *q != ' ' && i < MAX_TEXTO - 1) prueba[i++] = *q++;   // se añade la siguiente palabra a la línea de prueba
        prueba[i] = '\0';                                                  // el caracter nulo se agrega para saber donde termina la línea de prueba

        if (largo == 0 || tft_ancho_texto(prueba, f) <= ANCHO_UTIL)   // 0 si es la primera palabra de la línea, o si la línea de prueba cabe en el ancho máximo de la pantalla
        {
            for (uint8_t k = 0; k <= i; k++) linea[k] = prueba[k];    // se copia la línea de prueba en la línea actual
            largo = i;
            p = q;                                                    // se avanza el puntero al final de la palabra añadida para la siquiente iteración de prueba
        }
        else
        {
            linea_centrada(Y_NOMBRE + num_linea * f->yAdvance, linea, f, TFT_AMARILLO, TFT_NEGRO);  // si la línea de prueba no cabe, se deja asi la linea actual
            num_linea++;
            largo = 0;                                                // la p se deja igual ya que no se movio en la linea actual 
        }
    }

    if (largo > 0 && num_linea < MAX_LINEAS)                          // si hay texto pendiente y aún hay espacio para dibujarlo
    {
        linea_centrada(Y_NOMBRE + num_linea * f->yAdvance, linea, f, TFT_AMARILLO, TFT_NEGRO);
        num_linea++;
    }

    for (; num_linea < MAX_LINEAS; num_linea++)                       // esta para dibujar las filas vacías de la lista
        tft_rectangulo(0, Y_NOMBRE + num_linea * f->yAdvance, TFT_ANCHO, f->yAdvance, TFT_NEGRO);
}

// dibuja la pantalla de inicio
void pantalla_inicio(void)
{
    tft_llenar(TFT_NEGRO);                                                              // se llena la pantalla de negro
    tft_rectangulo(0, 0, TFT_ANCHO, BARRA_ALTO, TFT_AZUL_OSCURO);                       // se dibuja la barra superior
    tft_rectangulo(0, TFT_ALTO - BARRA_ALTO, TFT_ANCHO, BARRA_ALTO, TFT_AZUL_OSCURO);   // se dibuja la barra inferior
    tft_texto_centrado(TFT_ALTO - BARRA_ALTO + 4, "Prev  Pausa  Stop  Next",
                       &FreeSansBold9pt7b, TFT_GRIS, TFT_AZUL_OSCURO);                  // se dibuja el texto en el centro de la barra inferior
    tft_texto_centrado(45, "EPIC", &FreeSansBold18pt7b, TFT_NARANJA, TFT_NEGRO);        // se dibuja el nombre de la aplicación en el centro de la pantalla
}

// dibuja un mensaje de estado en la pantalla
void pantalla_estado(const char *texto)
{
    texto_linea(8, 4, texto, &FreeSansBold9pt7b, TFT_BLANCO, TFT_AZUL_OSCURO);  // se dibuja el texto en la posición especificada
}

// dibuja la información de la canción actual
void pantalla_cancion(uint8_t numero, const char *nombre)
{
    nombre_en_lineas(nombre);                // se divide el nombre en líneas que caben en la pantalla

    char contador[] = "00 / 40";             // cadena para mostrar el número de la canción
    contador[0] = '0' + (numero / 10);       // 0 porque es el dígito de las decenas
    contador[1] = '0' + (numero % 10);       // 0 porque es el dígito de las unidades
    texto_linea(X_CONTADOR, Y_CONTADOR, contador, &FreeSansBold12pt7b, TFT_BLANCO, TFT_NEGRO);  // se dibuja el contador en la posición especificada
}

#define FILAS_VISIBLES    8     // filas de la lista que caben entre las dos barras
#define FILA_ALTO        32     // alto de cada fila: (290 - 30) / 8 = 32 px
#define Y_LISTA          BARRA_ALTO   // la lista empieza justo debajo de la barra superior
#define X_NUMERO_MENU     6     // columna del número de canción en cada fila
#define MARGEN_DER        6     // espacio libre a la derecha de cada fila
#define TOTAL_CANCIONES  40

static uint8_t primera = 1;     // número de la canción que está en la fila de ARRIBA (fila 0)

// Escribe el texto; si no cabe antes de x_max, lo corta y termina con "..."
// Ej: "Would You Fall in Love with Me Again" -> "Would You Fall in L..."
static void texto_recortado(uint16_t x, uint16_t y, const char *s, const GFXfont *fuente,
                            uint16_t color, uint16_t fondo, uint16_t x_max)
{
    if (x + tft_ancho_texto(s, fuente) <= x_max)     // ¿cabe completo? -> escribirlo y listo
    {
        tft_texto(x, y, s, fuente, color, fondo);
        return;
    }
    uint16_t puntos = tft_ancho_texto("...", fuente);   // lo que ocupan los "..."
    char letra[2] = {0, '\0'};                          // texto de UNA letra, para medir su ancho
    while (*s)
    {
        letra[0] = *s;
        uint16_t ancho = tft_ancho_texto(letra, fuente);
        if (x + ancho + puntos > x_max) break;
        x += tft_letra(x, y, *s, fuente, color, fondo);  // dibujar la letra y avanzar
        s++;
    }
    tft_texto(x, y, "...", fuente, color, fondo);
}

// Dibuja UNA fila de la lista: número + nombre, resaltada si está marcada
static void fila_menu(char nombres[][40], uint8_t numero, uint8_t marcada)
{
    uint16_t y = Y_LISTA + (numero - primera) * FILA_ALTO;

    uint16_t fondo = marcada ? TFT_AZUL : TFT_NEGRO;
    uint16_t color = marcada ? TFT_BLANCO : TFT_GRIS;  // la marcada se ve más brillante

    tft_rectangulo(0, y, TFT_ANCHO, FILA_ALTO, fondo);

    char num[] = "00 ";                              // número de canción con un espacio
    num[0] = '0' + (numero / 10);
    num[1] = '0' + (numero % 10);
    uint16_t x = tft_texto(X_NUMERO_MENU, y + 5, num, &FreeSansBold9pt7b, TFT_AMARILLO, fondo);
    // (y + 5: el texto de 22 px queda centrado en la fila de 32)
    // tft_texto devuelve dónde terminó el número: ahí empieza el nombre

    texto_recortado(x, y + 5, nombres[numero - 1], &FreeSansBold9pt7b, color, fondo,
                    TFT_ANCHO - MARGEN_DER);
}

// Ajusta "primera" para que la selección quede visible. Devuelve 1 si la lista se desplazó
static uint8_t ajustar_ventana(uint8_t seleccion)
{
    uint8_t antes = primera;
    if (seleccion < primera)
        primera = seleccion;
    else if (seleccion >= primera + FILAS_VISIBLES)
        primera = seleccion - FILAS_VISIBLES + 1;
    // (la selección quedó ABAJO: la lista baja hasta que la selección quede en la última fila)
    return primera != antes;
}

// Dibuja las 8 filas visibles
static void lista_completa(char nombres[][40], uint8_t seleccion)
{
    for (uint8_t i = 0; i < FILAS_VISIBLES; i++)
        fila_menu(nombres, primera + i, (primera + i) == seleccion);
    // (el tercer parámetro vale 1 solo para la fila de la canción seleccionada)
}

// Dibuja el menú completo (barras + lista). Se llama al abrir el menú
void pantalla_menu_abrir(char nombres[][40], uint8_t seleccion)
{
    tft_rectangulo(0, 0, TFT_ANCHO, BARRA_ALTO, TFT_AZUL_OSCURO);
    tft_texto(8, 4, "Elige una cancion", &FreeSansBold9pt7b, TFT_BLANCO, TFT_AZUL_OSCURO);

    tft_rectangulo(0, TFT_ALTO - BARRA_ALTO, TFT_ANCHO, BARRA_ALTO, TFT_AZUL_OSCURO);
    tft_texto_centrado(TFT_ALTO - BARRA_ALTO + 4, "Pausa: elegir  Menu: salir",
                       &FreeSansBold9pt7b, TFT_GRIS, TFT_AZUL_OSCURO);

    // Al abrir, la canción actual queda en la fila de arriba...
    primera = seleccion;
    // ...salvo al final de la lista: con la 38 arriba solo habría 3 filas llenas
    if (primera > TOTAL_CANCIONES - FILAS_VISIBLES + 1)
        primera = TOTAL_CANCIONES - FILAS_VISIBLES + 1;
    lista_completa(nombres, seleccion);
}

// Mueve la marca de "anterior" a "seleccion".
// Si la lista no se desplazó, basta con redibujar 2 filas (rápido);
// si se desplazó, todas las filas cambian de canción y se redibuja la lista entera.
void pantalla_menu_mover(char nombres[][40], uint8_t anterior, uint8_t seleccion)
{
    if (ajustar_ventana(seleccion))
    {
        lista_completa(nombres, seleccion);
    }
    else
    {
        fila_menu(nombres, anterior, 0);
        fila_menu(nombres, seleccion, 1);
    }
}