#include "sd.h"
#include "spi.h"
#include "gpio.h"
#include "system.h"
#include "stm32f4xx.h"
#include "uart.h"

/* Seleccionar la tarjeta: CS en bajo + un byte de "relleno" */
static void sd_seleccionar(void)
{
    escribir_pin_state(GPIOB, SD_CS, 0);
    spi1_transferir(0xFF);
}

/* Soltar la tarjeta: CS en alto + un byte de "relleno" */
static void sd_soltar(void)
{
    escribir_pin_state(GPIOB, SD_CS, 1);
    spi1_transferir(0xFF);
}

/* Envía un comando de 6 bytes y devuelve la respuesta (R1) */
static uint8_t sd_comando(uint8_t cmd, uint32_t arg, uint8_t crc) /*arg de 32 bits */
{
    spi1_transferir(0x40 | cmd);            /* byte 1: comando */
    spi1_transferir(arg >> 24);             /* byte 2: bits 31:24 */
    spi1_transferir(arg >> 16);             /* byte 3: bits 23:16 */
    spi1_transferir(arg >> 8);              /* byte 4: bits 15:8 */
    spi1_transferir(arg);                   /* byte 5: bits 7:0 */
    spi1_transferir(crc);                   /* byte 6: CRC */

    /* Esperar la respuesta: máximo 10 intentos, hasta que el bit 7 sea 0 */
    uint8_t r = 0xFF;
    for (int i = 0; i < 10; i++)    /* máximo 10 intentos */
    {
        r = spi1_transferir(0xFF);           /* ¿qué se envía para "escuchar"? */
        if ((r & 0x80) == 0) break;       /* bit 7 en 0 = llegó la respuesta */
    }
    return r;
}

uint8_t sd_init(void)
{
    GPIO_InitTypeDef cs = {GPIO_MODO_SALIDA, SD_CS, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0};
    GPIO_Config(GPIOB, cs);
    escribir_pin_state(GPIOB, SD_CS, 1);
    uart_enviar_texto("A: CS listo\r\n");

    spi1_init();
    uart_enviar_texto("B: SPI1 listo\r\n");

    delay_ms(10);
    uart_enviar_texto("C: delay listo\r\n");

    for (int i = 0; i < 10; i++) spi1_transferir(0xFF);
    uart_enviar_texto("D: 80 ciclos enviados\r\n");

    uint8_t r = 0xFF;
    for (int intento = 0; intento < 10 && r != 0x01; intento++)
    {
        sd_seleccionar();
        r = sd_comando(0, 0, 0x95);
        sd_soltar();
        delay_ms(10);
    }

    return r;
}