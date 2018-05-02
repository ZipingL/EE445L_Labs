// driver-sound.c
// Runs on TM4C123
// Use PWM0A/PB6 and PWM0B/PB7 to generate pulse-width modulated outputs.
// Daniel Valvan
// March 28, 2014



/*

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
#include "driver-sound.h"
#include <stdint.h>

#include "../inc/tm4c123gh6pm.h"





// period is 16-bit number of PWM clock cycles in one period (3<=period)

// period for PB6 and PB7 must be the same

// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)

// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV

//                = BusClock/2

//                = 80 MHz/2 = 40 MHz (in this example)

// Output on PB6/M0PWM0


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode


// period is 16-bit number of PWM clock cycles in one period (3<=period)
// period for PB6 and PB7 must be the same
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PB6/M0PWM0
// Allows for a high pitch sound to be played
// onto the speaker
void PWM0A_Init(uint16_t period, uint16_t duty){
	
	SYSCTL_RCGCTIMER_R |= 0x01;
	SYSCTL_RCGCGPIO_R |= 0x2;
	while((SYSCTL_PRGPIO_R&0x02) == 0){};
  GPIO_PORTB_AFSEL_R |= 0x40;           // enable alt funct on PB6
  GPIO_PORTB_DEN_R |= 0x40;
	GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xF0FFFFFF) + 0x07000000;
	TIMER0_CTL_R &= ~0x00000001;
	TIMER0_CFG_R = 0x00000004;
	TIMER0_TAMR_R = 0x0000000A;
	TIMER0_TAILR_R = period - 1;
	TIMER0_TAMATCHR_R = period-duty-1;
	TIMER0_CTL_R |= 0x00000001;
}

