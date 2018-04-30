// driver-systick.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize the SysTick module, wait at least a
// designated number of clock cycles, and wait approximately a multiple
// of 10 milliseconds using busy wait.  After a power-on-reset, the
// LM4F120 gets its clock from the 16 MHz precision internal oscillator,
// which can vary by +/- 1% at room temperature and +/- 3% across all
// temperature ranges.  If you are using this module, you may need more
// precise timing, so it is assumed that you are using the PLL to set
// the system clock to 50 MHz.  This matters for the function
// SysTick_Wait10ms(), which will wait longer than 10 ms if the clock is
// slower.
// Daniel Valvano
// September 11, 2013

/* This example accompanies the books
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 4.7

   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Program 2.11, Section 2.6

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdint.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "driver-systick.h"

#define NVIC_ST_CTRL_COUNT      0x00010000  // Count flag
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_RELOAD_M        0x00FFFFFF  // Counter load value


int8_t hundreth_seconds_counter = 10;
int8_t seconds_counter = 30;
int8_t minutes_counter = 4;
int8_t hours_counter = 10;
uint32_t heartbeat_counter = 0;
bool ante_meridiem = false;


void SysTick_Handler(void){


	// Heartbeat every second
	if(heartbeat_counter % 500 == 0)
		GPIO_PORTF_DATA_R ^= 0x04;       // toggle PF2

	// Increment times
	if(heartbeat_counter++ % 10 == 0)
	// once hundreth seconds reaches a total of one second..
		if(++hundreth_seconds_counter % 100 == 0)
		{
			//.. increment seconds counter and reset hundreth counter
			hundreth_seconds_counter = 0;
			if(++seconds_counter % 60 == 0)
			{
				seconds_counter = 0;
				if(++minutes_counter % 60 == 0)
				{
					minutes_counter = 0;
					if( (++hours_counter-1) % 12 == 0)
					{
						hours_counter = 1;
						if(hours_counter % 12)
							ante_meridiem = !ante_meridiem;
					} // end if(++hours...
				} // end if(++minutes...
			} // end if(++seconds...
		} // end if(++ tenth_seconds...

}


void SysTick_Init(unsigned long period){
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2
  NVIC_ST_CTRL_R = 0x07; // enable SysTick with core clock and interrupts
  // enable interrupts after all initialization is finished
}


void ClockTimerInit()
{
	SysTick_Init(80000); // Every 0.001 Seconds, assuming 80MHz clock
}
