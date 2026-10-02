/*
 * Laboratory Exercise 5 - Lab Exercise 2 (LAB session 1)
 * FRECUENCIMETRO bare-metal (NUCLEO-F401RE) - basado en el ejemplo 13-Timers-IC-1
 *
 *  - TIM5_CH1 (PA0) en modo Input Capture + Reset mode mide el PERIODO de la onda.
 *  - SysTick (1 ms) multiplexa los displays de 7 segmentos (5461BS-1, anodo comun).
 *  - Cada 250 ms el main calcula f = TIM_CLK / periodo_promedio y la muestra en Hz.
 *
 *  Conexiones (todo en GPIOC):
 *    PC0..PC7  -> segmentos a,b,c,d,e,f,g,dp  (con resistencia de 220-330 ohm c/u)
 *    PC8..PC12 -> digitos (PC8 = decenas de mil ... PC12 = unidades)
 *    PA0       -> senal de entrada (0 - 3.3 V)
 *    PA5       -> (opcional) PWM de prueba de 10 kHz, puentear PA5 -> PA0
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "inc/gpio_config.h"
#include "inc/system_config.h"
#include "inc/gpio_config.h"

/* ----------------------------- Configuracion ----------------------------- */
#define NUM_DIGITS        5         // La guia pide minimo 5 displays (hasta 50000 Hz)
#define SEG_FIRST_PIN     0         // PC0..PC7  = a,b,c,d,e,f,g,dp
#define DIG_FIRST_PIN     8         // PC8..PC12 = digito 0 (izquierda) ... digito 4 (derecha)

/* 5461BS-1 es de ANODO COMUN: el segmento enciende con 0 en el catodo */
#define SEG_ACTIVE_LOW    1
/* 1 = digitos manejados con transistor PNP (el digito enciende con 0 en la base)
   0 = pin del digito conectado directo al anodo comun (enciende con 1)          */
#define DIG_ACTIVE_LOW    1

#define INPUT_PIN         0         // PA0 -> TIM5_CH1 (AF2)
#define PWM_TEST_PIN      5         // PA5 -> TIM2_CH1 (AF1), senal de prueba
#define SELF_TEST         1         // 1 = genera 10 kHz en PA5 para probar sin generador

/* APB1 = 42 MHz, pero como el prescaler de APB1 es /2 (distinto de 1) el reloj
   que llega a los timers es 2 x PCLK1 = 84 MHz                                  */
#define TIM_CLK           (2UL * APB1CLK)

#define DISPLAY_UPDATE_MS 250       // Cada cuanto se recalcula la frecuencia
#define BLANK             10        // Indice de "digito apagado" en la tabla

#define SEG_MASK          (0xFFUL << SEG_FIRST_PIN)
#define DIG_MASK          (((1UL << NUM_DIGITS) - 1UL) << DIG_FIRST_PIN)

/* Tabla 7 segmentos: bit0=a, bit1=b, ... bit6=g, bit7=dp (1 = segmento encendido) */
static const uint8_t seg_table[11] = {
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F, // 9
  0x00  // apagado
};

/* ------------------------------- Variables ------------------------------- */
/* Must be delcared volatile as the timer can update asynchounosly*/
volatile SW_Timers timers;

volatile uint64_t period_sum = 0;    // Suma de los periodos capturados (en ticks del timer)
volatile uint32_t period_count = 0;  // Numero de periodos capturados en la ventana
volatile uint8_t  display_buf[NUM_DIGITS] = {BLANK, BLANK, BLANK, BLANK, 0};

uint32_t freq = 0;                   // Frecuencia medida en Hz

/* ------------------------------- Display --------------------------------- */
/* Enciende un solo digito por llamada (multiplexacion). Se llama cada 1 ms. */
void Display_Refresh(void){
  static uint32_t current = 0;
  uint32_t odr, seg, dig;

  seg = seg_table[display_buf[current]];
  dig = (1UL << current);
#if SEG_ACTIVE_LOW
  seg = (~seg) & 0xFF;
#endif
#if DIG_ACTIVE_LOW
  dig = (~dig) & ((1UL << NUM_DIGITS) - 1UL);
#endif

  odr = GPIOC->ODR & ~(SEG_MASK | DIG_MASK);
#if DIG_ACTIVE_LOW
  WRITE_REG(GPIOC->ODR, GPIOC->ODR | DIG_MASK);   // 1) apaga todos los digitos (evita "fantasmas")
#else
  WRITE_REG(GPIOC->ODR, GPIOC->ODR & ~DIG_MASK);
#endif
  odr |= (seg << SEG_FIRST_PIN) | (dig << DIG_FIRST_PIN);
  WRITE_REG(GPIOC->ODR, odr);                     // 2) pone los segmentos y enciende el digito actual

  current++;
  if(current >= NUM_DIGITS) current = 0;
}

/* Convierte el numero a digitos BCD y apaga los ceros a la izquierda */
void Display_SetNumber(uint32_t value){
  uint8_t tmp[NUM_DIGITS];
  int i;

  if(value > 99999) value = 99999;
  for(i = NUM_DIGITS - 1; i >= 0; i--){
    tmp[i] = value % 10;
    value /= 10;
  }
  for(i = 0; i < NUM_DIGITS - 1; i++){     // ceros a la izquierda -> apagados
    if(tmp[i] != 0) break;
    tmp[i] = BLANK;
  }
  for(i = 0; i < NUM_DIGITS; i++){
    display_buf[i] = tmp[i];
  }
}

/* ---------------------------- Interrupciones ----------------------------- */
/*The SysTick_Handler, was already defined as weak during the crt0.s init file, so when we define it here,
the Vector table is updated whit the new address where the function is allocated, so that when and interrupt happen
the vector table knows where to find the SysTick_Handler*/
void SysTick_Handler(void){
  update_sw_timers(&timers);
  Display_Refresh();
}

/* Se ejecuta en cada flanco de subida de la entrada: CCR1 contiene el numero de
   ticks transcurridos desde el flanco anterior (= periodo), porque el contador
   se reinicia en cada flanco (Reset mode)                                      */
void TIM5_IRQHandler(void){
  if(TIM5->SR&TIM_SR_CC1IF){
    period_sum += TIM5->CCR1;                    // Leer CCR1 tambien limpia la bandera CC1IF
    period_count++;
    WRITE_REG_FIELD(TIM5->SR, TIM_SR_CC1IF, 0); // Clear the interrupt flag
  }
}

/* -------------------------------- Timers --------------------------------- */
#if SELF_TEST
/* Senal de prueba: PWM de 10 kHz, 50% en PA5 (igual al ejemplo 13) */
void TIM2_PWM_10KHz_Init(void){
  WRITE_REG_FIELD(RCC->APB1ENR, RCC_APB1ENR_TIM2EN, 1);
  volatile unsigned int dummy;
  dummy =  RCC->APB1ENR;
  dummy =  RCC->APB1ENR;
  (void)dummy;

  WRITE_REG(TIM2->PSC, 0);                          // Sin prescaler: 84 MHz
  WRITE_REG(TIM2->ARR, (TIM_CLK/10000)-1);          // 84 MHz / 8400 = 10 kHz
  WRITE_REG(TIM2->CCR1, (TIM_CLK/20000));           // Duty 50%
  WRITE_REG_FIELD(TIM2->CCMR1, TIM_CCMR1_OC1M, 6);  // Set output compare mode to PWM mode 1
  WRITE_REG_FIELD(TIM2->CCMR1, TIM_CCMR1_OC1PE, 1); // Enable output compare preload for channel 1
  WRITE_REG_FIELD(TIM2->CCER, TIM_CCER_CC1E, 1);    // Enable the output of channel 1
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_DIR, 0);       // Counter Up
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_ARPE, 1);      // Autoreload
  WRITE_REG(TIM2->CNT, 0);                          // restart the counter
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_CEN, 1);       // Enable the timer
}
#endif

/* TIM5 (32 bits) en Input Capture por CH1 (PA0), reiniciando el contador en cada flanco de subida */
void TIM5_IC_Init(void){

  WRITE_REG_FIELD(RCC->APB1ENR, RCC_APB1ENR_TIM5EN, 1);
  volatile unsigned int dummy;
  dummy =  RCC->APB1ENR;
  dummy =  RCC->APB1ENR;
  (void)dummy;

  WRITE_REG(TIM5->PSC, 0);                  // Sin prescaler: el contador corre a 84 MHz
  WRITE_REG(TIM5->ARR, 0xffffffff);         // maximum reload value

  WRITE_REG_FIELD(TIM5->CCMR1, TIM_CCMR1_CC1S, 1);  // enable Input capture CH1 on TI1
  WRITE_REG_FIELD(TIM5->CCMR1, TIM_CCMR1_IC1F, 3);  // Filtro digital: 8 muestras iguales (~95 ns) para rechazar ruido
  WRITE_REG_FIELD(TIM5->CCMR1, TIM_CCMR1_IC1PSC, 0);// Captura en todos los flancos validos
  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC1P, 0);    // Capture configured on rising edge
  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC1NP, 0);   // Capture configured on rising edge

  WRITE_REG_FIELD(TIM5->SMCR, TIM_SMCR_TS, 5);      // TI1FP1 selected
  WRITE_REG_FIELD(TIM5->SMCR, TIM_SMCR_SMS, 4);     // Reset the Timer on every rising capture event

  WRITE_REG_FIELD(TIM5->CR1, TIM_CR1_DIR, 0);       // Counter Up
  WRITE_REG_FIELD(TIM5->CR1, TIM_CR1_ARPE, 1);      // Autoreload

  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC1E, 1);    // Enable capture CC1
  WRITE_REG_FIELD(TIM5->DIER, TIM_DIER_CC1IE, 1);   // Enable CC1 interrupt (only on rising edge)
  WRITE_REG_FIELD(TIM5->CR1, TIM_CR1_CEN, 1);       // Enable the timer

  NVIC_SetPriority(TIM5_IRQn, 0);                   // La captura tiene la mayor prioridad
  NVIC_EnableIRQ(TIM5_IRQn);                        // Enable the TIM5 IRQ
}

/* --------------------------------- GPIO ---------------------------------- */
void GPIO_board_config(void){
  GPIO_InitTypeDef GPIO_Init = {0};
  uint32_t pin;

  /* Estado inicial: todo apagado antes de poner los pines como salida */
  GPIO_clock_enable(GPIOC);
  WRITE_REG(GPIOC->ODR, (SEG_ACTIVE_LOW ? SEG_MASK : 0) | (DIG_ACTIVE_LOW ? DIG_MASK : 0));

  /* PC0..PC7 segmentos y PC8..PC12 digitos: salidas push-pull */
  for(pin = SEG_FIRST_PIN; pin < DIG_FIRST_PIN + NUM_DIGITS; pin++){
    GPIO_Init.Pin = pin;
    GPIO_Init.Mode = 1; // Output mode
    GPIO_Init.Pull = 0; // No pull-up or pull-down
    GPIO_Init.Speed = 1;
    GPIO_Init.Alternate = 0;
    GPIO_Config(GPIOC,GPIO_Init);
  }

  /* PA0: entrada de la senal a medir (TIM5_CH1) */
  GPIO_Init.Pin = INPUT_PIN;
  GPIO_Init.Mode = 2; // Alternate function mode
  GPIO_Init.Pull = 2; // Pull-down: si no hay senal la entrada queda en 0
  GPIO_Init.Speed = 3;
  GPIO_Init.Alternate = 2; // Set alternate function to AF2 (TIM5_CH1)
  GPIO_Config(GPIOA,GPIO_Init);

#if SELF_TEST
  /* PA5: salida PWM de prueba (TIM2_CH1) */
  GPIO_Init.Pin = PWM_TEST_PIN;
  GPIO_Init.Mode = 2; // Alternate function mode
  GPIO_Init.Pull = 0; // No pull-up or pull-down
  GPIO_Init.Speed = 3;
  GPIO_Init.Alternate = 1; // Set alternate function to AF1 (TIM2_CH1)
  GPIO_Config(GPIOA,GPIO_Init);
#endif
}

/* --------------------------------- main ---------------------------------- */
int main()
{
  uint64_t sum;
  uint32_t count;

  clock_config();
  GPIO_board_config();

  timers.sw_tmr1_period = 1;                  // no se usa (el refresco se hace en el SysTick_Handler)
  timers.sw_tmr2_period = DISPLAY_UPDATE_MS;  // ventana de medicion / actualizacion del display

  SysTick_Init(1000);     // SysTick cada 1 ms
  SysTick_enable_IrQ(1);

#if SELF_TEST
  TIM2_PWM_10KHz_Init();
#endif
  TIM5_IC_Init();

  while(1)
  {
    if(timers.sw_tmr2_flag){
      timers.sw_tmr2_flag = 0;

      /* Seccion critica: copiar y reiniciar los acumuladores de la ISR */
      NVIC_DisableIRQ(TIM5_IRQn);
      sum = period_sum;
      count = period_count;
      period_sum = 0;
      period_count = 0;
      NVIC_EnableIRQ(TIM5_IRQn);

      if(count != 0 && sum != 0){
        /* f = TIM_CLK / periodo_promedio = TIM_CLK * count / sum  (redondeado) */
        freq = (uint32_t)(((uint64_t)TIM_CLK * count + sum / 2) / sum);
      } else {
        freq = 0;   // No llegaron flancos en la ventana: sin senal
      }
      Display_SetNumber(freq);
    }
  }
}