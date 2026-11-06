#ifndef __UART_H__
#define __UART_H__
#include <stdint.h>

void uart_init(uint32_t baudios); // Inicializa la UART con la velocidad de baudios especificada 
void uart_enviar(uint8_t dato);// Envía un byte a través de la UART
void uart_enviar_texto(const char *str);// Envía una cadena de texto a través de la UART
void uart_hexadecimal(uint32_t valor);// Envía un valor hexadecimal a través de la UART
void uart_numero(uint32_t valor);// Envía un número decimal a través de la UART
#endif