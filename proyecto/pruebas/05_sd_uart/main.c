/* =====================================================================
 * PRUEBA DE HARDWARE: modulo microSD (SPI1) + pantalla (SPI2)
 * NUCLEO-F401RE - registros directos con CMSIS, sin librerias.
 *
 * OJO: solo verifica que la tarjeta y el modulo responden. NO lee el
 * sistema de archivos ni reproduce canciones: eso es el nucleo del
 * proyecto y lo escriben ustedes.
 *
 * Que hace:
 *   1. Inicializa la tarjeta en modo SPI (CMD0, CMD8, ACMD41, CMD58).
 *   2. Lee el sector 0 (CMD17) y revisa la firma 0x55AA.
 *   3. Revisa que la tarjeta este formateada en FAT32.
 *
 * Resultado en la pantalla:
 *   VERDE -> todo bien (tarjeta inicializada, sector 0 leido, FAT32)
 *   ROJO  -> algo fallo. El LED LD2 parpadea N veces (pausa) N veces...
 *            N = paso que fallo (ver tabla de codigos abajo)
 *   AZUL (al inicio) -> esta probando
 *
 * Codigos de error (parpadeos del LED):
 *   1 = CMD0 sin respuesta    -> cableado, alimentacion o tarjeta no insertada
 *   2 = CMD8 respuesta rara   -> tarjeta muy vieja (SD v1) o ruido
 *   3 = ACMD41 timeout        -> la tarjeta no termina de iniciar
 *   4 = CMD58 fallo
 *   5 = CMD17 no entrego datos (lectura del sector 0)
 *   6 = sector 0 sin firma 0x55AA -> tarjeta sin formato
 *   7 = no es FAT32           -> formatear en FAT32
 *
 * Conexiones:
 *   Modulo microSD (SIG0104):          Pantalla (igual que antes):
 *     VCC  -> 5V                         SCK -> PB13   SDA -> PB15
 *     GND  -> GND                        CS  -> PB12   DC  -> PB14
 *     SCK  -> PB3  (D3)                  RST -> PB1
 *     MISO -> PB4  (D5)                  VCC -> 3.3V   GND -> GND
 *     MOSI -> PB5  (D4)
 *     CS   -> PB6  (D10)
 * ===================================================================== */

#include <stdint.h>
#include "stm32f4xx.h"

/* ---------- Pines ---------- */
#define TFT_CS   12   /* PB12 */
#define TFT_DC   14   /* PB14 */
#define TFT_RST   1   /* PB1  */
#define SD_CS     6   /* PB6  */

#define TFT_W  240
#define TFT_H  320

#define ROJO    0xF800
#define VERDE   0x07E0
#define AZUL    0x001F

/* =====================================================================
 * Utilidades
 * ===================================================================== */
static void delay_ms(uint32_t ms)
{
    SysTick->LOAD = 16000 - 1;                  /* 1 ms con HSI 16 MHz */
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    while (ms--) {
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) { }
    }
    SysTick->CTRL = 0;
}

static void pin_salida(GPIO_TypeDef *port, uint32_t pin)
{
    port->MODER &= ~(3U << (pin * 2));
    port->MODER |=  (1U << (pin * 2));
}

static void pin_af(GPIO_TypeDef *port, uint32_t pin, uint32_t af)
{
    port->MODER   &= ~(3U << (pin * 2));
    port->MODER   |=  (2U << (pin * 2));
    port->OSPEEDR |=  (3U << (pin * 2));
    port->AFR[pin / 8] &= ~(0xFU << ((pin % 8) * 4));
    port->AFR[pin / 8] |=  (af   << ((pin % 8) * 4));
}

static void pin_alto(GPIO_TypeDef *port, uint32_t pin) { port->BSRR = (1U << pin); }
static void pin_bajo(GPIO_TypeDef *port, uint32_t pin) { port->BSRR = (1U << (pin + 16)); }


/* =====================================================================
 * UART2 (PA2) para ver cada paso en el PC: 115200 baudios con HSI 16 MHz
 * ===================================================================== */
static void uart_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    pin_af(GPIOA, 2, 7);
    USART2->BRR = (16000000U + 57600U) / 115200U;
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}
static void uart_c(char c) { while (!(USART2->SR & USART_SR_TXE)) { } USART2->DR = c; }
static void uart_s(const char *s) { while (*s) uart_c(*s++); }
static void uart_h(uint8_t v) { const char *h = "0123456789ABCDEF"; uart_c(h[v >> 4]); uart_c(h[v & 15]); }

/* =====================================================================
 * PANTALLA (SPI2, modo 3) - igual que en prueba_pantalla_modo3
 * ===================================================================== */
static void spi2_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    pin_af(GPIOB, 13, 5);
    pin_af(GPIOB, 15, 5);
    SPI2->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI
              | SPI_CR1_CPOL | SPI_CR1_CPHA      /* modo 3 */
              | (3U << SPI_CR1_BR_Pos);          /* 1 MHz */
    SPI2->CR1 |= SPI_CR1_SPE;
}

static void spi2_tx(uint8_t d)
{
    while (!(SPI2->SR & SPI_SR_TXE)) { }
    *(volatile uint8_t *)&SPI2->DR = d;
}

static void spi2_esperar(void)
{
    while (!(SPI2->SR & SPI_SR_TXE)) { }
    while (SPI2->SR & SPI_SR_BSY) { }
}

static void tft_cmd(uint8_t c)
{
    spi2_esperar();
    pin_bajo(GPIOB, TFT_DC);
    spi2_tx(c);
    spi2_esperar();
    pin_alto(GPIOB, TFT_DC);
}

static void tft_init(void)
{
    pin_salida(GPIOB, TFT_CS);
    pin_salida(GPIOB, TFT_DC);
    pin_salida(GPIOB, TFT_RST);
    pin_alto(GPIOB, TFT_CS);
    spi2_init();
    pin_alto(GPIOB, TFT_RST); delay_ms(10);
    pin_bajo(GPIOB, TFT_RST); delay_ms(20);
    pin_alto(GPIOB, TFT_RST); delay_ms(150);
    pin_bajo(GPIOB, TFT_CS);
    tft_cmd(0x01); delay_ms(150);
    tft_cmd(0x11); delay_ms(150);
    tft_cmd(0x3A); spi2_tx(0x55);
    tft_cmd(0x36); spi2_tx(0x00);
    tft_cmd(0x21);
    tft_cmd(0x13);
    tft_cmd(0x29); delay_ms(50);
}

static void tft_llenar(uint16_t color)
{
    tft_cmd(0x2A); spi2_tx(0); spi2_tx(0); spi2_tx(0); spi2_tx(TFT_W - 1);
    tft_cmd(0x2B); spi2_tx(0); spi2_tx(0); spi2_tx((TFT_H - 1) >> 8); spi2_tx((TFT_H - 1) & 0xFF);
    tft_cmd(0x2C);
    for (uint32_t i = 0; i < (uint32_t)TFT_W * TFT_H; i++) {
        spi2_tx(color >> 8);
        spi2_tx(color & 0xFF);
    }
}

/* =====================================================================
 * MICROSD (SPI1 en PB3/PB4/PB5, AF5, modo 0)
 * ===================================================================== */
static void spi1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    pin_af(GPIOB, 3, 5);                        /* SPI1_SCK  */
    pin_af(GPIOB, 4, 5);                        /* SPI1_MISO */
    pin_af(GPIOB, 5, 5);                        /* SPI1_MOSI */
    GPIOB->PUPDR |= (1U << (4 * 2));            /* pull-up en MISO */

    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI
              | (6U << SPI_CR1_BR_Pos);         /* 16 MHz/128 = 125 kHz (inicio <= 400 kHz) */
    SPI1->CR1 |= SPI_CR1_SPE;
}

static void spi1_velocidad_alta(void)
{
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 &= ~SPI_CR1_BR;
    SPI1->CR1 |= (1U << SPI_CR1_BR_Pos);        /* 16 MHz/4 = 4 MHz */
    SPI1->CR1 |= SPI_CR1_SPE;
}

/* Envia un byte y devuelve el que llego al mismo tiempo (full duplex) */
static uint8_t spi1_xfer(uint8_t d)
{
    while (!(SPI1->SR & SPI_SR_TXE)) { }
    *(volatile uint8_t *)&SPI1->DR = d;
    while (!(SPI1->SR & SPI_SR_RXNE)) { }
    return *(volatile uint8_t *)&SPI1->DR;
}

static void sd_sel(void)   { pin_bajo(GPIOB, SD_CS); spi1_xfer(0xFF); }
static void sd_desel(void) { pin_alto(GPIOB, SD_CS); spi1_xfer(0xFF); }

/* Envia un comando de 6 bytes y espera la respuesta R1 */
static uint8_t sd_cmd(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    spi1_xfer(0xFF);
    spi1_xfer(0x40 | cmd);
    spi1_xfer(arg >> 24);
    spi1_xfer(arg >> 16);
    spi1_xfer(arg >> 8);
    spi1_xfer(arg);
    spi1_xfer(crc);
    uint8_t r = 0xFF;
    uart_s("  CMD"); uart_h(cmd); uart_s(" bytes: ");
    for (int i = 0; i < 10 && (r & 0x80); i++) { r = spi1_xfer(0xFF); uart_h(r); uart_c(' '); }
    uart_s("\r\n");
    return r;                                   /* 0xFF = sin respuesta */
}

static uint8_t sector[512];
static uint8_t sd_es_sdhc = 0;

/* Devuelve 0 si todo bien, o el codigo de error (1..7) */
static int sd_probar(void)
{
    pin_salida(GPIOB, SD_CS);
    pin_alto(GPIOB, SD_CS);
    spi1_init();
    delay_ms(10);

    /* >= 74 ciclos de reloj con CS en alto para despertar la tarjeta */
    for (int i = 0; i < 10; i++) spi1_xfer(0xFF);

    /* CMD0: GO_IDLE_STATE -> esperar R1 = 0x01 */
    uint8_t r = 0xFF;
    sd_sel();
    for (int i = 0; i < 10 && r != 0x01; i++) r = sd_cmd(0, 0, 0x95);
    sd_desel();
    if (r != 0x01) return 1;

    /* CMD8: SEND_IF_COND (tarjetas SD v2) */
    uint8_t v2 = 0;
    sd_sel();
    r = sd_cmd(8, 0x000001AA, 0x87);
    if (r == 0x01) {
        uint8_t eco[4];
        for (int i = 0; i < 4; i++) eco[i] = spi1_xfer(0xFF);
        uart_s("  eco CMD8: "); for (int i = 0; i < 4; i++) { uart_h(eco[i]); uart_c(' '); } uart_s("\r\n");
        if (eco[2] != 0x01 || eco[3] != 0xAA) { sd_desel(); return 2; }
        v2 = 1;
    } else if (!(r & 0x04)) {                   /* 0x05 = SD v1, otra cosa = error */
        sd_desel(); return 2;
    }
    sd_desel();

    /* ACMD41: SD_SEND_OP_COND hasta que R1 = 0x00 (maximo ~1 s) */
    int listo = 0;
    for (int i = 0; i < 100 && !listo; i++) {
        sd_sel();
        sd_cmd(55, 0, 0x01);                    /* APP_CMD */
        r = sd_cmd(41, v2 ? 0x40000000 : 0, 0x01);
        sd_desel();
        if (r == 0x00) listo = 1;
        else delay_ms(10);
    }
    if (!listo) return 3;

    /* CMD58: leer OCR para saber si es SDHC (direccion por bloque) */
    if (v2) {
        sd_sel();
        r = sd_cmd(58, 0, 0x01);
        if (r != 0x00) { sd_desel(); return 4; }
        uint8_t ocr0 = spi1_xfer(0xFF);
        uint8_t o1 = spi1_xfer(0xFF), o2 = spi1_xfer(0xFF), o3 = spi1_xfer(0xFF);
        uart_s("  OCR: "); uart_h(ocr0); uart_c(' '); uart_h(o1); uart_c(' '); uart_h(o2); uart_c(' '); uart_h(o3); uart_s("\r\n");
        sd_desel();
        sd_es_sdhc = (ocr0 & 0x40) ? 1 : 0;
    }

    spi1_velocidad_alta();

    /* CMD17: leer el sector 0 */
    sd_sel();
    r = sd_cmd(17, 0, 0x01);
    if (r != 0x00) { sd_desel(); return 5; }
    uint8_t token = 0xFF;
    for (uint32_t i = 0; i < 100000 && token == 0xFF; i++) token = spi1_xfer(0xFF);
    uart_s("  token: "); uart_h(token); uart_s("\r\n");
    if (token != 0xFE) { sd_desel(); return 5; }
    for (int i = 0; i < 512; i++) sector[i] = spi1_xfer(0xFF);
    spi1_xfer(0xFF); spi1_xfer(0xFF);           /* CRC (se ignora) */
    sd_desel();

    uart_s("  sector[510..511]: "); uart_h(sector[510]); uart_c(' '); uart_h(sector[511]); uart_s("\r\n");
    /* Firma de fin de sector */
    if (sector[510] != 0x55 || sector[511] != 0xAA) return 6;

    /* FAT32: tipo de particion 0x0B/0x0C en el MBR, o texto "FAT32"
     * en el sector de arranque si la tarjeta no tiene MBR */
    uint8_t tipo = sector[0x1C2];
    int fat32 = (tipo == 0x0B || tipo == 0x0C);
    if (!fat32 && sector[82] == 'F' && sector[83] == 'A' && sector[84] == 'T'
               && sector[85] == '3' && sector[86] == '2') fat32 = 1;
    if (!fat32) return 7;

    return 0;
}

/* =====================================================================
 * PROGRAMA PRINCIPAL
 * ===================================================================== */
int main(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    pin_salida(GPIOA, 5);                       /* LED LD2 */

    uart_init();
    uart_s("\r\n=== prueba_sd_uart ===\r\n");
    uart_s("Iniciando pantalla...\r\n");
    tft_init();
    tft_llenar(AZUL);                           /* probando... */
    uart_s("Pantalla azul. Probando SD...\r\n");

    int codigo = sd_probar();
    uart_s("Resultado (0 = OK): "); uart_h((uint8_t)codigo); uart_s("\r\n");

    if (codigo == 0) {
        tft_llenar(VERDE);
        pin_alto(GPIOA, 5);                     /* LED encendido fijo = OK */
        while (1) { }
    }

    tft_llenar(ROJO);
    while (1) {                                 /* N parpadeos, pausa */
        for (int i = 0; i < codigo; i++) {
            pin_alto(GPIOA, 5); delay_ms(250);
            pin_bajo(GPIOA, 5); delay_ms(250);
        }
        delay_ms(1500);
    }
}
