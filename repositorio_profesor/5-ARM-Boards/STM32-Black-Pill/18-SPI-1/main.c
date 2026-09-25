#include <stdatomic.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "inc/gpio_config.h"
#include "inc/spi.h"
#include "inc/system_config.h"
#include "inc/gpio_config.h"
#include "inc/usart.h"

#define LED_PIN 13 // Pin PC13 corresponds to the on-board LED on the Black Pill board
#define BUTTON_PIN 0 // Pin PA0 corresponds to the on-board button on the Black Pill board

/*Declaration of two timers made by software updated every 1ms*/


/* Must be delcared volatile as the timer can update asynchounosly*/
volatile SW_Timers timers;
volatile uint8_t spi_slave_received;
volatile uint8_t spi_received_data;
uint8_t spi_slave[12] ="Slave-MSG\r\n\0";
uint8_t spi_slave_rcv[12]="";
uint8_t i=1;


/*The SysTick_Handler, was already defined as weak during the crt0.s init file, so when we define it here, 
the Vector table is updated whit the new address where the function is allocated, so that when and interrupt happen
the vector table knows where to find the SysTick_Handler*/
void SysTick_Handler(void){
  update_sw_timers(&timers);
}


void SPI3_IRQHandler(void){
  if(SPI3->SR&SPI_SR_RXNE){
    spi_slave_rcv[i] = SPI_Slave_Receive(SPI3);
    i++;
    SPI_Slave_Transmit(SPI3,spi_slave[i]);
  }
}


void GPIO_board_config(void){
  GPIO_InitTypeDef GPIO_Init; 
  GPIO_Init.Pin = LED_PIN;
  GPIO_Init.Mode = 2; // Alternate function mode
  GPIO_Init.Pull = 0; // No pull-up or pull-down
  GPIO_Init.Speed = 3; 
  GPIO_Init.Alternate = 1; // Set alternate function to AF1 (TIM2_CH1)
  GPIO_Config(GPIOC,GPIO_Init);

  GPIO_Init.Pin = BUTTON_PIN;
  GPIO_Init.Mode = 0;
  GPIO_Init.Pull = 1;
  GPIO_Config(GPIOA,GPIO_Init);

  UART_GPIO_Config(USART2);
  SPI_GPIO_Config(SPI2);
  SPI_GPIO_Config(SPI3);
}

int main()
{

  clock_config();
  SysTick_Init(1000);
  SysTick_enable_IrQ(1);
   
  timers.sw_tmr1_period = 1000;
  timers.sw_tmr2_period = 1000;
  /*Enable the CLK to the GPIOA and GPIOC, this needs to be done before the configuration opf the GPIO*/
  
  GPIO_board_config();
  UART_Init(USART2,115200);
  
  SPI_Master_Init(SPI2);
  SPI_Slave_Init(SPI3);
  SPI_Enable(SPI2,1);
  SPI_Enable(SPI3,1);
  SPI_Slave_IRQ(SPI3,1);

  NVIC_EnableIRQ(SPI3_IRQn);

  write_pin_state(GPIOB,9,1);
  //SPI_Enable(SPI4,0);
  uint8_t spi_master[12]="MasterMSG\r\n\0";
  
  while(1)
  {
    if(timers.sw_tmr1_flag){
      timers.sw_tmr1_flag=0;
      i=0;
      sprintf(spi_master,"MasterMSG\r\n\0");
      write_pin_state(GPIOB,9,0);
      SPI_Slave_Transmit(SPI3,spi_slave[i]);
      SPI_Master_Tranfer(SPI2,spi_master,12);
      write_pin_state(GPIOB,9,1);

      uart_puts(USART2,"The SPI Master Sent: ");
      uart_puts(USART2,spi_slave_rcv);

      uart_puts(USART2,"The SPI Master Received: ");
      uart_puts(USART2,spi_master);

      uart_puts(USART2,"The SPI Slave Sent: ");
      uart_puts(USART2,spi_slave);

      uart_puts(USART2,"The SPI Slave Received: ");
      uart_puts(USART2,spi_slave_rcv);
    }
  }


}
