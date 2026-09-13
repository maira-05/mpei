#include <stdint.h>
#include "stm32f401.h"
#include "gpio_config.h"

uint8_t board[NUM_CELLS]; // 0 = libre, 1 = rojo (jugador), 2 = verde (máquina)

void delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms * 4000; i++);
}

void maquina_juega(void)
{
    for (int i = 0; i < NUM_CELLS; i++)
    {
        if (board[i] == 0)
        {
            board[i] = 2;
            write_pin_state(GPIOA, green_pins[i], 1);
            return;
        }
    }
}

int main(void)
{
    GPIO_Config_Triqui();

    for (int i = 0; i < NUM_CELLS; i++) board[i] = 0;

    while(1)
    {
        for (int i = 0; i < NUM_CELLS; i++)
        {
            if (board[i] == 0 && read_pin_state(GPIOC, button_pins[i]) == 0)
            {
                for (volatile uint32_t d = 0; d < 50000; d++);

                if (read_pin_state(GPIOC, button_pins[i]) == 0)
                {
                    board[i] = 1;
                    write_pin_state(GPIOB, red_pins[i], 1);

                    while (read_pin_state(GPIOC, button_pins[i]) == 0);

                    delay_ms(2000);

                    maquina_juega();
                }
            }
        }
    }
}