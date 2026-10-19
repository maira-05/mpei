/******************************************************************************************
  Filename    : SysTickTimer.h
  
  Core        : ARM Cortex-M33 / RISC-V Hazard3
  
  MCU         : RP2350
    
  Author      : Chalandi Amine
 
  Owner       : Chalandi Amine
  
  Date        : 04.09.2024
  
  Description : System timer driver header file
  
******************************************************************************************/

#ifndef __SYSTICK_TIMER_H__
#define __SYSTICK_TIMER_H__

#include "Platform_Types.h"
#include  "stdint.h"
#include "system_config.h"



//=========================================================================================
// Prototypes
//=========================================================================================
void SysTickTimer_Init(uint32_t us);
void SysTickTimer_Start(void);
void SysTickTimer_Stop(void);
void SysTickTimer_Reload(uint32 timeout);
void SysTickTimer_IRQ_Enable(char state);

#endif /*__SYSTICK_TIMER_H__*/
