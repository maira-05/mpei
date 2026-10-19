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


int main(void)
{
  //RP2350_ClockInit(); // Initialize the clock system 150MHz

  SysTickTimer_Init(1); // Initialize SysTick timer for 1ms interrupts


  GPIO_InitTypeDef GPIO_info;
  GPIO_info.Mode = OUTPUT;
  GPIO_info.Pin = LED;
  GPIO_info.PullUp = 0;
  GPIO_info.PullDown = 0;
  GPIO_info.Funct = IO_BANK0_GPIO0_CTRL_FUNCSEL_siob_proc_0;
  GPIO_Config(&GPIO_info);

  while(1)
  {
      GPIO_TogglePin_state(LED);
      SysTickTimer_Delay_ms(500);
  }

}
