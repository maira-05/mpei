#include "gpio_config.h"
const uint8_t button_pins[NUM_CELLS] = {0,1,2,3,4,5,6,7,8};
const uint8_t red_pins[NUM_CELLS]    = {0,1,2,3,4,5,6,7,8};
const uint8_t green_pins[NUM_CELLS]  = {0,1,4,6,7,8,9,10,11};



void GPIO_Config(void)
{
  RCC->AHB1ENR |= (1 << 0); // Enable GPIOA clock in RCC_AHB1ENR register (bit 0)
  RCC->AHB1ENR |= (1 << 1);
  RCC->AHB1ENR |= (1 << 2); // Enable GPIOC clock in RCC_AHB1ENR register (bit 2)

  // do two dummy reads after enabling the peripheral clock, as per the errata
  volatile unsigned int dummy;
  dummy = (RCC->AHB1ENR);
  dummy = (RCC->AHB1ENR);

    
  //GPIOA->MODER |= (1 << LED_PIN*2); // Set GPIOA pin 5 as output (bits 10-11 = 01)
  //GPIOC->MODER &= ~(3 << 13*2); // Set GPIOC pin 13 as input (bits 26-27 = 00)
  GPIOA->MODER_Bits.MODER5 = 1; // Set GPIOA pin 5 as output (bits 10-11 = 01)
  GPIOC->MODER_Bits.MODER13 = 0; // Set GPIOC pin 13 as input (bits 26-27 = 00)

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

void GPIO_Config_Triqui(void)
{
    RCC->AHB1ENR |= (1 << 0) | (1 << 1) | (1 << 2);

    volatile uint32_t dummy;
    dummy = RCC->AHB1ENR;
    dummy = RCC->AHB1ENR;

    for (int i = 0; i < NUM_CELLS; i++) {
        GPIOC->MODER &= ~(3 << (button_pins[i] * 2));
    }

    for (int i = 0; i < NUM_CELLS; i++) {
        GPIOB->MODER &= ~(3 << (red_pins[i] * 2));
        GPIOB->MODER |=  (1 << (red_pins[i] * 2));
    }

    for (int i = 0; i < NUM_CELLS; i++) {
        GPIOA->MODER &= ~(3 << (green_pins[i] * 2));
        GPIOA->MODER |=  (1 << (green_pins[i] * 2));
    }
}