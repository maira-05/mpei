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
#define BUTTON 1

volatile SW_Timers sw_timers;
volatile uint8_t input_state = 0;

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

  GPIO_info.Mode = INPUT;
  GPIO_info.Pin = BUTTON;
  GPIO_info.PullUp = 1;
  GPIO_info.PullDown = 0;
  GPIO_info.Funct = IO_BANK0_GPIO1_CTRL_FUNCSEL_siob_proc_1;
  GPIO_Config(&GPIO_info);
}


void TIMER0_Init(void)
{
  HW_PER_TIMER0->ALARM0.bit.ALARM0 = HW_PER_TIMER0->TIMELR.bit.TIMELR + 500000;
  HW_PER_TIMER0->ALARM1.bit.ALARM1 = HW_PER_TIMER0->TIMELR.bit.TIMELR + 100000;


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

  TIMER0_Init(); // Initialize Timer0

  while(1)
  {

    if(GPIO_ReadPin_state(BUTTON)) {
      if(HW_PER_TIMER0->INTR.bit.ALARM_0) // Check if Timer0 interrupt flag is set
      {
        HW_PER_TIMER0->INTR.bit.ALARM_0 = 1; // Clear the Timer0 interrupt flag
        HW_PER_TIMER0->ALARM0.bit.ALARM0 = HW_PER_TIMER0->TIMELR.bit.TIMELR + 500000; // Set the next alarm0 for Timer0
        HW_PER_TIMER0->ALARM1.bit.ALARM1 = HW_PER_TIMER0->TIMELR.bit.TIMELR + 100000; // Set the next alarm1 for Timer0
        GPIO_TogglePin_state(LED); // Toggle the LED state
      }
    } else {
      if(HW_PER_TIMER0->INTR.bit.ALARM_1) // Check if Timer0 interrupt flag is set
      {
        HW_PER_TIMER0->INTR.bit.ALARM_1 = 1; // Clear the Timer0 interrupt flag
        HW_PER_TIMER0->ALARM0.bit.ALARM0 = HW_PER_TIMER0->TIMELR.bit.TIMELR + 500000; // Set the next alarm0 for Timer0
        HW_PER_TIMER0->ALARM1.bit.ALARM1 = HW_PER_TIMER0->TIMELR.bit.TIMELR + 100000; // Set the next alarm1 for Timer0
        GPIO_TogglePin_state(LED); // Toggle the LED state
      }
    }

  }

}
