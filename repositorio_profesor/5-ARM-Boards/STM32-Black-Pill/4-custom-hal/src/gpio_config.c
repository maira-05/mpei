#include "gpio_config.h"




void GPIO_Config(void)
{
  RCC->AHB1ENR |= (1 << 0); // Enable GPIOA clock in RCC_AHB1ENR register (bit 0)
  RCC->AHB1ENR |= (1 << 2); // Enable GPIOC clock in RCC_AHB1ENR register (bit 2)

  // do two dummy reads after enabling the peripheral clock, as per the errata
  volatile unsigned int dummy;
  dummy = (RCC->AHB1ENR);
  dummy = (RCC->AHB1ENR);

    
  //GPIOA->MODER |= (1 << LED_PIN*2); // Set GPIOA pin 5 as output (bits 10-11 = 01)
  //GPIOC->MODER &= ~(3 << 13*2); // Set GPIOC pin 13 as input (bits 26-27 = 00)
  GPIOA->MODER_Bits.MODER0 = 0; // Set GPIOA pin 0 as input (bits 0-1 = 01)
  GPIOA->PUPDR |= (1 << 0*2); // Set GPIOA pin 0 as pull-up (bits 0-1 = 01)
  GPIOC->MODER_Bits.MODER13 = 1; // Set GPIOC pin 13 as input (bits 26-27 = 00)


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