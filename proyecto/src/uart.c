#include "uart.h"
#include "gpio.h"
#include "stm32f4xx.h"
#include "system.h"

void uart_init(uint32_t baudios)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN_Msk; // Habilitar reloj para USART2

    // PA2 (TX) y PA3 (RX) en AF7
    GPIO_InitTypeDef pin_tx = {GPIO_MODO_AF, 2, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 7};
    GPIO_Config(GPIOA, pin_tx);
    pin_tx.Pin = 3;
    GPIO_Config(GPIOA, pin_tx);

    USART2->BRR = (42000000U + baudios / 2) / baudios;        // APB1 = 42 MHz
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE; // TX, RX y USART encendidos
}

void uart_enviar(uint8_t dato)
{
    while (!(USART2->SR & USART_SR_TXE)); // Esperar hasta que el buffer de transmisión esté vacío
    USART2->DR = dato; // Enviar el dato
}

void uart_enviar_texto(const char *str)
{
    while (*str)
    {
        uart_enviar((uint8_t)(*str));
        str++;
    }
}

void uart_hexadecimal(uint32_t valor)
{
    char buffer[9]; // 8 dígitos hexadecimales + terminador nulo
    for (int i = 0; i < 8; i++)
    {
        uint8_t nibble = (valor >> (28 - i * 4)) & 0x0F;
        buffer[i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
    }
    buffer[8] = '\0'; // Terminador nulo
    uart_enviar_texto(buffer);
}

void uart_numero(uint32_t valor)
{
    char buffer[11]; // 10 dígitos decimales + terminador nulo
    int i = 0;
    if (valor == 0)
    {
        buffer[i++] = '0';
    }
    else
    {
        while (valor > 0)
        {
            buffer[i++] = '0' + (valor % 10);
            valor /= 10;
        }
        // Invertir el orden de los dígitos
        for (int j = 0; j < i / 2; j++)
        {
            char temp = buffer[j];
            buffer[j] = buffer[i - j - 1];
            buffer[i - j - 1] = temp;
        }
    }
    buffer[i] = '\0'; // Terminador nulo
    uart_enviar_texto(buffer);
}