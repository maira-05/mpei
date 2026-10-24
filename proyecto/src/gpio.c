#include "gpio.h"


void GPIO_clock_habilitar(volatile GPIO_TypeDef *GPIOx){
  if(GPIOx==GPIOA){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN_Msk; //habilito el reloj del puerto GPIOA
  } else if(GPIOx==GPIOB){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN_Msk; //habilito el reloj del puerto GPIOB
  } else if(GPIOx==GPIOC){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN_Msk; //habilito el reloj del puerto GPIOC
  } else if(GPIOx==GPIOD){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN_Msk; //habilito el reloj del puerto GPIOD
  } else if(GPIOx==GPIOE){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN_Msk; //habilito el reloj del puerto GPIOE
  } else if(GPIOx==GPIOH){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN_Msk; //habilito el reloj del puerto GPIOH
  }
  volatile unsigned int dummy;
  dummy = (RCC->AHB1ENR);//leo el registro para asegurarme de que el reloj se ha habilitado
  dummy = (RCC->AHB1ENR);
  (void)dummy; //evito la advertencia de variable no utilizada
}

void GPIO_Config(volatile GPIO_TypeDef *GPIOx, GPIO_InitTypeDef Config)
{
  GPIO_clock_habilitar(GPIOx); //habilito el reloj del puerto GPIOx
  GPIOx->MODER &= ~(3U << (Config.Pin * 2));
  GPIOx->MODER |=  (Config.Mode << (Config.Pin * 2));          //00 = entrada, 01 = salida, 10 = función alternativa, 11 = analógico
  GPIOx->PUPDR &= ~(3U << (Config.Pin * 2));
  GPIOx->PUPDR |=  (Config.Pull << (Config.Pin * 2));          /* 00 = sin pull-up/pull-down, 01 = pull-up, 10 = pull-down */
  GPIOx->OSPEEDR &= ~(3U << (Config.Pin * 2));
  GPIOx->OSPEEDR |=  (Config.Speed << (Config.Pin * 2));          /* 00 = baja velocidad, 01 = media velocidad, 10 = alta velocidad, 11 = muy alta velocidad */
  GPIOx->AFR[Config.Pin >> 3] &= ~(0xFU << ((Config.Pin & 0x07) * 4));
  GPIOx->AFR[Config.Pin >> 3] |=  (Config.Alternate << ((Config.Pin & 0x07) * 4));  
}

uint32_t leer_pin_state(volatile GPIO_TypeDef *GPIOx, uint32_t pin){
  return (GPIOx->IDR & (1U<<pin)) ? 1 : 0; //leo el estado del pin del puerto GPIOx
}

void escribir_pin_state(volatile GPIO_TypeDef *GPIOx, uint32_t pin, uint32_t state){
  if(state){
    GPIOx->ODR |= (1U<<pin); //escribo el estado del pin del puerto GPIOx
  } else {
    GPIOx->ODR &= ~(1U<<pin); //escribo el estado del pin del puerto GPIOx
  }
}

void toggle_pin_state(volatile GPIO_TypeDef *GPIOx, uint32_t pin){
    GPIOx->ODR ^= (1U<<pin); //alterno el estado del pin del puerto GPIOx
}
