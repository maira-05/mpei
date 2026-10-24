/* =====================================================================
 * PRUEBA DE HARDWARE: parlante + pantalla TFT (ST7789 / ILI9341)
 * VERSION 2: SPI en modo 3 (CPOL=1, CPHA=1) y a 1 MHz
 * NUCLEO-F401RE - registros directos con CMSIS, sin librerias.
 *
 * OJO: este archivo es SOLO para verificar que el hardware funciona.
 * No es parte del codigo del proyecto; los drivers del proyecto los
 * escriben ustedes.
 *
 * Reloj: HSI de fabrica = 16 MHz (no se configura el PLL).
 *
 * Que hace (en ciclo):
 *   - Pinta la pantalla de ROJO, VERDE, AZUL y BLANCO (1 s cada uno)
 *   - Con cada color cambia el tono del parlante (440, 660, 880 Hz, silencio)
 *   - El LED verde de la placa (LD2, PA5) cambia en cada paso
 *
 * Conexiones:
 *   Parlante (via GF1002):
 *     PA0 (A0)  -> resistencia 1k -> entrada L del GF1002
 *                  (10 nF de ese punto a GND, el filtro RC)
 *     GND       -> GND de entrada del GF1002
 *     5V / GND  -> alimentacion del GF1002
 *     Parlante entre L+ y L- (NO conectar L- a GND)
 *   Pantalla (SPI2):
 *     SCK -> PB13     SDA -> PB15
 *     CS  -> PB12     DC  -> PB14     RST -> PB1
 *     VCC -> 3.3V     GND -> GND
 * ===================================================================== */

#include <stdint.h>
#include "stm32f4xx.h"

/* ---------- Pines de la pantalla ---------- */
#define TFT_CS   12   /* PB12 */
#define TFT_DC   14   /* PB14 */
#define TFT_RST   1   /* PB1  */

#define TFT_W  240
#define TFT_H  320

/* ---------- Colores RGB565 ---------- */
#define ROJO    0xF800
#define VERDE   0x07E0
#define AZUL    0x001F
#define BLANCO  0xFFFF

/* ---------- Retardo con SysTick (sin interrupcion) ---------- */
static void delay_ms(uint32_t ms)
{
    SysTick->LOAD = 16000 - 1;          /* 16 MHz / 16000 = 1 kHz -> 1 ms */
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    while (ms--) {
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) { }
    }
    SysTick->CTRL = 0;
}

/* ---------- Pin como salida / funcion alterna ---------- */
static void pin_salida(GPIO_TypeDef *port, uint32_t pin)
{
    port->MODER &= ~(3U << (pin * 2));
    port->MODER |=  (1U << (pin * 2));          /* 01 = salida */
}

static void pin_af(GPIO_TypeDef *port, uint32_t pin, uint32_t af)
{
    port->MODER   &= ~(3U << (pin * 2));
    port->MODER   |=  (2U << (pin * 2));        /* 10 = funcion alterna */
    port->OSPEEDR |=  (3U << (pin * 2));        /* velocidad alta */
    port->AFR[pin / 8] &= ~(0xFU << ((pin % 8) * 4));
    port->AFR[pin / 8] |=  (af   << ((pin % 8) * 4));
}

static void pin_alto(GPIO_TypeDef *port, uint32_t pin) { port->BSRR = (1U << pin); }
static void pin_bajo(GPIO_TypeDef *port, uint32_t pin) { port->BSRR = (1U << (pin + 16)); }

/* =====================================================================
 * PARLANTE: TIM2 canal 1 en PA0 (AF1), PWM onda cuadrada
 * ===================================================================== */
static void tono_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    pin_af(GPIOA, 0, 1);                        /* PA0 = TIM2_CH1 */

    TIM2->PSC   = 16 - 1;                       /* 16 MHz / 16 = 1 MHz */
    TIM2->CCMR1 = (6U << TIM_CCMR1_OC1M_Pos)    /* PWM modo 1 */
                | TIM_CCMR1_OC1PE;
    TIM2->CCER  = TIM_CCER_CC1E;                /* habilitar salida CH1 */
    TIM2->CR1   = TIM_CR1_ARPE;
}

static void tono(uint32_t hz)
{
    if (hz == 0) {                              /* silencio */
        TIM2->CR1 &= ~TIM_CR1_CEN;
        TIM2->CCR1 = 0;
        TIM2->EGR  = TIM_EGR_UG;
        return;
    }
    uint32_t periodo = 1000000U / hz;           /* en ticks de 1 us */
    TIM2->ARR  = periodo - 1;
    TIM2->CCR1 = periodo / 2;                   /* 50 % */
    TIM2->EGR  = TIM_EGR_UG;                    /* cargar valores */
    TIM2->CR1 |= TIM_CR1_CEN;
}

/* =====================================================================
 * PANTALLA: SPI2 (PB13 = SCK, PB15 = MOSI, AF5), modo 0, 8 bits
 * ===================================================================== */
static void spi2_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    pin_af(GPIOB, 13, 5);                       /* SPI2_SCK  */
    pin_af(GPIOB, 15, 5);                       /* SPI2_MOSI */

    SPI2->CR1 = SPI_CR1_MSTR                    /* maestro */
              | SPI_CR1_SSM | SPI_CR1_SSI       /* CS manejado por software */
              | SPI_CR1_CPOL | SPI_CR1_CPHA     /* modo SPI 3 */
              | (3U << SPI_CR1_BR_Pos);         /* 16 MHz / 16 = 1 MHz */
    SPI2->CR1 |= SPI_CR1_SPE;                   /* habilitar */
}

static void spi2_tx(uint8_t dato)
{
    while (!(SPI2->SR & SPI_SR_TXE)) { }
    *(volatile uint8_t *)&SPI2->DR = dato;
}

static void spi2_esperar(void)
{
    while (!(SPI2->SR & SPI_SR_TXE)) { }
    while (SPI2->SR & SPI_SR_BSY) { }
}

static void tft_cmd(uint8_t c)
{
    spi2_esperar();
    pin_bajo(GPIOB, TFT_DC);                    /* DC = 0 -> comando */
    spi2_tx(c);
    spi2_esperar();
    pin_alto(GPIOB, TFT_DC);                    /* DC = 1 -> datos */
}

static void tft_dato(uint8_t d) { spi2_tx(d); }

static void tft_init(void)
{
    pin_salida(GPIOB, TFT_CS);
    pin_salida(GPIOB, TFT_DC);
    pin_salida(GPIOB, TFT_RST);
    pin_alto(GPIOB, TFT_CS);
    spi2_init();

    /* Reset por hardware */
    pin_alto(GPIOB, TFT_RST); delay_ms(10);
    pin_bajo(GPIOB, TFT_RST); delay_ms(20);
    pin_alto(GPIOB, TFT_RST); delay_ms(150);

    pin_bajo(GPIOB, TFT_CS);                    /* seleccionar pantalla */

    tft_cmd(0x01); delay_ms(150);               /* SWRESET */
    tft_cmd(0x11); delay_ms(150);               /* SLPOUT: salir de sleep */
    tft_cmd(0x3A); tft_dato(0x55);              /* COLMOD: 16 bits por pixel */
    tft_cmd(0x36); tft_dato(0x00);              /* MADCTL: orientacion */
    tft_cmd(0x21);                              /* INVON (ST7789 lo necesita) */
    tft_cmd(0x13);                              /* NORON */
    tft_cmd(0x29); delay_ms(50);                /* DISPON: encender */
}

static void tft_llenar(uint16_t color)
{
    tft_cmd(0x2A);                              /* CASET: columnas 0..239 */
    tft_dato(0); tft_dato(0);
    tft_dato((TFT_W - 1) >> 8); tft_dato((TFT_W - 1) & 0xFF);
    tft_cmd(0x2B);                              /* RASET: filas 0..319 */
    tft_dato(0); tft_dato(0);
    tft_dato((TFT_H - 1) >> 8); tft_dato((TFT_H - 1) & 0xFF);
    tft_cmd(0x2C);                              /* RAMWR: escribir pixeles */

    uint8_t hi = color >> 8, lo = color & 0xFF;
    for (uint32_t i = 0; i < (uint32_t)TFT_W * TFT_H; i++) {
        tft_dato(hi);
        tft_dato(lo);
    }
}

/* =====================================================================
 * PROGRAMA PRINCIPAL
 * ===================================================================== */
int main(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    pin_salida(GPIOA, 5);                       /* LED LD2 */

    tono_init();
    tono(440);                                  /* suena mientras inicia */
    tft_init();

    const uint16_t colores[4] = { ROJO, VERDE, AZUL, BLANCO };
    const uint32_t tonos[4]   = { 440, 660, 880, 0 };

    while (1) {
        for (int i = 0; i < 4; i++) {
            tono(tonos[i]);
            tft_llenar(colores[i]);
            GPIOA->ODR ^= (1U << 5);            /* parpadeo del LED */
            delay_ms(1000);
        }
    }
}
