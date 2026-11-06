#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio.h"
#include "system.h"
#include "tft.h"
#include "uart.h"
#include "ff.h"
#include "audio.h"

#define BOTON_PIN 13    // PC13: boton azul B1 de la Nucleo (da 0 al presionarlo)

#define TAM_BUFFER 4096   // bytes por buffer = 2048 muestras de 16 bits (~93 ms a 22050 Hz)

static FATFS fs;          // datos de la tarjeta montada
static FIL   archivo;     // la cancion abierta (fuera del main: la usa el while)

// ===== Doble buffer =====
static uint8_t buffer[2][TAM_BUFFER];   // buffer[0] y buffer[1]
static volatile uint8_t  lleno[2];      // 1 = el buffer tiene audio listo para sonar
static volatile uint8_t  activo = 0;    // cual buffer esta sonando ahora (0 o 1)
static volatile uint32_t pos = 0;       // posicion (en bytes) dentro del buffer activo

// Interrupcion del TIM3: 22050 veces por segundo saca UNA muestra
void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF)
    {
        TIM3->SR &= ~TIM_SR_UIF;

        if (!lleno[activo])                 // el buffer no esta listo (la SD no alcanzo)
        {
            audio_poner_muestra(128);       // [1] ¿que valor es "silencio"?
            return;
        }

        // 1. Armar la muestra de 16 bits con signo (little endian: byte bajo primero)
        int16_t muestra = (int16_t)(buffer[activo][pos] | (buffer[activo][pos + 1] << 8));   // [2]

        // 2. Pasar de -32768..32767 a 0..255 y ponerla en el PWM
        audio_poner_muestra((uint8_t)((muestra + 32768) >> 8));                            // [3]

        // 3. Avanzar a la siguiente muestra
        pos += 2;                                                                        // [4]

        // 4. Si se acabo el buffer: marcarlo vacio y pasar al otro
        if (pos >= TAM_BUFFER)                                                                    // [5]
        {
            lleno[activo] = 0;                                                           // [6]
            activo ^= 1;            // 0 -> 1, 1 -> 0
            pos = 0;
        }
    }
}

int main(void)
{
    GPIO_InitTypeDef boton = {GPIO_MODO_ENTRADA, BOTON_PIN, GPIO_SIN_PULL, GPIO_BAJA_VELOCIDAD, 0};
    GPIO_Config(GPIOC, boton);

   // tft_init();
    //tft_llenar(TFT_AZUL);
    //tft_apagar();

    uart_init(115200);
    uart_enviar_texto("\r\nHola desde la Nucleo\r\n");

    // Montar la tarjeta y abrir la cancion
    FRESULT fr = f_mount(&fs, "", 1);
    uart_enviar_texto("f_mount: "); uart_numero(fr); uart_enviar_texto("\r\n");

    fr = f_open(&archivo, "/MUSIC/EPIC/CANC01.WAV", FA_READ);
    uart_enviar_texto("f_open: "); uart_numero(fr); uart_enviar_texto("\r\n");

    uint8_t reproduciendo = 0;
    if (fr == FR_OK)
    {
        // Leer el encabezado: despues de el empieza el audio
                // 1. Los primeros 12 bytes: "RIFF" + tamaño + "WAVE"
        uint8_t cab[12];
        UINT leidos;
        f_read(&archivo, cab, 12, &leidos);

        // 2. Recorrer los chunks hasta encontrar "data"
        uint8_t encontrado = 0;
        while (1)
        {
            uint8_t trozo[8];                       // nombre (4 letras) + tamaño (4 bytes)
            f_read(&archivo, trozo, 8, &leidos);
            if (leidos < 8) break;                  // se acabo el archivo y no aparecio "data"

            // El tamaño esta en little endian, igual que la frecuencia que ya leyeron
            uint32_t tam = trozo[4] | (trozo[5] << 8) | (trozo[6] << 16) | ((uint32_t)trozo[7] << 24);   // [A]

            // Mostrar cada chunk por la UART (para ver que tiene el archivo)
            uart_enviar_texto("Chunk: ");
            for (int i = 0; i < 4; i++) uart_enviar(trozo[i]);
            uart_enviar_texto("  tamano: "); uart_numero(tam); uart_enviar_texto("\r\n");

            if (trozo[0] == 'd' && trozo[1] == 'a' && trozo[2] == 't' && trozo[3] == 'a')       // [B]
            {
                encontrado = 1;
                break;          // el archivo ya quedo parado en el primer byte de audio
            }

            // No es "data": saltar este chunk completo
            f_lseek(&archivo, f_tell(&archivo) + tam + (tam & 1));                               // [C]
        }

                if (!encontrado)
        {
            uart_enviar_texto("No se encontro el chunk 'data'\r\n");
        }
        else
        {
            uart_enviar_texto("Encabezado OK, reproduciendo...\r\n");

            // Llenar LOS DOS buffers antes de arrancar el sonido
            f_read(&archivo, buffer[0], TAM_BUFFER, &leidos);  lleno[0] = 1;
            f_read(&archivo, buffer[1], TAM_BUFFER, &leidos);  lleno[1] = 1;

            // Arrancar el audio (el PWM y la interrupcion a la frecuencia de la cancion)
            audio_pwm_init();
            audio_muestreo_init(22050);
            reproduciendo = 1;
        }
    }

    uint32_t encendido = 0;
    uint32_t anterior  = 1;

    while (1)
    {
        // ===== Rellenar el buffer que este vacio =====
        if (reproduciendo)
        {
            for (int b = 0; b < 2; b++)
            {
                if (lleno[b] == 0)                                                                    // [10] ¿este buffer esta vacio?
                {
                    UINT leidos;
                    f_read(&archivo, buffer[b], TAM_BUFFER, &leidos);
                    if (leidos == 0)                                                      // [11] ¿se acabo el archivo?
                    {
                        reproduciendo = 0;
                        TIM3->CR1 &= ~TIM_CR1_CEN;      // detener la interrupcion
                        audio_poner_muestra(128);       // dejar en silencio
                        f_close(&archivo);
                        uart_enviar_texto("Fin de la cancion\r\n");
                        break;
                    }
                    lleno[b] = 1;
                }
            }
        }

        // ===== Boton: encender/apagar la pantalla (igual que antes) =====
        uint32_t actual = leer_pin_state(GPIOC, BOTON_PIN);
       // if (anterior == 1 && actual == 0)
       // {
         //   if (encendido == 1) { tft_apagar();   encendido = 0; }
           // else                { tft_encender(); encendido = 1; }
            //delay_ms(50);
        //}
        anterior = actual;
    }
}