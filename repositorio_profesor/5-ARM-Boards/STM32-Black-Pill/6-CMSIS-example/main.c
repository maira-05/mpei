#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx.h"
#include "inc/gpio_config.h"
#include "inc/SysTick.h"

#define LED_PIN 13 // Pin 13 corresponds to the on-board LED on the NUCLEO-64 board
#define BUTTON_PIN 0 // Pin 0 corresponds to the on-board button on the NUCLEO-64 board

int main()
{

  GPIO_Config();
  SysTick_Init();

  while(1)
  {

    if(read_pin_state(GPIOA, BUTTON_PIN) != 0)
    {
      write_pin_state(GPIOC, LED_PIN, 1);
      SysTick_Delay_us(500000);
      write_pin_state(GPIOC, LED_PIN, 0);
      SysTick_Delay_us(500000);
    }
    else
    {
      write_pin_state(GPIOC, LED_PIN, 0);
      SysTick_Delay_us(250000);
      write_pin_state(GPIOC, LED_PIN, 1);
      SysTick_Delay_us(250000);
    }
    
  }


}
