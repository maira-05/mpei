/******************************************************************************************
  Filename    : SysTickTimer.c
  
  Core        : ARM Cortex-M33 / RISC-V Hazard3
  
  MCU         : RP2350
    
  Author      : Chalandi Amine
 
  Owner       : Chalandi Amine
  
  Date        : 04.09.2024
  
  Description : System timer driver implementation
  
******************************************************************************************/

#include "SysTickTimer.h"

//=========================================================================================
// Functions
//=========================================================================================

//-----------------------------------------------------------------------------
/// \brief
///
/// \descr
///
/// \param
///
/// \return
//-----------------------------------------------------------------------------


void SysTickTimer_Init(uint32_t us){
    uint32_t period;
    period = (SYSCLK / MICROSECONDS_PER_SECOND) * us - 1;
    SysTick->CTRL = 0; // Disable SysTick
    SysTick->LOAD = period; // Set reload value to maximum (24-bit)
    SysTick->VAL = 0; // Clear current value
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk; // Enable SysTick with processor clock
}

//-----------------------------------------------------------------------------
/// \brief
///
/// \descr
///
/// \param
///
/// \return
//-----------------------------------------------------------------------------
void SysTickTimer_Start(void)
{
  //SysTick->LOAD    = timeout;
  SysTick->VAL     = 0;
  SysTick->CTRL    |= SysTick_CTRL_ENABLE_Msk;
}

//-----------------------------------------------------------------------------
/// \brief
///
/// \descr
///
/// \param
///
/// \return
//-----------------------------------------------------------------------------
void SysTickTimer_Reload(uint32 timeout)
{
  SysTick->VAL     = 0;
  SysTick->LOAD    = timeout;
}

//-----------------------------------------------------------------------------
/// \brief
///
/// \descr
///
/// \param
///
/// \return
//-----------------------------------------------------------------------------
void SysTickTimer_Stop(void)
{
  SysTick->CTRL    &= ~SysTick_CTRL_ENABLE_Msk;
}


