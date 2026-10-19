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


void GPIO_board_init(void)
{
  GPIO_InitTypeDef GPIO_info;
  GPIO_info.Mode = OUTPUT;
  GPIO_info.Pin = LED;
  GPIO_info.PullUp = 0;
  GPIO_info.PullDown = 0;
  GPIO_info.Funct = IO_BANK0_GPIO25_CTRL_FUNCSEL_pwm_b_4;
  GPIO_Config(&GPIO_info);

}


void PWM_CH4_Init(void){
  HW_PER_RESETS->RESET.bit.PWM = 0;
  while (HW_PER_RESETS->RESET_DONE.bit.PWM != 1U);
  HW_PER_PWM->CH4_TOP.bit.CH4_TOP = 15000; // Set the PWM period to 1000 clock cycles
  HW_PER_PWM->CH4_CC.bit.A = 0; // Set the PWM duty cycle to 50%
  HW_PER_PWM->CH4_CC.bit.B = 7500; // Set the PWM duty cycle to 50%
  HW_PER_PWM->CH4_CSR.bit.B_INV = 1; // Invert the PWM output
  HW_PER_PWM->CH4_CSR.bit.DIVMODE = 0; // Enable PWM channel 4
  HW_PER_PWM->CH4_DIV.bit.INT = 1; // Set the PWM clock divider to 1;
  //HW_PER_PWM->CH4_DIV.bit.FRAC = 0; // Set the PWM clock divider to 1;
  HW_PER_PWM->CH4_CSR.bit.EN = 1; // Enable PWM channel 4
  //HW_PER_PWM->EN.bit.CH4 = 1; // Enable PWM channel 4
}


int main(void)
{
  __enable_irq(); // Enable global interrupts
  RP2350_ClockInit(); // Initialize the clock system to 150MHz
  SysTickTimer_Init(1000); // Initialize SysTick timer for 1ms interrupts
  sw_timers.sw_tmr1_period = 5; // Set software timer 1 period to 1000ms
  
  SysTickTimer_IRQ_Enable(1); // Enable SysTick interrupt

  GPIO_board_init(); // Initialize GPIO for LED

  PWM_CH4_Init(); // Initialize PWM channel 4

  uint32_t pwm_duty_cycle = 10; // Set the PWM duty cycle
  while(1)
  {

    if(sw_timers.sw_tmr1_flag)
    {
      sw_timers.sw_tmr1_flag = 0; // Clear the software timer 1 flag
      pwm_duty_cycle += 50;
      if(pwm_duty_cycle > 15000)
      {
        pwm_duty_cycle = 10; // Reset the PWM duty cycle if it exceeds the maximum value
      }
      HW_PER_PWM->CH4_CC.bit.B = pwm_duty_cycle; // Update the PWM duty cycle
      
    }

  }

}
