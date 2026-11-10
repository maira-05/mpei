#include "tft.h"
#include "gpio.h"
#include "spi.h"
#include "system.h"
#include "stm32f4xx.h"

void tft_comando(uint8_t cmd)
{
    spi2_esperar(); // Asegurarse de que la transmisión anterior haya terminado
    escribir_pin_state(GPIOB, TFT_DC, 0); // DC = 0 para comando
    spi2_enviar(cmd); // Enviar el comando
    spi2_esperar(); // Asegurarse de que la transmisión haya terminado antes de cambiar el estado del pin
    escribir_pin_state(GPIOB, TFT_DC, 1); // CS = 1 para deseleccionar la pantalla
}

void tft_dato(uint8_t dato)
{
    spi2_enviar(dato);// Enviar el dato
}

void tft_init(void)
{
    // Configuración de pines para los pines de la pantalla TFT (CS, DC, RST) y a que 
    GPIO_InitTypeDef pin_cs = {GPIO_MODO_SALIDA, TFT_CS, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0}; // salida, sin pull, alta velocidad, sin función alternativa
    GPIO_InitTypeDef pin_dc = {GPIO_MODO_SALIDA, TFT_DC, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0}; // salida, sin pull, alta velocidad, sin función alternativa
    GPIO_InitTypeDef pin_rst = {GPIO_MODO_SALIDA, TFT_RST, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 0}; // salida, sin pull, alta velocidad, sin función alternativa
    
    //Configuro los pines del puerto GPIOB con la estructura de configuración
    GPIO_Config(GPIOB, pin_cs); // Configurar el pin CS, para seleccionar la pantalla
    GPIO_Config(GPIOB, pin_dc);// Configurar el pin DC, para indicar si es comando o dato
    GPIO_Config(GPIOB, pin_rst);// Configurar el pin RST, para resetear la pantalla

    // Inicialización de SPI2
    spi2_init(); // Inicializar SPI2, para comunicarme con la pantalla

    // Reset de la pantalla
    escribir_pin_state(GPIOB, TFT_RST, 0);
    delay_ms(100);
    escribir_pin_state(GPIOB, TFT_RST, 1);
    delay_ms(100);
    escribir_pin_state(GPIOB, TFT_CS, 0); // CS = 0 para seleccionar la pantalla

    // Secuencia de inicialización de la pantalla (según el datasheet)
    tft_comando(0x01); // Software reset
    delay_ms(150);
    tft_comando(0x11); // Sleep out
    delay_ms(500);
    tft_comando(0x3A); // COLMOD: Pixel Format Set
    tft_dato(0x55);    // 16 bits/pixel   
    tft_comando(0x36); // MADCTL: Memory Data Access Control
    tft_dato(0x00);    
    tft_comando(0x20);
    tft_comando(0x29); // Display on

}

// ==================== PASO 2: completar los ??? ====================
// Guíense por su tft_llenar() anterior, que ya hacía casi todo. Era así:
//
//   tft_comando(0x2A);                                     // CASET
//   tft_dato(0); tft_dato(0);                              // inicio = 0
//   tft_dato((TFT_ANCHO - 1) >> 8); tft_dato((TFT_ANCHO - 1) & 0xFF);  // fin = 239
//   tft_comando(0x2B);                                     // RASET
//   tft_dato(0); tft_dato(0);
//   tft_dato((TFT_ALTO - 1) >> 8); tft_dato((TFT_ALTO - 1) & 0xFF);    // fin = 319
//   tft_comando(0x2C);                                     // RAMWR
//   for (uint32_t i = 0; i < TFT_ANCHO * TFT_ALTO; i++) {
//       tft_dato(color >> 8); tft_dato(color & 0xFF);
//   }
//
// La idea ahora es separar eso en 3 funciones reutilizables.

// Define la ventana (rectángulo) de la pantalla donde se van a escribir los siguientes píxeles.
// x0..x1 son columnas y y0..y1 son filas, ambos extremos incluidos.
// Después de esta función, cada píxel que se envíe llena la ventana de izquierda a derecha
// y de arriba abajo; la pantalla avanza sola a la siguiente posición.
void tft_ventana(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) 
{
    tft_comando(0x2A);                            // P1: ¿qué comando define las COLUMNAS?
                                                 //     Pista: en su tft_llenar() anterior es el que tiene el comentario CASET.
    tft_dato(x0 >> 8); tft_dato(x0 & 0xFF);    // P2: ¿columna inicial: x o y? ¿qué hacen >> 8 y & 0xFF?
                                                 //     Pista: columna = horizontal. Un uint16_t son 2 bytes y SPI manda de a 1;
                                                 //     >> 8 deja el byte de arriba, & 0xFF deja el de abajo. Ej: 319 = 0x013F.
    tft_dato(x1 >> 8); tft_dato(x1 & 0xFF);    //     columna final

    tft_comando(0x2B);                            // P3: ¿qué comando define las FILAS?
                                                 //     Pista: RASET, el siguiente número después del de CASET.
    tft_dato(y0 >> 8); tft_dato(y0 & 0xFF);    //     fila inicial
    tft_dato(y1 >> 8); tft_dato(y1 & 0xFF);    //     fila final

    tft_comando(0x2C);                            // P4: ¿qué comando dice "lo que sigue son píxeles"?
                                                 //     Pista: RAMWR (RAM WRite). También está en su tft_llenar().
}

// Envía un píxel en formato RGB565 (16 bits) en dos bytes
void tft_pixel(uint16_t color)
{
    tft_dato(color >> 8);                               // P5: ¿va primero el byte alto o el bajo? ¿cómo se saca cada uno?
                                                 //     Pista: igual que en P2, pero con la variable color.
    tft_dato(color & 0xFF);
}

// Pinta un rectángulo lleno de un solo color
void tft_rectangulo(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint16_t color)
{
    tft_ventana(x, y, x + ancho - 1, y + alto - 1); // P6: si empieza en x y mide "ancho", ¿en qué columna termina? ¿y la fila?
                                                 //     Pista: tft_ventana(10, 20, 19, 29) tiene 10x10 píxeles:
                                                 //     de 10 a 19 hay 10 columnas, entonces fin = inicio + ancho - 1
    for (uint32_t i = 0; i < (uint32_t)ancho * alto; i++) // P7: ¿cuántos píxeles hay que mandar en total?
                                                 //     Pista: área de un rectángulo. Usen (uint32_t) para que no se desborde:
                                                 //     240*320 = 76800 no cabe en 16 bits.
    {
        tft_pixel(color);                        // P8: ¿qué función envía un píxel?
                                                 //     Pista: la escribieron arriba; recibe el color.
    }
}

// Pinta toda la pantalla: es un rectángulo del tamaño completo
void tft_llenar(uint16_t color)
{
    tft_rectangulo(0, 0, TFT_ANCHO, TFT_ALTO, color);   // P9: ¿desde qué esquina? ¿qué constantes de tft.h dan el tamaño?
                                                 //     Pista: la esquina superior izquierda es (0, 0); las constantes empiezan con TFT_.
}

void tft_apagar(void)
{
    tft_comando(0x28); // Display off
    delay_ms(10);
    tft_comando(0x10); // Enter sleep mode
}

void tft_encender(void)
{
    tft_comando(0x11); // Sleep out
    delay_ms(120);
    tft_comando(0x29); // Display on
}