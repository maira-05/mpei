#include "system.h"
#include "stm32f4xx.h"

void SystemInit(void) //f = 16 MHz / M * N / P = 16 / 16 * 336 / 4 = 84 MHz -> formula que se utiliza 
{
    // Configuración de la flash y el PLL para que el sistema funcione a 84 MHz
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_2WS; // 2 wait states para 84 MHz (60-84 MHz a 3.3 V -> 2 WS, manual de referencia)
    FLASH->ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;  // habilitar prefetch, cache de instrucciones y cache de datos

    // Configuración del PLL para generar 84 MHz a partir del HSI (16 MHz)
    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLP | RCC_PLLCFGR_PLLQ);
    RCC->PLLCFGR |= (16U << RCC_PLLCFGR_PLLM_Pos);       // estamos usando el HSI de 16 MHz donde por defecto el PLLSRC=0, asi que M = 16: 16 MHz / 16 = 1 MHz a la entrada del PLL (el manual pide 1-2 MHz)
    RCC->PLLCFGR |= (336U << RCC_PLLCFGR_PLLN_Pos);       // establece N = 336: 1 MHz * 336 = 336 MHz en el VCO (debe estar entre 100 y 432 MHz)
    RCC->PLLCFGR |= (1U << RCC_PLLCFGR_PLLP_Pos);       // establece P = 4 (en el codigo PLLP=1=> Divisor = 4)
    RCC->PLLCFGR |= (7U << RCC_PLLCFGR_PLLQ_Pos);       // establece Q = 7 (para USB, no nos importa ahora)

    // Se realiza un divisor de reloj para APB1 (TIM3 está en APB1) para que funcione a 42 MHz
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;   // APB1 a 42 MHz (TIM3 funciona a 84 MHz porque tiene un multiplicador de 2)

    // Encender el PLL y esperar a que esté listo
    RCC->CR |= RCC_CR_PLLON;   // encender el PLL
    while (!(RCC->CR & RCC_CR_PLLRDY)) { }   // esperar PLLRDY = 1

    // Cambiar el reloj del sistema al PLL y esperar a que se haga el cambio
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL; // SW = PLL
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) { } // esperar SWS = PLL
}

void delay_ms(uint32_t ms) // reloj de 84 MHz: 1 ms = 84000 ciclos
{
    SysTick->LOAD = 84000 - 1; // valor de recarga para 1 ms a 84MHz
    SysTick->VAL = 0;          // reiniciar el contador
    SysTick->CTRL = 5;         // 0b101: bit0 ENABLE=1, bit2 CLKSOURCE=1 (reloj del procesador), bit1 TICKINT=0 (sin interrupcion)
    while (ms > 0) {
        while ((SysTick->CTRL & 0x10000) == 0); //bit 16 = COUNTFLAG: se pone en 1 cada vez que el contador llega a 0
        ms--;
    }
    SysTick->CTRL = 0;         // deshabilitar SysTick
}