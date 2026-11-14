#include "audio.h"
#include "gpio.h"
#include "stm32f4xx.h"

void audio_pwm_init(void)
{
    // El reloj de TIM2/TIM3 es 84 MHz (APB1 x2). La frecuencia del PWM es 84 MHz / (PSC+1) / (ARR+1). Para un PWM de 8 bits, ARR=255.
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    GPIO_InitTypeDef pin = {GPIO_MODO_AF, 0, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 1}; //Se configura PA0 como función alterna (AF1) para TIM2_CH1, sin pull-up/pull-down, alta velocidad
    GPIO_Config(GPIOA, pin);

    //Frecuencia del PWM: 84 MHz / (PSC+1) / (ARR+1)
    TIM2->PSC = 0;          // sin preescalador
    TIM2->ARR = 255;        // 256 niveles (0-255)
    TIM2->CCR1 = 128;       // empezar en la mitad (50% duty) promedio de 1,65 V = silencio (el parlante en reposo)

    //Canal 1 en modo PWM 1, con precarga del CCR1
    // PWM modo 1 (OC1M = 110): la salida esta en 1 mientras CNT < CCR1, y en 0 despues.
    // Asi el duty = CCR1 / 256: CCR1 = 0 -> siempre 0 V; CCR1 = 255 -> casi siempre 3,3 V.
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM2->CCMR1 |= (6U << TIM_CCMR1_OC1M_Pos);   // PWM modo 1 = 110 en binario
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;

    //Habilitar la salida del canal 1
    TIM2->CCER |= TIM_CCER_CC1E;

    //Precarga del ARR, cargar los valores y arrancar
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR = TIM_EGR_UG; // copia PSC/ARR/CCR1 a sus registros internos
    TIM2->CR1 |= TIM_CR1_CEN; // encender el contador
}

void audio_muestreo_init(uint32_t frecuencia)
{
    // Se habilita el TIM3 para generar interrupciones a la frecuencia de muestreo deseada.
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    //Frecuencia de la interrupción: 84 MHz / (ARR+1) = frecuencia
    TIM3->PSC = 0;
    TIM3->ARR = (AUDIO_TIMER_HZ / frecuencia) - 1;

    //Habilitar la interrupción por "update" (cuando el contador da la vuelta)
    TIM3->DIER |= TIM_DIER_UIE;
    TIM3->EGR = TIM_EGR_UG; //carga PSC/ARR a los registros internos("update")
    TIM3->SR = 0; // limpiar banderas pendientes

    //Habilitar la interrupción en el NVIC y arrancar
    NVIC_EnableIRQ(TIM3_IRQn);// permite la interrupcion en el nucleo; la funcion que se ejecuta es TIM3_IRQHandler (main.c), por su nombre en la tabla de vectores de crt0.S
    TIM3->CR1 |= TIM_CR1_CEN;
}

void audio_poner_muestra(uint8_t valor)
{
    TIM2->CCR1 = valor;  // cambiar el duty cycle del PWM (0..255)
}