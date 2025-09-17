/*
 * Time.c
 *
 *  Created on: Jun 26, 2020
 *      Author: Alberto Gallegos Ramonet
 */


#include <AppHardwareApi.h>
#include <AppQueueApi.h>
#include "Time.h"

// without prescaler this will overflow in 35.79 minutes
uint32 tickCount;


PRIVATE void vTimer0Callback(uint32 u32Device, uint32 u32ItemBitmap)
{
  tickCount++;
}

PUBLIC void vInitTimer ()
{
  tickCount = 0;

  // 16 Mhz clock
  // With a 2^4 Prescaler = 16/2^4 = 1Mhz clock = 1 000 000 hz
  // therefore, 1 tick = 1/1000 000 = 1us (microsecond)

  vAHI_TimerEnable (E_AHI_TIMER_0,// Using Timer 0, (Options 0-4)
		           4,            // Prescale 2^4
		           FALSE,         // Enable interrupt at the beginning of timer (lowToHigh)
		           TRUE,          // Enable interrupt in end of timer (highToLow)
		           FALSE);        // OutputEnabled: False= Timer Mode, True= PWN or Delta Sigma Mode

  vAHI_TimerClockSelect (E_AHI_TIMER_0,
		                 FALSE,    // FALSE = Use the internal 16Mhz clock, TRUE= Use external clock
		                 TRUE);

  vAHI_Timer0RegisterCallback(vTimer0Callback);

  vAHI_TimerStartRepeat(E_AHI_TIMER_0,
  		                30000,
  		                60000);  //repeat every 60ms
}

PUBLIC uint32 vGetMicroSeconds()
{
  return (u16AHI_TimerReadCount(E_AHI_TIMER_0) + (60000 * tickCount));
}









