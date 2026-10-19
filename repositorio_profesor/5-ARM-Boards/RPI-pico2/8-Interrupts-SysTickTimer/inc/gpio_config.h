#ifndef __RP2350_GPIO_CONFIG_H__
#define __RP2350_GPIO_CONFIG_H__
#include "Platform_Types.h"
#include "RP2350.h"
#include "stdint.h"

#define INPUT 0
#define OUTPUT 1



typedef struct {
      __IOM uint32_t SLEWFAST   : 1;            /*!< Slew rate control. 1 = Fast, 0 = Slow                                     */
      __IOM uint32_t SCHMITT    : 1;            /*!< Enable schmitt trigger                                                    */
      __IOM uint32_t PDE        : 1;            /*!< Pull down enable                                                          */
      __IOM uint32_t PUE        : 1;            /*!< Pull up enable                                                            */
      __IOM uint32_t DRIVE      : 2;            /*!< Drive strength.                                                           */
      __IOM uint32_t IE         : 1;            /*!< Input enable                                                              */
      __IOM uint32_t OD         : 1;            /*!< Output disable. Has priority over output enable from peripherals          */
      __IOM uint32_t ISO        : 1;            /*!< Pad isolation control. Remove this once the pad is configured
                                                     by software.                                                              */
            uint32_t            : 23;
    } GPIO_PADS_TypeDef;

typedef struct {
      __IOM uint32_t FUNCSEL    : 5;            /*!< 0-31 -> selects pin function according to the gpio table 31
                                                     == NULL                                                                   */
            uint32_t            : 7;
      __IOM uint32_t OUTOVER    : 2;            /*!< OUTOVER                                                                   */
      __IOM uint32_t OEOVER     : 2;            /*!< OEOVER                                                                    */
      __IOM uint32_t INOVER     : 2;            /*!< INOVER                                                                    */
            uint32_t            : 10;
      __IOM uint32_t IRQOVER    : 2;            /*!< IRQOVER                                                                   */
            uint32_t            : 2;
    } GPIO_CTRL_TypeDef;
//=============================================================================
// Macros
//=============================================================================
#define GPIO_SET_DIRECTION_OUTPUT(pin)  HW_PER_SIO->GPIO_OE_CLR.bit.GPIO_OE_CLR |= 1UL<<pin; HW_PER_SIO->GPIO_OUT_CLR.bit.GPIO_OUT_CLR |= 1UL<<pin; \
                                        HW_PER_SIO->GPIO_OE_SET.bit.GPIO_OE_SET |= 1UL<<pin; ((volatile GPIO_PADS_TypeDef *)(PADS_BANK0_BASE+(pin+1)*4))->ISO = 0u

#define PIO_SET_DIRECTION_OUTPUT(id, pin) HW_PER_SIO->GPIO_OE_CLR.bit.GPIO_OE_CLR |= 1UL<<pin; HW_PER_SIO->GPIO_OUT_CLR.bit.GPIO_OUT_CLR |= 1UL<<pin;\
                                          ((volatile GPIO_CTRL_TypeDef *)(IO_BANK0_BASE+(2*pin+1)*4))->FUNCSEL = IO_BANK0_GPIO##pin##_CTRL_FUNCSEL_pio##id##_##pin; \
                                          HW_PER_SIO->GPIO_OE_SET.bit.GPIO_OE_SET |= 1UL<<pin


#define GPIO_SET_OUTPUT_HIGH(pin)       HW_PER_SIO->GPIO_OUT_SET.bit.GPIO_OUT_SET |= 1UL<<pin
#define GPIO_SET_OUTPUT_LOW(pin)        HW_PER_SIO->GPIO_OUT_CLR.bit.GPIO_OUT_CLR |= 1UL<<pin
#define GPIO_SET_OUTPUT_XOR(pin)        HW_PER_SIO->GPIO_OUT_XOR.bit.GPIO_OUT_XOR |= 1UL<<pin

#define GPIO_SET_INPUT(pin)                 HW_PER_SIO->GPIO_OE_CLR.bit.GPIO_OE_CLR |= 1UL<<pin; ((volatile GPIO_PADS_TypeDef *)(PADS_BANK0_BASE+(pin+1)*4))->ISO = 1u
#define GPIO_SET_INPUT_PULLUP(pin)          HW_PER_SIO->GPIO_OE_CLR.bit.GPIO_OE_CLR |= 1UL<<pin; ((volatile GPIO_PADS_TypeDef *)(PADS_BANK0_BASE+(pin+1)*4))->ISO = 1u; ((volatile GPIO_PADS_TypeDef *)(PADS_BANK0_BASE+(pin+1)*4))->PUE = 1u
#define GPIO_SET_INPUT_PULLDOWN(pin)        HW_PER_SIO->GPIO_OE_CLR.bit.GPIO_OE_CLR |= 1UL<<pin; ((volatile GPIO_PADS_TypeDef *)(PADS_BANK0_BASE+(pin+1)*4))->ISO = 1u; ((volatile GPIO_PADS_TypeDef *)(PADS_BANK0_BASE+(pin+1)*4))->PDE = 1u
#define GPIO_SET_INPUT_PULLNONE(pin)        HW_PER_SIO->GPIO_OE_CLR.bit.GPIO_OE_CLR |= 1UL<<pin; ((volatile GPIO_PADS_TypeDef *)(PADS_BANK0_BASE+(pin+1)*4))->ISO = 1u; ((volatile GPIO_PADS_TypeDef *)(PADS_BANK0_BASE+(pin+1)*4))->PUE = 0u; ((volatile GPIO_PADS_TypeDef *)(PADS_BANK0_BASE+(pin+1)*4))->PDE = 0u

#define GPIO_SET_FUNCTION(pin, funsel)      ((volatile GPIO_CTRL_TypeDef *)(IO_BANK0_BASE+(2*pin+1)*4))->FUNCSEL = funsel

//#define GPIO_SET_FUNCTION(pin, funsel)      HW_PER_IO_BANK0->GPIO##pin##_CTRL.bit.FUNCSEL = funsel

//=============================================================================
// Defines
//=============================================================================

//#define GPIO_WRITE_PIN(pin, value)  if(value) { GPIO_SET_OUTPUT_HIGH(pin); } else { GPIO_SET_OUTPUT_LOW(pin); }
#define GPIO_READ_PIN(pin)           (HW_PER_SIO->GPIO_IN.bit.GPIO_IN & (1UL<<pin)) ? 1 : 0 


typedef struct {
    uint32_t Mode;
    uint32_t Pin;
    uint32_t PullUp;
    uint32_t PullDown;
    uint32_t Funct;
}GPIO_InitTypeDef;

void GPIO_Config(GPIO_InitTypeDef * GPIO_info);

uint32_t GPIO_ReadPin_state(uint32_t pin);

void GPIO_WritePin_state(uint32_t pin, uint32_t value);

void GPIO_TogglePin_state(uint32_t pin);



#endif /* __RP2350_GPIO_H__ */