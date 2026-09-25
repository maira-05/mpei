#include <stdint.h>
#include "stm32f401.h"
#include "gpio_config.h"

uint8_t board[NUM_CELLS]; //Es el tablero del juego, 0 libre, 1 jugador, 2 maquina
uint8_t game_over = 0; //El juego termina cuando hay un ganador o empate
volatile uint32_t contador_random = 0; // crece constantemente mientras el programa espera, para generar un número pseudo-aleatorio
void delay_ms(uint32_t ms)//frecuencia del reloj del microcontrolador 16MHz, 1ms = 4000 ciclos
{
    for (volatile uint32_t i = 0; i < ms * 4000; i++);
}
// Funciones para encender y apagar todos los LEDs de un color (titilar)
void apagar_todos_rojos(void)  { for (int i = 0; i < NUM_CELLS; i++) write_pin_state(GPIOB, red_pins[i], 0); }
void apagar_todos_verdes(void) { for (int i = 0; i < NUM_CELLS; i++) write_pin_state(GPIOA, green_pins[i], 0); }
void prender_todos_verdes(void){ for (int i = 0; i < NUM_CELLS; i++) write_pin_state(GPIOA, green_pins[i], 1); }
void prender_todos_rojos(void) { for (int i = 0; i < NUM_CELLS; i++) write_pin_state(GPIOB, red_pins[i], 1); }

uint8_t check_winner(void)
{
    const uint8_t combos[8][3] = {
        //Posibles combinaciones para ganar
        {0,1,2}, {3,4,5}, {6,7,8},
        {0,3,6}, {1,4,7}, {2,5,8},
        {0,4,8}, {2,4,6}
    };
    for (int c = 0; c < 8; c++)
    // Recorremos todas las combinaciones posibles 
    {
        uint8_t a = board[combos[c][0]];
        uint8_t b = board[combos[c][1]];
        uint8_t d = board[combos[c][2]];
        if (a != 0 && a == b && b == d) return a;
    }
    return 0;
}

void celebrar_victoria(uint8_t ganador)//celebrar la victoria del jugador o de la máquina, dependiendo del valor de ganador (1 o 2)
{
    if (ganador == 2)
    {
        apagar_todos_rojos();
        for (int i = 0; i < 2; i++)
        {
            prender_todos_verdes();
            delay_ms(300);
            apagar_todos_verdes();
            delay_ms(300);
        }
        prender_todos_verdes();
    }
    else if (ganador == 1)
    {
        apagar_todos_verdes();
        for (int i = 0; i < 2; i++)
        {
            prender_todos_rojos();
            delay_ms(300);
            apagar_todos_rojos();
            delay_ms(300);
        }
        prender_todos_rojos();
    }
}
uint8_t tablero_lleno(void)
{
    for (int i = 0; i < NUM_CELLS; i++)
    {
        if (board[i] == 0) return 0; // todavía hay una celda libre
    }
    return 1; // no quedan celdas libres
}
void celebrar_empate(void) //en caso de empate, se encienden y apagan todos los LEDs rojos y verdes 3 veces
{
    for (int i = 0; i < 3; i++)
    {
        prender_todos_rojos();
        prender_todos_verdes();
        delay_ms(300);

        apagar_todos_rojos();
        apagar_todos_verdes();
        delay_ms(300);
    }
}

void maquina_juega(void)
{
    uint8_t libres[NUM_CELLS];
    uint8_t num_libres = 0;

    // Primero se arma la lista de celdas libres
    for (int i = 0; i < NUM_CELLS; i++)
    {
        if (board[i] == 0)
        {
            libres[num_libres] = i;
            num_libres++;
        }
    }

    if (num_libres == 0) return; // no debería pasar, pero por seguridad

    // Elegimos una al azar usando el contador como "semilla"
    uint8_t elegido = libres[contador_random % num_libres];

    board[elegido] = 2;
    write_pin_state(GPIOA, green_pins[elegido], 1);
}

int main(void)
{
    GPIO_Config_Triqui();
    for (int i = 0; i < NUM_CELLS; i++) board[i] = 0; // Inicializamos el tablero a 0 (libre)

    while(1)
    {
        contador_random++; // sigue creciendo mientras nadie presiona nada

        for (int i = 0; i < NUM_CELLS && !game_over; i++)// Recorremos todos los botones y nos detenemos si el juego terminó
        {
            if (board[i] == 0 && read_pin_state(GPIOC, button_pins[i]) == 0)
            {
                for (volatile uint32_t d = 0; d < 50000; d++);

                if (read_pin_state(GPIOC, button_pins[i]) == 0)// Si el botón sigue presionado después del retardo, se considera una pulsación válida
                {
                    board[i] = 1;
                    write_pin_state(GPIOB, red_pins[i], 1);
                    while (read_pin_state(GPIOC, button_pins[i]) == 0);

                    uint8_t ganador = check_winner();
                    if (ganador)// Si hay un ganador después del movimiento del jugador, celebramos la victoria y terminamos el juego
                    {
                        celebrar_victoria(ganador);
                        game_over = 1;
                        break;
                    }
                    else if (tablero_lleno())// Si el tablero está lleno después del movimiento del jugador, celebramos el empate y terminamos el juego
                    {
                        celebrar_empate();
                        game_over = 1;
                        break;
                    }

                    delay_ms(200);
                    maquina_juega();

                    ganador = check_winner();// Si hay un ganador después del movimiento de la máquina, celebramos la victoria y terminamos el juego
                    if (ganador)
                    {
                        celebrar_victoria(ganador);
                        game_over = 1;
                    }
                    else if (tablero_lleno())
                    {
                        celebrar_empate();
                        game_over = 1;
                    }
                }
            }
        }
    }
}