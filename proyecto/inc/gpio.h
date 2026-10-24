#ifndef __GPIO_H__
#define __GPIO_H__
#include "stm32f4xx.h"
#include <stdint.h>
//valores de mode
#define GPIO_MODO_ENTRADA 0U
#define GPIO_MODO_SALIDA 1U
#define GPIO_MODO_AF 2U
#define GPIO_MODO_ANALOGICO 3U
//valores de pull
#define GPIO_SIN_PULL 0U
#define GPIO_PULL_UP 1U
#define GPIO_PULL_DOWN 2U
//valores de speed
#define GPIO_BAJA_VELOCIDAD 0U
#define GPIO_MEDIA_VELOCIDAD 1U
#define GPIO_ALTA_VELOCIDAD 2U
#define GPIO_MUY_ALTA_VELOCIDAD 3U

// Estructura de configuración de un pin 
typedef struct {
    uint32_t Mode;
    uint32_t Pin;
    uint32_t Pull;
    uint32_t Speed;
    uint32_t Alternate;

} GPIO_InitTypeDef;

void GPIO_clock_habilitar(volatile GPIO_TypeDef *GPIOx); //habilito el reloj del puerto GPIOx

void GPIO_Config(volatile GPIO_TypeDef *GPIOx, GPIO_InitTypeDef Config); //configuro el pin del puerto GPIOx con la estructura de configuración

uint32_t leer_pin_state(volatile GPIO_TypeDef *GPIOx, uint32_t pin); // lee el estado del pin del puerto GPIOx
void escribir_pin_state(volatile GPIO_TypeDef *GPIOx, uint32_t pin, uint32_t state); // escribe el estado del pin del puerto GPIOx
void toggle_pin_state(volatile GPIO_TypeDef *GPIOx, uint32_t pin); // alterna el estado del pin del puerto GPIOx

#endif