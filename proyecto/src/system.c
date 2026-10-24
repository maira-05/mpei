#include "system.h"
#include "stm32f4xx.h"

void SystemInit(void)
{
    // 1. Latencia de la FLASH: a 84 MHz la memoria no alcanza a responder
    //    en 1 ciclo, hay que darle ciclos de espera.
    //    (RM0368, tabla "Number of wait states according to CPU clock")
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_2;                                   // ¿cuántos wait states a 84 MHz y 3.3 V?
    FLASH->ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;  // prefetch y cachés

    // 2. Configurar el PLL (M, N, P) en RCC->PLLCFGR. Fuente = HSI (bit PLLSRC = 0)
    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLP | RCC_PLLCFGR_PLLQ);
    RCC->PLLCFGR |= (??? << RCC_PLLCFGR_PLLM_Pos);       // M
    RCC->PLLCFGR |= (??? << RCC_PLLCFGR_PLLN_Pos);       // N
    RCC->PLLCFGR |= (??? << RCC_PLLCFGR_PLLP_Pos);       // P = 4  (ojo: NO se escribe 4, ver tabla del RM)
    RCC->PLLCFGR |= (??? << RCC_PLLCFGR_PLLQ_Pos);       // Q

    // 3. Divisores de los buses en RCC->CFGR
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    RCC->CFGR |= ???;                                    // APB1 dividido entre 2 (busquen RCC_CFGR_PPRE1_DIV2)

    // 4. Encender el PLL y esperar a que esté listo
    RCC->CR |= ???;                                      // PLLON
    while (???) { }                                      // esperar PLLRDY = 1

    // 5. Cambiar el reloj del sistema al PLL y esperar a que se haga el cambio
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= ???;                                    // SW = PLL
    while ((RCC->CFGR & RCC_CFGR_SWS) != ???) { }        // esperar SWS = PLL
}