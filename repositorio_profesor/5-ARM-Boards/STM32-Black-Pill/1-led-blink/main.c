#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PERIPHERAL_BASE (0x40000000U) // Peripheral base address for STM32F4 reference manual  pg. 38
#define AHB1_BASE (PERIPHERAL_BASE + 0x20000U) // Peripheral base address for STM32F4 reference manual  pg. 38

#define GPIOA_BASE (AHB1_BASE) // Peripheral base address for STM32F4 reference manual  pg. 38
#define GPIOC_BASE (AHB1_BASE + 0x800U) // Peripheral base address for STM32F4 reference manual  pg. 38
#define RCC_BASE   (AHB1_BASE + 0x3800U) // Peripheral base address for STM32F4 reference manual  pg. 38

#define RCC_AHB1ENR ((volatile uint32_t*) (RCC_BASE + 0x30u)) // Peripheral base address for STM32F4 reference manual  pg. 137
#define RCC_AHB1ENR_GPIOCEN (0x2) // Enable GPIOC clock in RCC_AHB1ENR register (bit 2) for STM32F4 reference manual  pg. 137

#define GPIOC_MODER ((volatile uint32_t*) (GPIOC_BASE)) // Peripheral base address for STM32F4 reference manual  pg. 164
#define GPIOC_MODER_MODER13 (2*13U) // Set GPIOC pin 13 as output in GPIOC_MODER register (bits 26-27) for STM32F4 reference manual  pg. 164

#define GPIOC_ODR ((volatile uint32_t*) (GPIOC_BASE + 0x14u)) // Peripheral base address for STM32F4 reference manual  pg. 164

#define LED_PIN 13 // Pin 13 corresponds to the on-board LED on the STM32-Black-Pill board


int main()
{

  *RCC_AHB1ENR |= (1 << RCC_AHB1ENR_GPIOCEN);

  // do two dummy reads after enabling the peripheral clock, as per the errata
  volatile uint32_t dummy;
  dummy = *(RCC_AHB1ENR);
  dummy = *(RCC_AHB1ENR);

  *GPIOC_MODER |= (1 << GPIOC_MODER_MODER13);

  while(1)
  {
    *GPIOC_ODR ^= (1 << LED_PIN);
    for (uint32_t i = 0; i < 500000; i++);
  }


}
