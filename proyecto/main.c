#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio.h"
#include "system.h"
#include "tft.h"
#include "uart.h"
#include "ff.h"
#include "audio.h"

#define TAM_BUFFER     4096   // bytes por buffer (~93 ms de audio)
#define NUM_CANCIONES  40     // CANC01 ... CANC40

// ===== Botones: todos en el puerto C, entre el pin y GND =====
#define BTN_PAUSE  10        // PC10
#define BTN_STOP   11       // PC11
#define BTN_NEXT   12       // PC12
#define BTN_PREV   8        // PC8
#define BTN_MENU   6        // PC6
#define NUM_BOTONES 5
#define BTN_POWER 13        //Boton azul de la nucleo y la que encendera todo (azul=encendido, blanco=apagado)
 
// ===== Estados del reproductor =====
#define DETENIDO      0
#define REPRODUCIENDO 1
#define PAUSADO       2

static FATFS fs;
static FIL   archivo;

// ===== Doble buffer (igual que antes) =====
static uint8_t buffer[2][TAM_BUFFER];
static volatile uint8_t  lleno[2];
static volatile uint8_t  activo = 0;
static volatile uint32_t pos = 0;

static uint8_t estado  = DETENIDO;
static uint8_t cancion = 1;           // numero de la cancion actual (1..40)

// Arreglo con los pines: boton 0 = Pause, 1 = Stop, 2 = Next, 3 = Prev, 4 = Menu
static const uint8_t pines_botones[NUM_BOTONES] = {BTN_PAUSE, BTN_STOP, BTN_NEXT, BTN_PREV, BTN_MENU};
static uint8_t anterior[NUM_BOTONES] = {1, 1, 1, 1, 1};   // 1 = suelto

//¿El reproductor arranca?
static uint8_t encendido = 0;   // 0 = apagado, 1 = encendido
static uint8_t anterior_power = 1;   // 1 = suelto, 0 = presionado
// ===== Interrupcion del TIM3 (IGUAL que la que ya tienen, copienla aqui) =====
void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF)
    {
        TIM3->SR &= ~TIM_SR_UIF;

        if (!lleno[activo])
        {
            audio_poner_muestra(128);
            return;
        }

        int16_t muestra = (int16_t)(buffer[activo][pos] | (buffer[activo][pos + 1] << 8));
        audio_poner_muestra((uint8_t)((muestra + 32768) >> 8));
        pos += 2;

        if (pos >= TAM_BUFFER)
        {
            lleno[activo] = 0;
            activo ^= 1;
            pos = 0;
        }
    }
}

// ===== Configurar los 5 botones =====
static void botones_init(void)
{
    for (int i = 0; i < NUM_BOTONES; i++)
    {
        GPIO_InitTypeDef b = {GPIO_MODO_ENTRADA, pines_botones[i], GPIO_PULL_UP, GPIO_BAJA_VELOCIDAD, 0};   // [2]
        GPIO_Config(GPIOC, b);
    }
}

// ===== Devuelve que boton se acaba de presionar (0..4), o -1 si ninguno =====
static int boton_presionado(void)
{
    int presionado = -1;
    for (int i = 0; i < NUM_BOTONES; i++)
    {
        uint8_t actual = leer_pin_state(GPIOC, pines_botones[i]);
        if (anterior[i] == 1 && actual == 0 && presionado == -1)    // [3] flanco: suelto -> presionado
            presionado = i;
        anterior[i] = actual;
    }
    if (presionado != -1) delay_ms(30);    // antirrebote
    return presionado;
}

// ===== Silencio y detener la interrupcion =====
static void detener_audio(void)
{
    TIM3->CR1 &= ~TIM_CR1_CEN;                // [4] apagar el contador del TIM3 (ya lo usan en "Fin de la cancion")
    audio_poner_muestra(128);
}

// ===== Buscar el chunk "data" (es SU codigo del main, movido a una funcion) =====
static uint8_t buscar_data(void)    // devuelve 1 si lo encontro
{
    uint8_t cab[12];
    UINT leidos;
    f_read(&archivo, cab, 12, &leidos);
    while (1)
    {
        uint8_t trozo[8];
        f_read(&archivo, trozo, 8, &leidos);
        if (leidos < 8) return 0;
        uint32_t tam = trozo[4] | (trozo[5] << 8) | (trozo[6] << 16) | ((uint32_t)trozo[7] << 24);
        if (trozo[0] == 'd' && trozo[1] == 'a' && trozo[2] == 't' && trozo[3] == 'a') return 1;
        f_lseek(&archivo, f_tell(&archivo) + tam + (tam & 1));
    }
}

// ===== Abrir la cancion 'numero' y empezar a sonar =====
static uint8_t iniciar_cancion(uint8_t numero)   // devuelve 0 si arranco bien
{
    detener_audio();             // 1. parar lo que este sonando
    f_close(&archivo);           // 2. cerrar el archivo anterior
    estado = DETENIDO;

    // 3. Armar el nombre: "/MUSIC/EPIC/CANCxx.WAV"
    char nombre[] = "/MUSIC/EPIC/CANC00.WAV";
    nombre[16] = '0' + (numero / 10);         // nombre[16] es para el numero de la cancion 0 - 4
    nombre[17] = '0' + (numero % 10);         // nombre[17] es el segundo numero de la cancion del 0 - 9
    uart_enviar_texto("Abriendo "); uart_enviar_texto(nombre); uart_enviar_texto("\r\n");

    // 4. Abrir y buscar el audio
    if (f_open(&archivo, nombre, FA_READ) != FR_OK) { uart_enviar_texto("No se pudo abrir\r\n"); return 1; }
    if (!buscar_data()) { uart_enviar_texto("Sin chunk data\r\n"); f_close(&archivo); return 2; }

    // 5. Llenar los dos buffers y reiniciar la posicion
    UINT leidos;
    f_read(&archivo, buffer[0], TAM_BUFFER, &leidos);  lleno[0] = 1;
    f_read(&archivo, buffer[1], TAM_BUFFER, &leidos);  lleno[1] = 1;
    activo = 0;                                 // desde que cancion empieza 00 en este caso (buffer)
    pos    = 0;

    // 6. Arrancar
    audio_muestreo_init(22050);
    estado = REPRODUCIENDO;                                 // reproduce la cancion
    return 0;
}

// ===== Siguiente y anterior, dando la vuelta (40 -> 1 y 1 -> 40) =====
static uint8_t siguiente(uint8_t n) { return (n >= NUM_CANCIONES) ? 1 : n + 1; }    // [9]
static uint8_t previa(uint8_t n)    { return (n <= 1) ? NUM_CANCIONES : n - 1; }                // [10]

int main(void)
{
    tft_init();
    tft_llenar(TFT_AZUL);
    tft_apagar();

    uart_init(115200);
    uart_enviar_texto("\r\nHola desde la Nucleo\r\n");

    botones_init();
    audio_pwm_init();            // PA0 queda en silencio (128) desde el arranque
    
    GPIO_InitTypeDef power = {GPIO_MODO_ENTRADA, BTN_POWER, GPIO_SIN_PULL, GPIO_BAJA_VELOCIDAD, 0};   // [B]
    GPIO_Config(GPIOC, power);

    FRESULT fr = f_mount(&fs, "", 1);
    uart_enviar_texto("f_mount: "); uart_numero(fr); uart_enviar_texto("\r\n");

    while (1)
    {
        uint8_t power_actual = leer_pin_state(GPIOC, BTN_POWER);
        if (anterior_power == 1 && power_actual == 0)    // fl
        {
            if (encendido)                    // estaba encendido -> APAGAR
            {
                detener_audio();
                f_close(&archivo);
                estado = DETENIDO;                                            // [D]
                tft_apagar();
                encendido = 0;
                uart_enviar_texto("Apagado\r\n");
            }
            else                              // estaba apagado -> ENCENDER
            {
                tft_encender();
                encendido = 1;
                uart_enviar_texto("Encendido\r\n");
                iniciar_cancion(cancion);     // empieza a sonar la cancion donde iba
            }
            delay_ms(50);                     // antirrebote
        }
        anterior_power = power_actual;

        if (!encendido) continue;   // si esta apagado, no hacer nada mas
        // ===== 1. Rellenar buffers (SIEMPRE, mientras suena) =====

        if (estado == REPRODUCIENDO)
        {
            for (int b = 0; b < 2; b++)
            {
                if (lleno[b] == 0)
                {
                    UINT leidos;
                    f_read(&archivo, buffer[b], TAM_BUFFER, &leidos);
                    if (leidos == 0)                          // se acabo la cancion
                    {
                        uart_enviar_texto("Fin de la cancion\r\n");
                        cancion = siguiente(cancion);                         // [11] pasar a la siguiente
                        iniciar_cancion(cancion);
                        break;
                    }
                    // El ultimo pedazo casi nunca llena los 4096: el resto se llena con silencio
                    for (UINT i = leidos; i < TAM_BUFFER; i++) buffer[b][i] = 0;   // [12]
                    lleno[b] = 1;
                }
            }
        }

        // ===== 2. Botones (su cadena if / else if) =====
        int boton = boton_presionado();
        if (boton == 0)                                   // PAUSE
        {
            if (estado == REPRODUCIENDO)
            {
                detener_audio();
                estado = PAUSADO;                                              // [13]
                uart_enviar_texto("Pausa\r\n");
            }
            else if (estado == PAUSADO)
            {
                TIM3->CR1 |= TIM_CR1_CEN;                                          // [14] volver a encender el TIM3
                estado = REPRODUCIENDO;
                uart_enviar_texto("Continuar\r\n");
            }
            else                                          // estaba DETENIDO: empezar desde el principio
            {
                iniciar_cancion(cancion);
            }
        }
        else if (boton == 1)                              // STOP
        {
            detener_audio();
            f_close(&archivo);
            estado = DETENIDO;                                                  // [15]
            uart_enviar_texto("Stop\r\n");
        }
        else if (boton == 2)                              // NEXT
        {
            cancion = siguiente(cancion);                                                 // [16]
            iniciar_cancion(cancion);
        }
        else if (boton == 3)                              // PREV
        {
            cancion = previa(cancion);                                                 // [17]
            iniciar_cancion(cancion);
        }
        else if (boton == 4)                              // MENU (lo hacemos con la pantalla)
        {
            uart_enviar_texto("Menu (pendiente)\r\n");
        }
    }
}