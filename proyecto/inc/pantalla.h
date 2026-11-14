// pantalla.h encarga de dibujar en la pantalla, se llama desde el main y desde la interrupción del timer
#ifndef __PANTALLA_H__
#define __PANTALLA_H__
#include <stdint.h>

void pantalla_inicio(void);                                  // dibuja la pantalla de inicio, se llama al arrancar el programa
void pantalla_cancion(uint8_t numero, const char *nombre);   // dibuja el número y nombre de la canción actual, se llama cuando cambia la canción
void pantalla_estado(const char *texto);                     // dibuja un texto de estado en la pantalla, se llama cuando hay que mostrar un mensaje de estado

void pantalla_menu_abrir(char nombres[][40], uint8_t seleccion);                    // abre el menú y dibuja la lista de canciones  
void pantalla_menu_mover(char nombres[][40], uint8_t vieja, uint8_t nueva);         // mueve la selección en el menú, se llama cuando se pulsa NEXT o PREV

#endif