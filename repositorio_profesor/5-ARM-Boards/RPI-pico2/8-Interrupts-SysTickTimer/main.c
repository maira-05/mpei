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

void SysTickTimer_IRQHandler(void)
{
  update_sw_timers(&sw_timers);
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
}

int main(void)
{
  __enable_irq(); // Enable global interrupts
  RP2350_ClockInit(); // Initialize the clock system 150MHz
  SysTickTimer_Init(1); // Initialize SysTick timer for 1ms interrupts
  sw_timers.sw_tmr1_period = 500000; // Set software timer 1 period to 1000ms
  sw_timers.sw_tmr1_count = 0; // Initialize software timer 1 count

  SysTickTimer_IRQ_Enable(1); // Enable SysTick interrupt

  GPIO_board_init(); // Initialize GPIO for LED

  while(1)
  {
    if(sw_timers.sw_tmr1_flag)
    {
      sw_timers.sw_tmr1_flag = 0; // Clear the software timer flag
      GPIO_TogglePin_state(LED); // Toggle the LED state
    }

  }

}
