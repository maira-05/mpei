#include <stdint.h>
#include "stm32f401.h"
#include "gpio_config.h"
#include "display.h"



int main(void)
{
    GPIO_Config();     
    Display_Init();    
    int variable =0;
    int variable2 =1;
    int resultado = 0;
    while(1)
    {
        resultado= variable + variable2;
        
        uint8_t centenas = resultado / 100;
        uint8_t decenas  = (resultado / 10) % 10;
        uint8_t unidades = resultado % 10;
        variable=variable2;
        variable2=resultado;

        Display_Refresh(centenas, decenas, unidades);
        
    }
}