#include "system_config.h"

void update_sw_timers(volatile SW_Timers* timer){
  timer->sw_tmr1_count++;
  timer->sw_tmr2_count++;
  if(timer->sw_tmr1_count==timer->sw_tmr1_period){
    timer->sw_tmr1_count=0;
    timer->sw_tmr1_flag=1;
  }
  if(timer->sw_tmr2_count==timer->sw_tmr2_period){
    timer->sw_tmr2_count=0;
    timer->sw_tmr2_flag=1;
  }
}


void SysTickTimer_Delay_us(uint32_t delay){
    for (uint32_t i=0; i<delay; i++) {
      while((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
    }    
}

void SysTickTimer_Delay_ms(uint32_t delay){
    for (uint32_t i=0; i<delay; i++) {
     SysTickTimer_Delay_us(1000);
    }
}

void clock_config(void){
   

}