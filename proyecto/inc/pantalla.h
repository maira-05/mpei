#ifndef __PANTALLA_H__
#define __PANTALLA_H__
#include <stdint.h>

/* Capa de interfaz: decide QUÉ se muestra en la pantalla.
 * main.c solo llama estas funciones; las coordenadas, colores y fuentes
 * quedan escondidos en pantalla.c. */

void pantalla_inicio(void);                                  /* fondo + barras fijas (al encender) */

void pantalla_cancion(uint8_t numero, const char *nombre);   /* nombre en líneas + "xx / 40" */

void pantalla_estado(const char *texto);                     /* texto de la barra superior */

#endif