// tft.h contiene las definiciones y funciones para manejar la pantalla TFT, incluyendo inicialización, envío de comandos y datos, y dibujo de gráficos básicos.
#ifndef __TFT_H__
#define __TFT_H__
#include <stdint.h>

// pines de control de la pantalla TFT
#define TFT_CS    12   // SELECTOR DE CHIP (Chip Select)
#define TFT_DC    14   // LÍNEA DE DATOS/COMANDO (Data/Command)
#define TFT_RST    1   // LÍNEA DE REINICIO (Reset)

// dimensiones de la pantalla TFT
#define TFT_ANCHO  240
#define TFT_ALTO   320

// codigo hexadecimal de colores en formato RGB565
#define TFT_NEGRO        0x0000
#define TFT_BLANCO       0xFFFF
#define TFT_ROJO         0xF800
#define TFT_VERDE        0x07E0
#define TFT_AZUL         0x001F
#define TFT_AMARILLO     0xFFE0   
#define TFT_CIAN         0x07FF
#define TFT_GRIS         0x8410
#define TFT_NARANJA      0xFD20
#define TFT_AZUL_OSCURO  0x0010

void tft_init(void);                // inicializa la pantalla TFT
void tft_comando(uint8_t cmd);      // envía un comando a la pantalla TFT
void tft_dato(uint8_t dato);        // envía un dato a la pantalla TFT
void tft_ventana(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);  // define una ventana de dibujo
void tft_pixel(uint16_t color);     // dibuja un píxel en la pantalla TFT
void tft_rectangulo(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint16_t color);  // dibuja un rectángulo en la pantalla TFT
void tft_llenar(uint16_t color);    // llena la ventana de dibujo con un color
void tft_encender(void);            // enciende la pantalla TFT
void tft_apagar(void);              // apaga la pantalla TFT

#endif
