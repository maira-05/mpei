#include "gpio_config.h"




void GPIO_Config(void)
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Enable GPIOA clock in RCC_AHB1ENR register (bit 0)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // Enable GPIOC clock in RCC_AHB1ENR register (bit 2)

  // do two dummy reads after enabling the peripheral clock, as per the errata
  volatile unsigned int dummy;
  dummy = (RCC->AHB1ENR);
  dummy = (RCC->AHB1ENR);

  
  GPIOA->MODER &= ~(3 << GPIO_MODER_MODER0_Pos);
  GPIOA->PUPDR |= (1 << GPIO_PUPDR_PUPD0_Pos); // Set GPIOA pin 0 as pull-up (bits 0-1 = 01)
  GPIOC->MODER |= (1 << GPIO_MODER_MODER13_Pos);

}

uint8_t read_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin){
  return (GPIOx->IDR & (1 << pin)) ? 1 : 0;
}

void write_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state){
  if(state){
    GPIOx->ODR |= (1 << pin);
  } else {
    GPIOx->ODR &= ~(1 << pin);
  }
}