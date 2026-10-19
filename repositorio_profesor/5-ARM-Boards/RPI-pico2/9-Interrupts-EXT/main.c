#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "inc/Clock.h"
#include "inc/core_arch.h"
#include "inc/gpio_config.h"
#include "inc/Platform_Types.h"
#include "inc/RP2350.h"
#include "inc/SysTickTimer.h"

#define LED 25

volatile SW_Timers sw_timers;
volatile uint8_t input_state = 0;

void SysTickTimer_IRQHandler(void)
{
  update_sw_timers(&sw_timers);
}


void IO_IRQ_BANK0_IRQn_Handler (void)
{
  if(HW_PER_IO_BANK0->PROC0_INTS0.bit.GPIO1_EDGE_HIGH == 1) // Check if external interrupt 0 occurred
  {
    HW_PER_IO_BANK0->INTR0.bit.GPIO1_EDGE_HIGH = 1; // Clear the interrupt latch for GPIO1
    HW_PER_IO_BANK0->PROC0_INTS0.bit.GPIO1_EDGE_HIGH = 0; // Clear the interrupt flag for GPIO1
    input_state = ~input_state; // Toggle the input state variable
  }
}

void GPIO_board_init(void)
{
  GPIO_InitTypeDef GPIO_info;
  GPIO_info.Mode = OUTPUT;
  GPIO_info.Pin = LED;
  GPIO_info.PullUp = 0;
  GPIO_info.PullDown = 0;
  GPIO_info.Funct = IO_BANK0_GPIO0_CTRL_FUNCSEL_siob_proc_0;
  GPIO_Config(&GPIO_info);

  GPIO_info.Mode = INPUT;
  GPIO_info.Pin = 1;
  GPIO_info.PullUp = 1;
  GPIO_info.PullDown = 0;
  GPIO_info.Funct = IO_BANK0_GPIO1_CTRL_FUNCSEL_siob_proc_1;
  GPIO_Config(&GPIO_info);
}


void EXTIRQ_Init(void)
{
  HW_PER_IO_BANK0->PROC0_INTE0.bit.GPIO1_EDGE_HIGH = 1; // Enable external interrupt 0 for GPIO1

  NVIC_EnableIRQ(IO_IRQ_BANK0_IRQn); // Enable external interrupt 0
  NVIC_SetPriority(IO_IRQ_BANK0_IRQn, 1); // Set priority for external interrupt 0
}

int main(void)
{
  __enable_irq(); // Enable global interrupts
  RP2350_ClockInit(); // Initialize the clock system 150MHz
  SysTickTimer_Init(1); // Initialize SysTick timer for 1ms interrupts
  sw_timers.sw_tmr1_period = 500000; // Set software timer 1 period to 1000ms
  sw_timers.sw_tmr2_period = 100000; // Set software timer 2 period to 1000ms
  
  SysTickTimer_IRQ_Enable(1); // Enable SysTick interrupt

  GPIO_board_init(); // Initialize GPIO for LED

  GPIO_WritePin_state(LED, 1); // Turn off the LED initially
  EXTIRQ_Init(); // Initialize external interrupt for GPIO1

  while(1)
  {
    if(input_state)
    {
      if(sw_timers.sw_tmr1_flag)
      {
        sw_timers.sw_tmr1_flag = 0; // Clear the software timer flag
        GPIO_TogglePin_state(LED); // Toggle the LED state
      }
    }
    else
    {
     if(sw_timers.sw_tmr2_flag)
      {
        sw_timers.sw_tmr2_flag = 0; // Clear the software timer flag
        GPIO_TogglePin_state(LED); // Toggle the LED state
      }
    }  

  }

}
