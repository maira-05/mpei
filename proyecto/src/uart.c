#include "uart.h"
#include "gpio.h"
#include "stm32f4xx.h"
#include "system.h"


void uart_init(uint32_t baudios)// Inicializa el UART2 con la velocidad de baudios especificada
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN_Msk; // Habilitar reloj para USART2 (APB1, 42 MHz)

    // PA2 (TX) y PA3 (RX) en función alterna AF7
    GPIO_InitTypeDef pin_tx = {GPIO_MODO_AF, 2, GPIO_SIN_PULL, GPIO_ALTA_VELOCIDAD, 7};  //flancos rapidos del pin
    GPIO_Config(GPIOA, pin_tx); //PA2=TX
    pin_tx.Pin = 3; //cambia de pin PA2->PA3
    GPIO_Config(GPIOA, pin_tx); //PA3=RX

    USART2->BRR = (42000000U + baudios / 2) / baudios; // BRR = (f_APB1 + baudios/2) / baudios -> el + baudios/2 sirve para redondear. 42 MHz / 115200 = 364,58 -> 365
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE; // habilita TE(transmisor), TR(reseptor), y enciende la UASRT
}

void uart_enviar(uint8_t dato) 
{
    while (!(USART2->SR & USART_SR_TXE)); // Esperar hasta que el buffer de transmisión esté vacío, ya no tiene el estado anterior
    USART2->DR = dato; // Escribir en DR iniciando el envio de datos
}

void uart_enviar_texto(const char *str)
{
    while (*str) //*str= la letra actual 
    {
        uart_enviar((uint8_t)(*str)); //envia la letra 
        str++; //pasa a la siguiente letra
    }
}

void uart_hexadecimal(uint32_t valor) //envia un numero de 32 bits como 8 digitos hezadecimales
{
    char buffer[9]; // 8 dígitos hexadecimales + terminador nulo 0
    for (int i = 0; i < 8; i++)
    {
        uint8_t nibble = (valor >> (28 - i * 4)) & 0x0F; //saca los 4 bits del digito i, empezando del mayor al menor
        buffer[i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
    }
    buffer[8] = '\0'; // Terminador nulo
    uart_enviar_texto(buffer);
}

void uart_numero(uint32_t valor) //envia un numero de 32 bits en decimal 
{
    char buffer[11]; // el mayor uint32 tiene 10 digitos + terminador nulo 0
    int i = 0;
    if (valor == 0)
    {
        buffer[i++] = '0'; //caso especial para el 0, ya que el bucle while no se ejecutaria
    }
    else
    {
        while (valor > 0)
        {
            buffer[i++] = '0' + (valor % 10); //obtengo el digito menos significativo y lo convierto a caracter
            valor /= 10;//quito el digito menos significativo
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