#include <stdint.h>
#include "stm32f401.h"
#include "gpio_config.h"

#define NUM_CELLS 9

extern const uint8_t button_pins[NUM_CELLS];
extern const uint8_t red_pins[NUM_CELLS];
extern const uint8_t green_pins[NUM_CELLS];

uint8_t board[NUM_CELLS]; // 0 = vacío, 1 = rojo, 2 = verde
uint8_t turno = 1;        // empieza el jugador rojo

int main(void)
{
    GPIO_Config_Triqui();

    for (int i = 0; i < NUM_CELLS; i++) board[i] = 0;

    while(1)
    {
        for (int i = 0; i < NUM_CELLS; i++)
        {
            // Botón presionado (activo en bajo) y celda todavía vacía
            if (board[i] == 0 && read_pin_state(GPIOC, button_pins[i]) == 0)
            {
                // Debounce: espera un poco y confirma que sigue presionado
                for (volatile uint32_t d = 0; d < 50000; d++);

                if (read_pin_state(GPIOC, button_pins[i]) == 0)
                {
                    if (turno == 1) {
                        write_pin_state(GPIOB, red_pins[i], 1);
                        board[i] = 1;
                    } else {
                        write_pin_state(GPIOA, green_pins[i], 1);
                        board[i] = 2;
                    }

                    turno = (turno == 1) ? 2 : 1; // alterna turno

                    // Espera a que el usuario suelte el botón antes de seguir
                    while (read_pin_state(GPIOC, button_pins[i]) == 0);
                }
            }
        }
    }
}