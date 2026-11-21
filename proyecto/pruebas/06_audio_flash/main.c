/* =====================================================================
 * PRUEBA DE AUDIO: reproducir un fragmento de cancion guardado en FLASH
 * NUCLEO-F401RE - registros directos con CMSIS, sin librerias.
 *
 * OJO: solo es una prueba del "DAC por PWM". No lee la microSD; el
 * fragmento se convierte con wav_a_c.py y queda dentro del programa.
 * En el proyecto, las muestras vendran de la SD.
 *
 * Reloj: HSI de fabrica = 16 MHz.
 *
 * Como funciona:
 *   TIM2 canal 1 (PA0): PWM con ARR = 255 -> portadora de
 *                       16 MHz / 256 = 62.5 kHz, resolucion de 8 bits.
 *   TIM3: interrumpe a 22050 Hz. En cada interrupcion se copia la
 *         siguiente muestra (0..255) al duty del PWM (TIM2->CCR1).
 *   El filtro RC (1 k + 10 nF, corte ~16 kHz) quita la portadora y deja
 *   el audio.
 *
 * Conexiones (las mismas de la prueba anterior):
 *   PA0 -> 1 k -> (10 nF a GND) -> potenciometro -> entrada R del GF1002
 *   GND Nucleo -> G del GF1002 ; parlante en R+ / R-
 *   El LED LD2 cambia cada vez que la cancion vuelve a empezar.
 * ===================================================================== */

#include <stdint.h>
#include "stm32f4xx.h"

#define CANCION_FS   22050U             /* frecuencia de muestreo (Hz) */
#define F_TIMER      16000000U          /* reloj de TIM2/TIM3 con HSI  */

/* Muestras generadas por wav_a_c.py (src/cancion.c), en FLASH (.rodata) */
extern const uint8_t  cancion[];
extern const uint32_t cancion_len;

static volatile uint32_t indice = 0;    /* muestra actual */

/* Interrupcion de TIM3: una vez por muestra */
void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF) {
        TIM3->SR = ~TIM_SR_UIF;               /* limpiar la bandera */
        TIM2->CCR1 = cancion[indice];         /* nueva muestra -> duty */
        indice++;
        if (indice >= cancion_len) {
            indice = 0;                       /* repetir */
            GPIOA->ODR ^= (1U << 5);          /* LED: vuelta completa */
        }
    }
}

int main(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN;

    /* PA5 = LED (salida) */
    GPIOA->MODER &= ~(3U << (5 * 2));
    GPIOA->MODER |=  (1U << (5 * 2));

    /* PA0 = TIM2_CH1 (funcion alterna AF1) */
    GPIOA->MODER   &= ~(3U << (0 * 2));
    GPIOA->MODER   |=  (2U << (0 * 2));
    GPIOA->OSPEEDR |=  (3U << (0 * 2));
    GPIOA->AFR[0]  &= ~(0xFU << 0);
    GPIOA->AFR[0]  |=  (1U   << 0);

    /* TIM2: PWM de 8 bits a 62.5 kHz */
    TIM2->PSC   = 0;
    TIM2->ARR   = 255;
    TIM2->CCR1  = 128;                         /* silencio = mitad */
    TIM2->CCMR1 = (6U << TIM_CCMR1_OC1M_Pos)   /* PWM modo 1 */
                | TIM_CCMR1_OC1PE;             /* precarga del duty */
    TIM2->CCER  = TIM_CCER_CC1E;
    TIM2->CR1   = TIM_CR1_ARPE;
    TIM2->EGR   = TIM_EGR_UG;
    TIM2->CR1  |= TIM_CR1_CEN;

    /* TIM3: interrupcion a la frecuencia de muestreo */
    TIM3->PSC  = 0;
    TIM3->ARR  = (F_TIMER / CANCION_FS) - 1;   /* 16e6/22050 - 1 = 724 */
    TIM3->DIER = TIM_DIER_UIE;
    TIM3->EGR  = TIM_EGR_UG;
    TIM3->SR   = 0;
    NVIC_EnableIRQ(TIM3_IRQn);
    TIM3->CR1  = TIM_CR1_CEN;

    while (1) {
        /* Todo el trabajo lo hace la interrupcion */
    }
}
