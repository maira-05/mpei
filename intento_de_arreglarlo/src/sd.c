#include "sd.h"
#include "spi.h"
#include "gpio.h"
#include "system.h"
#include "stm32f4xx.h"
#include "uart.h"

static uint8_t sd_es_sdhc = 0;   /* 1 = SDHC/SDXC (direccion por bloque) */

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
static uint8_t sd_comando(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    /* Esperar a que la tarjeta esté lista (responde 0xFF) */
    for (int i = 0; i < 1000; i++)
    {
        if (spi1_transferir(0xFF) == 0xFF) break;
    }

    spi1_transferir(0x40 | cmd);            /* byte 1: comando */
    spi1_transferir(arg >> 24);             /* byte 2: bits 31:24 */
    spi1_transferir(arg >> 16);             /* byte 3: bits 23:16 */
    spi1_transferir(arg >> 8);              /* byte 4: bits 15:8 */
    spi1_transferir(arg);                   /* byte 5: bits 7:0 */
    spi1_transferir(crc);                   /* byte 6: CRC */

    /* Esperar la respuesta: máximo 10 intentos, hasta que el bit 7 sea 0 */
    uint8_t r = 0xFF;
    for (int i = 0; i < 10; i++)
    {
        r = spi1_transferir(0xFF);
        if ((r & 0x80) == 0) break;
    }
    return r;
}

/* Imprime 4 bytes separados por espacios */
static void imprimir_4_bytes(uint8_t *b)
{
    for (int i = 0; i < 4; i++)
    {
        uart_hexadecimal(b[i]);
        uart_enviar_texto(" ");
    }
}

uint8_t sd_init(void)
{
    sd_es_sdhc = 0;
    GPIO_InitTypeDef cs = {GPIO_MODO_SALIDA, SD_CS, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0};
    GPIO_Config(GPIOB, cs);
    escribir_pin_state(GPIOB, SD_CS, 1);

    spi1_init();
    delay_ms(10);

    /* ---- CMD0: entrar en modo SPI (con reintentos) ---- */
    uint8_t r = 0xFF;
    int intentos_cmd0 = 0;
    for (int intento = 0; intento < 10 && r != 0x01; intento++)
    {
        for (int i = 0; i < 10; i++) spi1_transferir(0xFF);
        sd_seleccionar();
        r = sd_comando(0, 0, 0x95);
        sd_soltar();
        intentos_cmd0 = intento;
        delay_ms(10);
    }
    uart_enviar_texto("CMD0:   "); uart_hexadecimal(r);
    uart_enviar_texto("  (intento "); uart_numero(intentos_cmd0); uart_enviar_texto(")\r\n");
    if (r != 0x01) return 1;   // la tarjeta no respondió al CMD0: no seguir

    /* ---- CMD8: ¿es una tarjeta SD v2? ---- */
    uint8_t v2 = 0;
    sd_seleccionar();
    r = sd_comando(8, 0x000001AA, 0x87);
    uint8_t eco[4];
    for (int i = 0; i < 4; i++) eco[i] = spi1_transferir(0xFF);
    sd_soltar();
    uart_enviar_texto("CMD8:   "); uart_hexadecimal(r);
    uart_enviar_texto("  eco: "); imprimir_4_bytes(eco);
    uart_enviar_texto("\r\n");

    if (r == 0x01)
    {
        v2 = 1;
        /* El eco debe ser 00 00 01 AA. Si no coincide, hay ruido en MISO;
           se avisa pero se intenta inicializar igual como SD v2. */
        if (eco[2] != 0x01 || eco[3] != 0xAA)
            uart_enviar_texto("Aviso: eco de CMD8 invalido (revisar MISO)\r\n");
    }
    else if (r != 0x05)   /* 0x05 = comando ilegal => tarjeta SD v1 */
    {
        uart_enviar_texto("CMD8 sin respuesta valida\r\n");
        return 2;
    }

    /* ---- ACMD41: inicializar (repetir hasta que responda 00) ----
       El argumento debe incluir la ventana de voltaje 2.7-3.6 V.
       Solo con HCS (0x40000000) esta tarjeta se queda en 0x01 para siempre. */
    uint32_t arg = (v2 ? 0x40000000U : 0x00000000U) | 0x00FF8000U;
    int intentos_acmd41 = 0;
    for (int i = 0; i < 200; i++)
    {
        sd_seleccionar();
        sd_comando(55, 0, 0x01);
        r = sd_comando(41, arg, 0x01);
        sd_soltar();
        intentos_acmd41 = i;
        if (r == 0x00) break;
        delay_ms(10);
    }
    uart_enviar_texto("ACMD41: "); uart_hexadecimal(r);
    uart_enviar_texto("  (intento "); uart_numero(intentos_acmd41); uart_enviar_texto(")\r\n");
    if (r != 0x00) return 3;   // la tarjeta no terminó de iniciarse

    /* ---- CMD58: leer el OCR ---- */
    sd_seleccionar();
    r = sd_comando(58, 0, 0x01);
    uint8_t ocr[4];
    for (int i = 0; i < 4; i++) ocr[i] = spi1_transferir(0xFF);
    sd_soltar();
    uart_enviar_texto("CMD58:  "); uart_hexadecimal(r);
    uart_enviar_texto("  OCR: "); imprimir_4_bytes(ocr);
    uart_enviar_texto("\r\n");

    /* Bit 30 del OCR (CCS): 1 = SDHC/SDXC (direccion por bloque) */
    sd_es_sdhc = (r == 0x00 && (ocr[0] & 0x40)) ? 1 : 0;

    /* ---- Subir la velocidad del SPI1 ---- */
    spi1_velocidad(3);   // 84 MHz / 16 = 5.25 MHz para leer los datos
    return 0;            // 0 = tarjeta lista
}

uint8_t sd_leer_sector(uint32_t sector, uint8_t *buffer)
{
    sd_seleccionar();

    /* 1. CMD17: leer un sector. SDHC usa número de sector; SDSC usa
       dirección en bytes (sector * 512). */
    uint32_t direccion = sd_es_sdhc ? sector : (sector << 9);
    uint8_t r = sd_comando(17, direccion, 0x01); /* 0x01 = CRC válido (aunque no se use) */
    if (r != 0x00)
    {
        sd_soltar();
        return 1;                       /* error: la tarjeta no aceptó el comando */
    }

    /* 2. Esperar el byte de inicio de datos (0xFE) */
    uint8_t token = 0xFF;
    for (uint32_t i = 0; i < 100000; i++)
    {
        token = spi1_transferir(0xFF);
        if (token != 0xFF) break;       /* llegó algo distinto de "esperando" */
    }
    if (token != 0xFE)                  /* 0xFE = inicio de datos */
    {
        sd_soltar();
        return 2;                       /* error: no llegó el inicio de datos */
    }

    /* 3. Leer los 512 bytes */
    for (int i = 0; i < 512; i++)       /* leer un byte y guardarlo en el buffer */ 
    {
        buffer[i] = spi1_transferir(0xFF);
    }

    /* 4. Descartar los 2 bytes de CRC */
    spi1_transferir(0xFF);
    spi1_transferir(0xFF);

    sd_soltar();
    return 0;                           /* OK */
}