int main()
{

  clock_config();
  SysTick_Init(1000);
  SysTick_enable_IrQ(1);
   
  timers.sw_tmr1_period = 1000;
  timers.sw_tmr2_period = 1000;
  
  GPIO_board_config();
  
  TIM2_PWM_10KHz_Init();
  TIM5_IC_Init();

  UART_GPIO_Config(USART2);
  UART_Init(USART2,115200);
  UART_RX_IRQ_EN(USART2,1); 
  NVIC_EnableIRQ(USART2_IRQn);

  char text_data[64];
  uint32_t new_duty=2100;
  while(1)
  {
      if(timers.sw_tmr1_flag){
        timers.sw_tmr1_flag=0;
        freq = 42000000.0 / (float)period; 
        dutyper = ((float) duty / (float) period)*100;
        sprintf(text_data, "PWM Input: Freq=%.2f, Duty_Cycle=%.2f\r\n\0",freq,dutyper);
        uart_puts(USART2, text_data);
      }
      if(uart_flag){
        uart_flag = 0;
        if(uart_data=='+'){
          if(new_duty<4200){
            new_duty+=500;
          } 
        }
        if(uart_data=='-'){
          if(new_duty>500){
            new_duty-=500;
          } 
        }
        WRITE_REG(TIM2->CCR1, new_duty);
      }
  }
