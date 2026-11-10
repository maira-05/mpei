#ifndef AUDIO_H
#define AUDIO_H
#include <stdint.h>

#define AUDIO_TIMER_HZ  84000000U   /* reloj de TIM2/TIM3 (APB1 x2), tal vez este es el que toque cambiar (probalo más tarde) */

void audio_pwm_init(void);                    /* TIM2 CH1 en PA0: PWM de 8 bits */
void audio_muestreo_init(uint32_t frecuencia);/* TIM3: interrupcion a 'frecuencia' Hz */
void audio_poner_muestra(uint8_t valor);      /* cambia el duty (0..255) */

#endif