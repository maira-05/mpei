#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__
#define STM32F401xE
#include "RP2350.h"
#include "stdint.h"

#define WRITE_REG_FIELD(REG, MASK, VAL) \
    (REG = (REG & ~MASK) | ((VAL << POSITION_VAL(MASK)) & MASK))

#define READ_REG_FIELD(REG, MASK) \
    ((REG & MASK) >> POSITION_VAL(MASK))


/*Declaration of two timers made by software updated every 1ms*/
typedef struct{
  unsigned int sw_tmr1_count;
  unsigned int sw_tmr2_count;
  unsigned int sw_tmr1_period;
  unsigned int sw_tmr2_period;
  unsigned int sw_tmr1_flag;
  unsigned int sw_tmr2_flag;
} SW_Timers;

void update_sw_timers(volatile SW_Timers* timer);

//System clock
#define SYSCLK 11000000 
#define MICROSECONDS_PER_SECOND 1000000


void clock_config(void);

void SysTickTimer_Delay_us(uint32_t delay);
void SysTickTimer_Delay_ms(uint32_t delay);

#endif /* __SYSTEM_CONFIG_H__ */