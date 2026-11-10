#include "audio.h"
#include "gpio.h"
#include "stm32f4xx.h"

void audio_pwm_init(void)
{
    // 1. Reloj del TIM2 (¿en qué bus está? ¿APB1 o APB2?)
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // 2. PA0 en función alterna: ¿qué número de AF es TIM2_CH1 en PA0?
    GPIO_InitTypeDef pin = {GPIO_MODO_AF, 0, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 1};
    GPIO_Config(GPIOA, pin);

    // 3. Frecuencia del PWM: 84 MHz / (PSC+1) / (ARR+1)
    TIM2->PSC = 0;          // sin preescalador
    TIM2->ARR = 255;        // 256 niveles (0..255) -> ¿qué valor va aquí?
    TIM2->CCR1 = 128;       // empezar en la mitad (silencio)

    // 4. Canal 1 en modo PWM 1, con precarga del CCR1
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM2->CCMR1 |= (6U << TIM_CCMR1_OC1M_Pos);   // PWM modo 1 = 110 en binario
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;

    // 5. Habilitar la salida del canal 1
    TIM2->CCER |= TIM_CCER_CC1E;

    // 6. Precarga del ARR, cargar los valores y arrancar
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR = TIM_EGR_UG;      // copia PSC/ARR/CCR1 a sus registros internos
    TIM2->CR1 |= TIM_CR1_CEN;            // encender el contador
}

void audio_muestreo_init(uint32_t frecuencia)
{
    // 1. Reloj del TIM3
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // 2. Frecuencia de la interrupción: 84 MHz / (ARR+1) = frecuencia
    TIM3->PSC = 0;
    TIM3->ARR = (AUDIO_TIMER_HZ / frecuencia) - 1;             // despejen ARR de la fórmula

    // 3. Habilitar la interrupción por "update" (cuando el contador da la vuelta)
    TIM3->DIER |= TIM_DIER_UIE;
    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR = 0;                // limpiar banderas pendientes

    // 4. Habilitar la interrupción en el NVIC y arrancar
    NVIC_EnableIRQ(TIM3_IRQn);         // el nombre de la interrupción del TIM3
    TIM3->CR1 |= TIM_CR1_CEN;
}

void audio_poner_muestra(uint8_t valor)
{
    TIM2->CCR1 = valor;  
}