#include "spi.h"
#include "gpio.h"
#include "stm32f4xx.h"

void spi1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN_Msk; // Habilitar reloj para SPI1

    GPIO_InitTypeDef pin = {GPIO_MODO_AF, 5, GPIO_SIN_PULL, GPIO_BAJA_VELOCIDAD, 5};
    GPIO_Config(GPIOA, pin); // PA5 (SCK)  = D13
    pin.Pin = 7;
    GPIO_Config(GPIOA, pin); // PA7 (MOSI) = D11
    pin.Pin = 6;
    pin.Pull = GPIO_PULL_UP;
    GPIO_Config(GPIOA, pin); // PA6 (MISO) = D12, con pull-up

    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI; // maestro, modo 0, CS por software
    SPI1->CR1 |= (7U << SPI_CR1_BR_Pos);                  // 328 kHz para iniciar la SD
    SPI1->CR1 |= SPI_CR1_SPE;                             // Habilitar SPI1
}

uint8_t spi1_transferir(uint8_t dato)
{
    while (!(SPI1->SR & SPI_SR_TXE)); // Esperar hasta que el buffer de transmisión esté vacío
    SPI1->DR = dato; // Enviar el dato
    while (!(SPI1->SR & SPI_SR_RXNE)); // Esperar hasta que haya datos recibidos
    return (uint8_t)SPI1->DR; // Leer el dato recibido (descartar)
}

void spi1_velocidad(uint32_t br)
{
    while (SPI1->SR & SPI_SR_BSY);          
    SPI1->CR1 &= ~SPI_CR1_SPE;              
    SPI1->CR1 &= ~SPI_CR1_BR_Msk;           
    SPI1->CR1 |= (br << SPI_CR1_BR_Pos);   
    SPI1->CR1 |= SPI_CR1_SPE;
}

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
