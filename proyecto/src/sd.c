#include "sd.h"
#include "spi.h"
#include "gpio.h"
#include "system.h"
#include "stm32f4xx.h"
#include "uart.h"

static uint8_t sd_es_sdhc = 0; //Variable que indica si la tarjeta es SDHC (1) o SDSC (0). Se determina en sd_init() y se usa en sd_leer_sector() para calcular la dirección del sector.

//Selecciona la tarjeta: CS en bajo + un byte de "relleno"
static void sd_seleccionar(void)
{
    escribir_pin_state(GPIOB, SD_CS, 0);
    spi1_transferir(0xFF);
}

//Soltar la tarjeta: CS en alto (deselecciona) + un byte de "relleno" para liberar la MISO
static void sd_soltar(void)
{
    escribir_pin_state(GPIOB, SD_CS, 1);
    spi1_transferir(0xFF);
}

//Envía un comando a la tarjeta SD y espera la respuesta. Devuelve el byte de respuesta (R1).
static uint8_t sd_comando(uint8_t cmd, uint32_t arg, uint8_t crc)
{
// Esperar a que la tarjeta este libre: mientras esta ocupada responde 0x00; cuando responde 0xFF, se puede enviar (maximo 1000 intentos)
    for (int i = 0; i < 1000; i++)
    {
        if (spi1_transferir(0xFF) == 0xFF) break;
    }

    spi1_transferir(0x40 | cmd); //byte 1: comando (bit 6=1, bit 7=0)
    spi1_transferir(arg >> 24); //byte 2: bits 31:24
    spi1_transferir(arg >> 16); //byte 3: bits 23:16
    spi1_transferir(arg >> 8); //byte 4: bits 15:8
    spi1_transferir(arg); //byte 5: bits 7:0
    spi1_transferir(crc); // byte 6: CRC. En modo SPI solo se revisa en CMD0 (0x95) y CMD8 (0x87); en los demas se envia 0x01

    //Esperar la respuesta R1: siempre tiene el bit 7 = 0. La tarjeta tarda hasta 8 bytes en responder (maximo 10 intentos)
    uint8_t r = 0xFF;
    for (int i = 0; i < 10; i++)
    {
        r = spi1_transferir(0xFF);
        if ((r & 0x80) == 0) break;
    }
    return r;
}

// Imprime 4 bytes separados por espacios
static void imprimir_4_bytes(uint8_t *b)
{
    for (int i = 0; i < 4; i++)
    {
        uart_hexadecimal(b[i]);
        uart_enviar_texto(" ");
    }
}

// Inicializa la tarjeta SD y la pone en modo SPI. Devuelve 0 si la tarjeta está lista, o un código de error (1, 2 o 3) si hubo un problema.
uint8_t sd_init(void)
{
    sd_es_sdhc = 0;
    GPIO_InitTypeDef cs = {GPIO_MODO_SALIDA, SD_CS, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0};
    GPIO_Config(GPIOB, cs);
    escribir_pin_state(GPIOB, SD_CS, 1);

    spi1_init();
    delay_ms(10);

    //cmd0 ayuda a que la tarjeta entre en modo SPI. Debe responder con 0x01 (idle)
    uint8_t r = 0xFF;
    int intentos_cmd0 = 0;
    for (int intento = 0; intento < 10 && r != 0x01; intento++)
    {
        for (int i = 0; i < 10; i++) spi1_transferir(0xFF); // 10 bytes de 0xFF = 80 pulsos de reloj con CS en alto: la SD necesita minimo 74 para "despertar" antes del CMD0
        sd_seleccionar();
        r = sd_comando(0, 0, 0x95);
        sd_soltar();
        intentos_cmd0 = intento;
        delay_ms(10);
    }
    uart_enviar_texto("CMD0:   "); uart_hexadecimal(r);
    uart_enviar_texto("  (intento "); uart_numero(intentos_cmd0); uart_enviar_texto(")\r\n");
    if (r != 0x01) return 1;   // la tarjeta no respondió al CMD0: no seguir

    //CMD8: enviar el patrón 0x1AA para verificar que la tarjeta es v2.0 o superior y que soporta voltajes de 2.7-3.6 V
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
        //Si hay ruido en la miso o la tarjeta no soporta CMD8, el eco puede ser incorrecto. Avisar si el eco no es 0x000001AA
        if (eco[2] != 0x01 || eco[3] != 0xAA)
            uart_enviar_texto("Aviso: eco de CMD8 invalido (revisar MISO)\r\n");
    }
    else if (r != 0x05)   // 0x05 = "comando ilegal" es decir que la tarjeta v1 (SDSC vieja): se sigue sin HCS. Cualquier otra respuesta es error
    {
        uart_enviar_texto("CMD8 sin respuesta valida\r\n");
        return 2;
    }

   // + 0x00FF8000 = ventana de voltaje 2.7-3.6 V (esta tarjeta no terminaba de iniciar sin ella). Maximo 200 intentos x 10 ms = 2 s
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

    //CMD58: leer el OCR (Operating Conditions Register) para determinar si la tarjeta es SDHC/SDXC (bit 30 = 1) o SDSC (bit 30 = 0)
    sd_seleccionar();
    r = sd_comando(58, 0, 0x01);
    uint8_t ocr[4];
    for (int i = 0; i < 4; i++) ocr[i] = spi1_transferir(0xFF);
    sd_soltar();
    uart_enviar_texto("CMD58:  "); uart_hexadecimal(r);
    uart_enviar_texto("  OCR: "); imprimir_4_bytes(ocr);
    uart_enviar_texto("\r\n");
    // Determinar si la tarjeta es SDHC/SDXC o SDSC según el bit 30 del OCR
    sd_es_sdhc = (r == 0x00 && (ocr[0] & 0x40)) ? 1 : 0; //La mascara de 0x40 significa que el bit 30 del OCR es 1, lo que indica que la tarjeta es SDHC/SDXC. Si el bit 30 es 0, la tarjeta es SDSC.

    //aumenta la velocidad de SPI1
    spi1_velocidad(3);   // 84 MHz / 16 = 5.25 MHz para leer los datos
    return 0;            // 0 = tarjeta lista
}

// Lee un sector de 512 bytes de la tarjeta SD y lo guarda en el buffer. Devuelve 0 si la lectura fue exitosa, o un código de error (1 o 2) si hubo un problema.
uint8_t sd_leer_sector(uint32_t sector, uint8_t *buffer)
{
    sd_seleccionar();

    //CMD17: leer un sector de 512 bytes. La dirección depende de si la tarjeta es SDHC (dirección = sector) o SDSC (dirección = sector * 512)
    uint32_t direccion = sd_es_sdhc ? sector : (sector << 9); // SDHC: direccion = numero de sector. SDSC: direccion en bytes = sector * 512 (<< 9 es multiplicar por 2^9 = 512)
    uint8_t r = sd_comando(17, direccion, 0x01); //sd_comando(cmd, arg, crc) envía el comando y devuelve el byte de respuesta (R1)
    if (r != 0x00)
    {
        sd_soltar();
        return 1; // error 1: la tarjeta no acepto el CMD17
    }

    //Espera a que llegue el token de inicio de datos (0xFE). La tarjeta puede tardar un tiempo en enviar el token, así que se hace un bucle de espera.
    uint8_t token = 0xFF;
    for (uint32_t i = 0; i < 100000; i++)
    {
        token = spi1_transferir(0xFF);
        if (token != 0xFF) break;   //espera hasta que llegue un byte distinto de 0xFF
    }
    if (token != 0xFE) // 0xFE = inicio de datos 
    {
        sd_soltar();
        return 2; // error: no llegó el inicio de datos 
    }

    //lee los 512 bytes del sector y los guarda en el buffer
    for (int i = 0; i < 512; i++) //lo hace byte por byte
    {
        buffer[i] = spi1_transferir(0xFF);
    }

    // Descartar los 2 bytes de CRC
    spi1_transferir(0xFF);
    spi1_transferir(0xFF);

    sd_soltar();
    return 0;                           
}