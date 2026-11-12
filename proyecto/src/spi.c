#include "spi.h"
#include "gpio.h"
#include "stm32f4xx.h"

//funciones para inicializar y transferir datos por SPI1 y SPI2
void spi1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN_Msk; // Habilitar reloj para SPI1

    GPIO_InitTypeDef pin = {GPIO_MODO_AF, 5, GPIO_SIN_PULL, GPIO_MUY_ALTA_VELOCIDAD, 5}; //aquí fue donde arreglamos el problema "GPIO_BAJA_VELOCIDAD" -> "GPIO_ALTA_VELOCIDAD" razon aun no sabemos
    GPIO_Config(GPIOA, pin); // PA5 (SCK) = D13
    pin.Pin = 7;
    GPIO_Config(GPIOA, pin); // PA7 (MOSI) = D11
    pin.Pin = 6;
    pin.Pull = GPIO_PULL_UP;
    GPIO_Config(GPIOA, pin); // PA6 (MISO) = D12, con pull-up

    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI; // maestro, modo 0, CS por software
    SPI1->CR1 |= (7U << SPI_CR1_BR_Pos); // 328 kHz para iniciar la SD
    SPI1->CR1 |= SPI_CR1_SPE; // Habilitar SPI1
}

uint8_t spi1_transferir(uint8_t dato)
{
    while (!(SPI1->SR & SPI_SR_TXE)); // Esperar hasta que el buffer de transmisión esté vacío
    SPI1->DR = dato; // Enviar el dato que se escribio en la DR anteriormente
    while (!(SPI1->SR & SPI_SR_RXNE)); // Esperar hasta que haya datos recibidos
    return (uint8_t)SPI1->DR; // Leer el dato recibido 
}

void spi1_velocidad(uint32_t br) // Cambia la velocidad del SPI1 (br = 0..7, donde 0 = f_PCLK/2, 7 = f_PCLK/256)
{
    while (SPI1->SR & SPI_SR_BSY); // Esperar hasta que la transmisión haya terminado         
    SPI1->CR1 &= ~SPI_CR1_SPE; // Deshabilitar SPI1 para cambiar la velocidad
    SPI1->CR1 &= ~SPI_CR1_BR_Msk; // Limpiar los bits de velocidad
    SPI1->CR1 |= (br << SPI_CR1_BR_Pos);// Establecer la nueva velocidad
    SPI1->CR1 |= SPI_CR1_SPE; // Habilitar SPI1 nuevamente
}

void spi2_init(void) // Inicializa el SPI2 en modo maestro, con CPOL=1, CPHA=1 (modo 3), 8 bits, y habilita los pines PB13 (SCK) y PB15 (MOSI) en función alterna
{
    // Configuración de pines PB13 (SCK), PB14 (MISO), PB15 (MOSI)
    GPIO_InitTypeDef pin_sck = {GPIO_MODO_AF, 13, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 5};
    GPIO_InitTypeDef pin_mosi = {GPIO_MODO_AF, 15, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 5};

    GPIO_Config(GPIOB, pin_sck);
    GPIO_Config(GPIOB, pin_mosi);

    // Habilitar reloj para SPI2
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN_Msk;

    // Configuración de SPI2: Modo maestro, CPOL=1, CPHA=1 (modo 3), 8 bits
    SPI2->CR1 = SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_SSM | SPI_CR1_SSI;
    SPI2->CR1 |= (1U << SPI_CR1_BR_Pos); // 42 MHz / 4 = 10,5 MHz (antes /16 = 2,6 MHz; la ST7789 aguanta ~60 MHz)
    SPI2->CR1 |= SPI_CR1_SPE; // Habilitar SPI
}

void spi2_enviar(uint8_t dato)
{
    while (!(SPI2->SR & SPI_SR_TXE)); // Esperar hasta que el buffer de transmisión esté vacío
    SPI2->DR = dato; // Enviar el dato
}

void spi2_esperar(void)
{
    while (!(SPI2->SR & SPI_SR_TXE)); // Esperar hasta que el buffer de transmisión esté vacío
    while (SPI2->SR & SPI_SR_BSY); // Esperar hasta que la transmisión haya terminado
}
