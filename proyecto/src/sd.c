#include "sd.h"
#include "spi.h"
#include "gpio.h"
#include "system.h"
#include "stm32f4xx.h"

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
    /* 1. CS como salida, empezando en alto (tarjeta sin seleccionar) */
    GPIO_InitTypeDef cs = {GPIO_MODO_SALIDA, SD_CS, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0};
    GPIO_Config(GPIOB, cs);
    escribir_pin_state(GPIOB, SD_CS, 1); /* tarjeta sin seleccionar */

    /* 2. SPI1 lento (328 kHz) */
    spi1_init();
    delay_ms(10);

    /* 3. Al menos 74 ciclos de reloj con CS en alto para "despertar" la tarjeta.
          Cada byte son 8 ciclos -> ¿cuántos bytes 0xFF hay que enviar? */
    for (int i = 0; i < 10; i++) spi1_transferir(0xFF); /* 10 bytes = 80 ciclos */

    /* 4. CMD0: GO_IDLE_STATE, argumento 0, CRC 0x95 */
    sd_seleccionar();
    uint8_t r = sd_comando(0, 0, 0x95);
    sd_soltar();

    return r;   /* 0x01 = la tarjeta entró en modo SPI */
}