// funciones que maneja la pantalla, se llaman desde el main 
#include "pantalla.h"
#include "tft.h"
#include "tft_text.h"
#include "Fonts/FreeSansBold9pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "Fonts/FreeSansBold18pt7b.h"

// definiciones de constantes para dibujar en la pantalla

#define BARRA_ALTO       30     // pixeles hasta donde va la barra de estado
#define Y_NOMBRE         90     // pixeles desde donde empieza a dibujar el nombre de la canción
#define MAX_LINEAS        3     // número máximo de líneas para mostrar el nombre
#define ANCHO_UTIL      224     // 224 para dejar un margen de 8 pixeles a cada lado, para que no se corte el texto
#define Y_CONTADOR      200     // pixeles desde donde empieza a dibujar el contador en y
#define X_CONTADOR       83     // pixeles desde donde empieza a dibujar el contador en x
#define MAX_TEXTO        40     // maximo de caracteres que puede tener el nombre de la canción contando el carácter nulo, para no desbordar el buffer

// definiciones usadas en el menu de selección de canciones
#define FILAS_VISIBLES    8     // cuantas canciones son visibles al mismo tiempo en el menu
#define FILA_ALTO        32     // pixeles de alto de cada fila del menu
#define Y_LISTA          BARRA_ALTO   // pixeles desde donde empieza a dibujar la lista de canciones
#define X_NUMERO_MENU     6     // pixeles desde donde empieza a dibujar el número de la canción en el menu
#define MARGEN_DER        6     // pixeles maximos donde debe dibujar el nombre de la canción en el menu, respecto al borde derecho de la pantalla
#define TOTAL_CANCIONES  40     // limite para saber cuando esta cerca de terminar la lista de canciones y ajustar la primera visible para que se vean las últimas canciones

// crea un texto en la pantalla, y borra el resto de la línea con el color de fondo
static void texto_linea(uint16_t x, uint16_t y, const char *s, const GFXfont *fuente,
                        uint16_t color, uint16_t fondo)
{
    uint16_t fin = tft_texto(x, y, s, fuente, color, fondo);                 // pixeles finales del texto dibujado, se usa para borrar el resto de la línea
    if (fin < TFT_ANCHO)
        tft_rectangulo(fin, y, TFT_ANCHO - fin, fuente->yAdvance, fondo);    // borra el resto de la línea
}

// dibuja una línea de texto centrada en la pantalla, y borra el resto de la línea con el color de fondo
static void linea_centrada(uint16_t y, const char *s, const GFXfont *fuente,
                           uint16_t color, uint16_t fondo)
{
    uint16_t ancho = tft_ancho_texto(s, fuente);                             // coge la fuente y calcula el ancho del texto en pixeles
    uint16_t x = (ancho < TFT_ANCHO) ? (TFT_ANCHO - ancho) / 2 : 0;          // si el texto es más ancho que la pantalla, se dibuja desde el borde izquierdo
    if (x > 0) tft_rectangulo(0, y, x, fuente->yAdvance, fondo);             // borra el espacio a la izquierda del texto
    texto_linea(x, y, s, fuente, color, fondo);                              // dibuja el texto
}

// divide un nombre largo en varias líneas y las dibuja en la pantalla
static void nombre_en_lineas(const char *nombre)
{
    const GFXfont *f = &FreeSansBold12pt7b;  // 12 pt es el tamaño de fuente que se usa para dibujar el nombre de la canción 18 haria que el nombre ocupara 4 lineas
    char linea[MAX_TEXTO];                   // almacena la línea actual que se está construyendo antes de decidir si se dibuja 
    char prueba[MAX_TEXTO];                  // almacena la línea que se está probando antes de decidir si se dibuja
    uint8_t largo = 0;                       // longitud de la línea actual
    uint8_t num_linea = 0;                   // número de la línea actual
    const char *p = nombre;                  // puntero al nombre de la canción

    while (*p && num_linea < MAX_LINEAS)     // mientras haya caracteres en el nombre y no se hayan dibujado todas las líneas
    {
        while (*p == ' ') p++;               // salta los espacios en blanco
        if (*p == '\0') break;               // si se llega al final del string, se detiene

        /*construye la línea de prueba que se va a dibujar, 
        añadiendo palabras o espacios con cada pasada hasta que se llegue al final del nombre o se llene la línea de prueba*/     
        uint8_t i = 0;
        if (largo > 0)
        {
            for (i = 0; i < largo; i++) prueba[i] = linea[i];   // se copia la línea actual en la línea de prueba para añadirle la siguiente palabra
            prueba[i++] = ' ';                                  // se añade un espacio entre la línea actual y la siguiente palabra
        }
        const char *q = p;
        while (*q && *q != ' ' && i < MAX_TEXTO - 1) prueba[i++] = *q++;   // se añade la siguiente palabra a la línea de prueba
        prueba[i] = '\0';                                                  // el caracter nulo se agrega para saber donde termina la línea de prueba

        if (largo == 0 || tft_ancho_texto(prueba, f) <= ANCHO_UTIL)   /*0 si es la primera palabra de la línea, o si la línea de prueba cabe en el ancho 
        máximo de la pantalla, ¨largo = 0 ¨ porque si la linea esta vacia y no cabe, en las demas no cabra y el programa se quedaria en esto */ 
        {
            for (uint8_t k = 0; k <= i; k++) linea[k] = prueba[k];    // se copia la línea de prueba en la línea actual
            largo = i;
            p = q;                                                    // se avanza el puntero al final de la palabra añadida para la siquiente iteración de prueba
        }
        else
        {
            linea_centrada(Y_NOMBRE + num_linea * f->yAdvance, linea, f, TFT_AMARILLO, TFT_NEGRO);  // si la línea de prueba no cabe, se deja asi la linea se dibuja sin la palabra que no cabe
            num_linea++;
            largo = 0;                                                // la p no avanza si la palabra que no cupo, asi que pasa a la siguiente linea
        }
    }

    if (largo > 0 && num_linea < MAX_LINEAS)                          // si hay texto pendiente y aún hay espacio para dibujarlo
    {
        linea_centrada(Y_NOMBRE + num_linea * f->yAdvance, linea, f, TFT_AMARILLO, TFT_NEGRO);
        num_linea++;
    }

    for (; num_linea < MAX_LINEAS; num_linea++)                       // borra las filas sobrantes del nombre anterior en pantalla
        tft_rectangulo(0, Y_NOMBRE + num_linea * f->yAdvance, TFT_ANCHO, f->yAdvance, TFT_NEGRO);
}

// dibuja la pantalla de inicio es decir (Fondo negro, barra superior e inferior, EPIC)
void pantalla_inicio(void)
{
    tft_llenar(TFT_NEGRO);                                                              // se llena la pantalla de negro
    tft_rectangulo(0, 0, TFT_ANCHO, BARRA_ALTO, TFT_AZUL_OSCURO);                       // se dibuja la barra superior
    tft_rectangulo(0, TFT_ALTO - BARRA_ALTO, TFT_ANCHO, BARRA_ALTO, TFT_AZUL_OSCURO);   // se dibuja la barra inferior
    tft_texto_centrado(TFT_ALTO - BARRA_ALTO + 4, "Prev  Pausa  Stop  Next",
                       &FreeSansBold9pt7b, TFT_GRIS, TFT_AZUL_OSCURO);                  // se dibuja el texto en el centro de la barra inferior
    tft_texto_centrado(45, "EPIC", &FreeSansBold18pt7b, TFT_NARANJA, TFT_NEGRO);        // se dibuja el nombre del musical en y = 45 (altura fijada en toda la reproduccion) 
}

// dibuja el estado en la barra superior, "Pausa", "Reproduciendo" y "Stop" 
void pantalla_estado(const char *texto)
{
    texto_linea(8, 4, texto, &FreeSansBold9pt7b, TFT_BLANCO, TFT_AZUL_OSCURO);  // borra las lineas sobrantes del estado anterior 
}

// dibuja la información de la canción actual
void pantalla_cancion(uint8_t numero, const char *nombre)
{
    nombre_en_lineas(nombre);                // se divide el nombre en líneas que caben en la pantalla

    char contador[] = "00 / 40";             // cadena para mostrar el número de la canción
    contador[0] = '0' + (numero / 10);       // convierte el dígito de las decenas en un caracter '1' '2'...
    contador[1] = '0' + (numero % 10);       // convierte el dígito de las unidades en un caracter '1' '2'...
    texto_linea(X_CONTADOR, Y_CONTADOR, contador, &FreeSansBold12pt7b, TFT_BLANCO, TFT_NEGRO);  // se dibuja el contador en la posición dada por los pixeles de lo anterior
}

// Menu del reproductor

static uint8_t primera = 1;     /* guarda el número de la primera canción visible en la lista, 
se usa para calcular en qué posición está la canción actual */
// dibuja un texto recortado si excede el ancho máximo
static void texto_recortado(uint16_t x, uint16_t y, const char *s, const GFXfont *fuente,
                            uint16_t color, uint16_t fondo, uint16_t x_max)
{
    if (x + tft_ancho_texto(s, fuente) <= x_max)     // si el texto cabe en el espacio disponible
    {
        tft_texto(x, y, s, fuente, color, fondo);
        return;                                       
    }
    uint16_t puntos = tft_ancho_texto("...", fuente);   // obtiene el ancho del texto "..."
    char letra[2] = {0, '\0'};                          // arreglo para mirar la medida de cada letra junto a su caracter nulo
    while (*s)
    {
        letra[0] = *s;
        uint16_t ancho = tft_ancho_texto(letra, fuente);    // obtiene el ancho del carácter actual
        if (x + ancho + puntos > x_max) break;               // termina si el texto actual más los puntos suspensivos excede el ancho máximo
        x += tft_letra(x, y, *s, fuente, color, fondo);      // dibuja el carácter actual
        s++;
    }
    tft_texto(x, y, "...", fuente, color, fondo);            // dibuja los puntos suspensivos si no cabe el texto completo
}

// dibuja una fila del menu, con el número y nombre de la canción, si es marcada = 1 se dibuja con fondo azul
static void fila_menu(char nombres[][40], uint8_t numero, uint8_t marcada)
{
    uint16_t y = Y_LISTA + (numero - primera) * FILA_ALTO;   // calcula la posición vertical de la fila

    uint16_t fondo = marcada ? TFT_AZUL : TFT_NEGRO;         // el fondo es azul si la fila está marcada, negro si no
    uint16_t color = marcada ? TFT_BLANCO : TFT_GRIS;        // el color del texto es blanco si la fila está marcada, gris si no

    tft_rectangulo(0, y, TFT_ANCHO, FILA_ALTO, fondo);       // pinta la fila completa de su color correspondiente

    char num[] = "00 ";                                      // arreglo para guardar numero de la cancion
    num[0] = '0' + (numero / 10);                            // guarda el dígito de las decenas
    num[1] = '0' + (numero % 10);                            // guarda el dígito de las unidades
    uint16_t x = tft_texto(X_NUMERO_MENU, y + 5, num, &FreeSansBold9pt7b, TFT_AMARILLO, fondo);
    // dibuja el #cancion centra el texto en el recuadro de cada cancion y guarda la pocision para poner luego el texto 

    texto_recortado(x, y + 5, nombres[numero - 1], &FreeSansBold9pt7b, color, fondo,
                    TFT_ANCHO - MARGEN_DER);
    // ¨-1¨ porque el arreglo empieza en 0, llega hasta TFT_ANCHO - MARGEN_DER, si no cabe corta con "..." 
}

// ajusta la ventana de visualización para que la selección esté dentro
static uint8_t ajustar_ventana(uint8_t seleccion)
{
    uint8_t antes = primera;                              // guarda el valor anterior de primera
    if (seleccion < primera)                              // si la selección está por arriba
        primera = seleccion;                              // actualiza primera
    else if (seleccion >= primera + FILAS_VISIBLES)       // si la selección está por abajo
        primera = seleccion - FILAS_VISIBLES + 1;         // actualiza primera
    return primera != antes;                              // devuelve 1 si primera cambió, 0 si no
}

// dibuja todas las filas visibles del menú
static void lista_completa(char nombres[][40], uint8_t seleccion)
{
    for (uint8_t i = 0; i < FILAS_VISIBLES; i++)
        fila_menu(nombres, primera + i, (primera + i) == seleccion);
    // dibuja la fila correspondiente a cada canción y marca la selección
}

// abre el menú y dibuja la lista de canciones
void pantalla_menu_abrir(char nombres[][40], uint8_t seleccion)
{
    tft_rectangulo(0, 0, TFT_ANCHO, BARRA_ALTO, TFT_AZUL_OSCURO);                          // dibuja la barra superior del menú
    tft_texto(8, 4, "Elige una cancion", &FreeSansBold9pt7b, TFT_BLANCO, TFT_AZUL_OSCURO);  // titulo del menu, sin tilde (no reconocia el caracter)

    tft_rectangulo(0, TFT_ALTO - BARRA_ALTO, TFT_ANCHO, BARRA_ALTO, TFT_AZUL_OSCURO);      // dibuja la barra inferior del menú
    tft_texto_centrado(TFT_ALTO - BARRA_ALTO + 4, "Pausa: elegir  Menu: salir",
                       &FreeSansBold9pt7b, TFT_GRIS, TFT_AZUL_OSCURO);                     // funciones de los botones dentro del menu

    primera = seleccion;                                         // decide la primera de la lista como la seleccionada, para que se vea en pantalla
    if (primera > TOTAL_CANCIONES - FILAS_VISIBLES + 1)          // si la seleccionada está cerca del final de la lista, ajusta primera para que se vean las últimas canciones
        primera = TOTAL_CANCIONES - FILAS_VISIBLES + 1;          // la primera visible de la lista siempre va a ser la misma para las ultimas canciones
    lista_completa(nombres, seleccion);                          // dibuja la lista de canciones
}

// redibuja la selección en el menu, moviendo la marca de la canción anterior a la nueva
void pantalla_menu_mover(char nombres[][40], uint8_t anterior, uint8_t seleccion)
{
    if (ajustar_ventana(seleccion))          // si la seleccion esta fuera de la ventana visible redibuja todo
    {
        lista_completa(nombres, seleccion);  // redibuja toda la lista
    }
    else
    {
        fila_menu(nombres, anterior, 0);     // desmarca la fila anterior
        fila_menu(nombres, seleccion, 1);    // marca la fila nueva
    }
}