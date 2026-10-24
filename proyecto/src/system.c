#include "system.h"
#include "stm32f4xx.h"

void SystemInit(void)
{
    // 1. Latencia de la FLASH: a 84 MHz la memoria no alcanza a responder
    //    en 1 ciclo, hay que darle ciclos de espera.
    //    (RM0368, tabla "Number of wait states according to CPU clock")
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_2WS;                                   // ¿cuántos wait states a 84 MHz y 3.3 V?
    FLASH->ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;  // prefetch y cachés

    // 2. Configurar el PLL (M, N, P) en RCC->PLLCFGR. Fuente = HSI (bit PLLSRC = 0)
    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLP | RCC_PLLCFGR_PLLQ);
    RCC->PLLCFGR |= (16U << RCC_PLLCFGR_PLLM_Pos);       // M
    RCC->PLLCFGR |= (336U << RCC_PLLCFGR_PLLN_Pos);       // N
    RCC->PLLCFGR |= (1U << RCC_PLLCFGR_PLLP_Pos);       // P = 4  (ojo: NO se escribe 4, ver tabla del RM)
    RCC->PLLCFGR |= (7U << RCC_PLLCFGR_PLLQ_Pos);       // Q

    // 3. Divisores de los buses en RCC->CFGR
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;                                    // APB1 dividido entre 2 (busquen RCC_CFGR_PPRE1_DIV2)

    // 4. Encender el PLL y esperar a que esté listo
    RCC->CR |= RCC_CR_PLLON;                                      // PLLON
    while (!(RCC->CR & RCC_CR_PLLRDY)) { }                                      // esperar PLLRDY = 1

    // 5. Cambiar el reloj del sistema al PLL y esperar a que se haga el cambio
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;                                    // SW = PLL
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) { }        // esperar SWS = PLL
}

void delay_ms(uint32_t ms) // reloj de 84 MHz: 1 ms = 84000 ciclos
{
    SysTick->LOAD = 84000 - 1; // valor de recarga para 1 ms
    SysTick->VAL = 0;          // reiniciar el contador
    SysTick->CTRL = 5;         // fuente = reloj del procesador, habilitar
    while (ms > 0) {
        while ((SysTick->CTRL & 0x10000) == 0); // esperar COUNTFLAG
        ms--;
    }
    SysTick->CTRL = 0;         // deshabilitar SysTick
}