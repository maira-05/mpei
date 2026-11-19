# Reproductor de música WAV con pantalla TFT — STM32F401RE (bare-metal)

Proyecto de 
**Microprocesadores e Interfaces (MPEI) 2026** — Escuela Colombiana de Ingeniería Julio Garavito.

**Autores:** Jaime Galvis y Maira Prieto
**Profesor:** Juan David Guerrero Balaguera

Reproductor de archivos WAV que lee 40 canciones de una microSD (FAT32), las reproduce
con un DAC por PWM y muestra el nombre de la canción, el estado y un menú en una pantalla
TFT ST7789 de 240×320. Se controla con cinco botones: Pause, Stop, Next, Prev y Menu.

Todo el firmware está escrito **sin HAL, LL ni CubeMX**: los drivers de los periféricos
son propios y trabajan directamente con los registros, y el arranque (`crt0.S`), el linker
script y el Makefile también son propios. Se compila con la cadena GNU
(`arm-none-eabi-gcc` y `make`) y se graba con OpenOCD.

---

## Contenido


---

## 1. Estructura del repositorio

```
.
├── main.c               Lógica del reproductor: estados, botones, doble buffer, menú
├── crt0.S               Arranque: tabla de vectores, copia .data, limpia .bss, llama a main
├── linker_script.ld     Mapa de memoria: Flash 512 KB en 0x08000000, RAM 96 KB en 0x20000000
├── Makefile             Compilación con arm-none-eabi-gcc y grabación con OpenOCD
├── inc/                 Encabezados propios (gpio.h, spi.h, sd.h, tft.h, audio.h, ...)
│   └── cmsis/           Encabezados CMSIS (nombres de los registros del STM32F401)
├── src/                 Drivers propios
│   ├── system.c         Reloj a 84 MHz y delay_ms
│   ├── gpio.c           Configuración, lectura y escritura de pines
│   ├── uart.c           USART2 para depuración (115200 baudios)
│   ├── spi.c            SPI1 (microSD) y SPI2 (pantalla)
│   ├── sd.c             Protocolo de la microSD por SPI (CMD0, CMD8, ACMD41, CMD58, CMD17)
│   ├── audio.c          TIM2 = PWM en PA0 (~328 kHz); TIM3 = interrupción a 22 050 Hz
│   ├── tft.c            Driver de la ST7789: inicio, ventana, píxel, rectángulo
│   └── pantalla.c       Interfaz: pantalla de reproducción y menú
├── source/              FatFs (elm-chan) — sistema de archivos FAT32
│   └── diskio.c         Puente entre FatFs y sd.c (escrito por nosotros)
├── tft_fuentes/         Dibujo de texto
│   ├── tft_text.c/.h    Dibuja letras con fuentes en formato GFXfont (propio)
│   ├── gfxfont.h        Estructuras de las fuentes
│   ├── Adafruit_GFX.h   Archivo puente para usar las fuentes sin Arduino
│   └── Fonts/           Fuentes de Adafruit GFX (FreeSansBold 9/12/18 pt)
├── sd/                  [Archivos de ejemplo para la SD: NOMBRES.TXT]
├── docs/                [Informe en PDF, diagramas, fotos]
```

## 2. Materiales

| Cantidad | Componente                             | Notas                                                 |
| -------- | -------------------------------------- | ------------------------------------------------------|
| 1        | Nucleo-F401RE                          | (STM32F401RE, Cortex-M4)                              |
| 1        | Módulo lector de microSD               | SIG0104 (SPI)                                         |
| 1        | microSD                                | Formateada en FAT32, (almacenamiento)                 |
| 1        | Pantalla                               | TFT ST7789 240×320 (SPI)                              |
| 1        | Amplificador                           | GF1002/ fuente externa de 5v por temas de corriente   |
| 1        | Parlante                               | 8Ω y 10 W                                             |  
| 1        | Filtro RC pasa-bajos                   | R = 1.2 kΩ, C = 104 nF → f<sub>c</sub> ≈ [ ] kHz      |
| 5        | Pulsadores                             | Pause, Stop, Next, Prev, Menu                         |

## 3. Conexiones

| Periférico                       | Señal             | Pin del micro     |      
| -------------------------------- | ----------------- | ----------------- | 
| microSD (SPI1)                   | SCK / MISO / MOSI | PA5 / PA6 / PA7   | 
|                                  | CS                | PB6               | 
| Pantalla ST7789 (SPI2)           | SCK / MOSI        | PB13 / PB15       | 
|                                  | CS / DC / RST     | PB12 / PB14 / PB1 | 
| Audio (TIM2 CH1)                 | PWM               | PA0               | 
| Botones (a GND, pull-up interno) | Pause / Stop      | PC10 / PC11       | 
|                                  | Next / Prev       | PC12 / PC8        | 
|                                  | Menu              | PC6               | 
| Encender / apagar                | Botón azul        | PC13              | 
| Depuración (USART2)              | TX / RX           | PA2 / PA3         | 

**Audio:** PA0 → filtro RC → entrada del GF1002 → parlante.
[Explicar cómo va conectado el GF1002: alimentación, salida en puente (R− no va a GND),
potenciómetro o divisor para evitar saturación.]

**Alimentación:** [de dónde se alimenta cada módulo; el amplificador no se alimenta
desde la Nucleo porque la nucleo esta alimentada desde el computador asi que la corriente es insuficiente para manejar el parlante]

(Diagrama)

## 4. Instalar las herramientas

El proyecto se compila con la **cadena GNU para ARM**, sin IDE. Se probó en
**Windows 11 + WSL (Ubuntu)**; en Linux nativo es igual, sin el paso de `usbipd`.

### En WSL / Ubuntu

```bash
sudo apt update
sudo apt install gcc-arm-none-eabi make openocd git
```

Comprobar:

```bash
arm-none-eabi-gcc --version
openocd --version
```

