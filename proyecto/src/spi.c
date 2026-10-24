#include "spi.h"
#include "gpio.h"
#include "stm32f4xx.h"

void spi2_init(void)
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
    SPI2->CR1 |= (3U << SPI_CR1_BR_Pos); // Configurar la velocidad del reloj (prescaler)
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