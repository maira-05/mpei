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

/* 5461BS-1 es de ANODO COMUN:
   - el segmento enciende con 0 (catodo)
   - el digito enciende con 1 (anodo conectado directo al pin)  */

#define INPUT_PIN         0         // PA0 -> TIM5_CH1 (AF2)

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
  static uint32_t digitos = 0;
  uint32_t odr, seg, dig;

  seg = seg_table[display_buf[digitos]];          // patron del numero (1 = segmento encendido)
  seg = (~seg) & 0xFF;                            // anodo comun: el segmento enciende con 0
  dig = (1UL << digitos);                         // el digito actual enciende con 1

  odr = GPIOC->ODR & ~(SEG_MASK | DIG_MASK);      // limpia los 13 bits del display
  WRITE_REG(GPIOC->ODR, GPIOC->ODR & ~DIG_MASK);  // 1) apaga todos los digitos (evita "fantasmas")
  odr |= (seg << SEG_FIRST_PIN) | (dig << DIG_FIRST_PIN);
  WRITE_REG(GPIOC->ODR, odr);                     // 2) pone los segmentos y enciende el digito actual

  digitos++;
  if(digitos >= NUM_DIGITS) digitos = 0;
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

/* --------------------------------- Reloj --------------------------------- */
/* clock_config() del curso usa el oscilador interno HSI (RC, error tipico 1-2 %),
   y ese error pasa directo a la frecuencia medida. Aqui se usa como fuente del PLL
   la senal de 8 MHz que entrega el ST-LINK de la NUCLEO (HSE en modo bypass, viene
   de un cristal), con los mismos 84 MHz: 8 MHz / M=8 * N=336 / P=4 = 84 MHz.
   Si el HSE no arranca, se usa clock_config() (HSI) como respaldo.               */
void clock_config_HSE(void){
  uint32_t timeout = 500000;

  WRITE_REG_FIELD(RCC->CR, RCC_CR_HSEBYP, 1);  // HSE en bypass (reloj externo, no cristal)
  WRITE_REG_FIELD(RCC->CR, RCC_CR_HSEON, 1);   // Enciende el HSE
  while(!(READ_REG_FIELD(RCC->CR, RCC_CR_HSERDY)) && timeout){
    timeout--;
  }
  if(timeout == 0){                            // No hay reloj externo: usar HSI
    WRITE_REG_FIELD(RCC->CR, RCC_CR_HSEON, 0);
    clock_config();
    return;
  }

  WRITE_REG_FIELD(FLASH->ACR,FLASH_ACR_LATENCY,FLASH_ACR_LATENCY_3WS);

  WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLSRC,1);  // Fuente del PLL = HSE
  WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLM,8);    // M=8  -> 1 MHz
  WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLN,336);  // N=336 -> 336 MHz
  WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLP,1);    // P=4  -> 84 MHz
  WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLQ,7);    // Q=7

  WRITE_REG_FIELD(RCC->CFGR,RCC_CFGR_PPRE1,4);         // APB1 = 42 MHz
  WRITE_REG_FIELD(RCC->CFGR,RCC_CFGR_PPRE2,0);
  WRITE_REG_FIELD(RCC->CFGR,RCC_CFGR_HPRE,0);

  WRITE_REG_FIELD(RCC->CR,RCC_CR_PLLON,1);             // enable the PLL
  while (! (READ_REG_FIELD(RCC->CR,RCC_CR_PLLRDY)));   // Wait for the PLL be ready

  WRITE_REG_FIELD(RCC->CFGR,RCC_CFGR_SW,RCC_CFGR_SW_PLL);
  while (!(READ_REG_FIELD(RCC->CFGR,RCC_CFGR_SWS_PLL))); // Wait for the system to switch the clk
}

/* --------------------------------- GPIO ---------------------------------- */
void GPIO_board_config(void){
  GPIO_InitTypeDef GPIO_Init = {0};
  int pin;

  /* OJO: la macro WRITE_REG_FIELD que usa GPIO_Config (archivo del curso) borra la
     configuracion de los pines de MENOR numero del mismo puerto cuando la mascara
     viene desplazada. Por eso en cada puerto se configura SIEMPRE del pin mayor
     al pin menor.                                                                */

  /* Estado inicial: todo apagado antes de poner los pines como salida */
  GPIO_clock_enable(GPIOC);
  WRITE_REG(GPIOC->ODR, SEG_MASK);   // segmentos en 1 (apagados) y digitos en 0 (apagados)

  /* PC12..PC8 digitos y PC7..PC0 segmentos: salidas push-pull (orden descendente) */
  for(pin = DIG_FIRST_PIN + NUM_DIGITS - 1; pin >= SEG_FIRST_PIN; pin--){
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
}

/* --------------------------------- main ---------------------------------- */
int main()
{
  uint64_t sum;
  uint32_t count;

  clock_config_HSE();   // 84 MHz desde el reloj de 8 MHz del ST-LINK (respaldo: HSI)
  GPIO_board_config();

  timers.sw_tmr1_period = 1;                  // no se usa (el refresco se hace en el SysTick_Handler)
  timers.sw_tmr2_period = DISPLAY_UPDATE_MS;  // ventana de medicion / actualizacion del display

  SysTick_Init(1000);     // SysTick cada 1 ms
  SysTick_enable_IrQ(1);

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