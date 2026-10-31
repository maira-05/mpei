#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio.h"
#include "system.h"
#include "tft.h"
#include "uart.h"
#include "ff.h"                    // [1] la librería de FatFs

#define BOTON_PIN 13    // PC13: boton azul B1 de la Nucleo (da 0 al presionarlo)

static FATFS fs;                  // [2] aquí FatFs guarda los datos de la tarjeta montada

int main(void)
{
    // Configuracion del boton
    GPIO_InitTypeDef boton = {GPIO_MODO_ENTRADA, BOTON_PIN, GPIO_SIN_PULL, GPIO_BAJA_VELOCIDAD, 0};
    GPIO_Config(GPIOC, boton);

    // Iniciar la pantalla, pintarla y dejarla apagada
    tft_init();
    tft_llenar(TFT_AZUL);
    tft_apagar();

    // UART para ver mensajes en el PC
    uart_init(115200);
    uart_enviar_texto("\r\nHola desde la Nucleo\r\n");

    // ===== A. Montar la tarjeta (FatFs llama a sd_init por dentro) =====
    FRESULT fr = f_mount(&fs, "", 1);          // [3]
    uart_enviar_texto("f_mount: ");
    uart_numero(fr);
    uart_enviar_texto("\r\n");

    if (fr == FR_OK)                               // [4] solo seguir si se montó bien esta en ff.h
    {
        // ===== B. Listar los archivos de la carpeta principal =====
        DIR dir;
        FILINFO info;
        fr = f_opendir(&dir, "/MUSIC/EPIC");               // [5] ¿cuál es la carpeta principal?
        uart_enviar_texto("Archivos:\r\n");
        while (fr == FR_OK)
        {
            fr = f_readdir(&dir, &info);           // [6] ¿dónde guarda los datos del archivo?
            if (fr != FR_OK || info.fname[0] == 0) break;  // [7] ¿cómo sé que ya no hay más archivos?
            uart_enviar_texto("  ");
            uart_enviar_texto(info.fname);              // [8] el nombre del archivo
            uart_enviar_texto("  ");
            uart_numero(info.fsize);                    // [9] el tamaño del archivo
            uart_enviar_texto("\r\n");
        }
        f_closedir(&dir);

        // ===== C. Abrir la canción =====
        FIL archivo;
        fr = f_open(&archivo, "/MUSIC/EPIC/CANC01.wav", FA_READ);         // [10] nombre del archivo y modo
        uart_enviar_texto("f_open: ");
        uart_numero(fr);
        uart_enviar_texto("\r\n");

        if (fr == FR_OK)
        {
            // ===== D. Leer el encabezado WAV (44 bytes) =====
            uint8_t cab[44];
            UINT leidos;
            f_read(&archivo, cab, 44, &leidos);   // [11] ¿cuántos bytes?

            // Los 4 primeros bytes deben ser las letras R I F F
            for (int i = 0; i < 4; i++) uart_enviar(cab[i]);
            uart_enviar_texto("\r\n");

            uint16_t canales    = cab[22] | (cab[23] << 8);
            uint32_t frecuencia = cab[24] | (cab[25] << 8) | (cab[26] << 16) | ((uint32_t)cab[27] << 24);  // [12]
            uint16_t bits       = cab[34] | (cab[35] << 8);                                              // [13]

            uart_enviar_texto("Canales: ");    uart_numero(canales);    uart_enviar_texto("\r\n");
            uart_enviar_texto("Frecuencia: "); uart_numero(frecuencia); uart_enviar_texto("\r\n");
            uart_enviar_texto("Bits: ");       uart_numero(bits);       uart_enviar_texto("\r\n");

            f_close(&archivo);                        // [14]
        }
    }

    // ===== Lo de siempre: botón para encender/apagar la pantalla =====
    uint32_t encendido = 0;   // 0 = pantalla apagada, 1 = encendida
    uint32_t anterior  = 1;   // 1 = boton suelto

    while (1)
    {
        uint32_t actual = leer_pin_state(GPIOC, BOTON_PIN);

        if (anterior == 1 && actual == 0)   // se acaba de presionar
        {
            if (encendido == 1) {
                tft_apagar();
                encendido = 0;
                uart_enviar_texto("Pantalla apagada\r\n");
            } else {
                tft_encender();
                encendido = 1;
                uart_enviar_texto("Pantalla encendida\r\n");
            }
            delay_ms(50);                   // antirrebote
        }

        anterior = actual;                  // recordar para la proxima vuelta
    }
}