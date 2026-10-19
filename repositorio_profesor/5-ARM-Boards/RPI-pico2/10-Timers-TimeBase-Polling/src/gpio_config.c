#include "gpio_config.h"


void GPIO_Config(GPIO_InitTypeDef * GPIO_info)
{
    if (GPIO_info->Mode == OUTPUT) {
        GPIO_SET_DIRECTION_OUTPUT(GPIO_info->Pin);
    } else {
        GPIO_SET_INPUT(GPIO_info->Pin);
        GPIO_SET_INPUT_PULLNONE(GPIO_info->Pin);
        if (GPIO_info->PullUp) {
            GPIO_SET_INPUT_PULLUP(GPIO_info->Pin);
        } else if (GPIO_info->PullDown) {
            GPIO_SET_INPUT_PULLDOWN(GPIO_info->Pin);
        } else {
            GPIO_SET_INPUT_PULLNONE(GPIO_info->Pin);
        }
    }
    GPIO_SET_FUNCTION(GPIO_info->Pin, GPIO_info->Funct);
}

uint32_t GPIO_ReadPin_state(uint32_t pin){
    return GPIO_READ_PIN(pin);
}

void GPIO_WritePin_state(uint32_t pin, uint32_t value){
    if(value) { 
        GPIO_SET_OUTPUT_HIGH(pin); 
    } else { 
        GPIO_SET_OUTPUT_LOW(pin); 
    }
}

void GPIO_TogglePin_state(uint32_t pin){
    GPIO_SET_OUTPUT_XOR(pin);
}