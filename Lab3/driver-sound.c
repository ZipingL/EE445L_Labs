// PWM.c

// Runs on TM4C123

// Use PWM0A/PB6 and PWM0B/PB7 to generate pulse-width modulated outputs.

// Daniel Valvano

// March 28, 2014



/* This example accompanies the book

   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",

   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015

   Program 6.8, section 6.3.2



   "Embedded Systems: Real-Time Operating Systems for ARM Cortex M Microcontrollers",

   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2015

   Program 8.4, Section 8.3



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

#define PWM_0_GENA_ACTCMPAD_ONE 0x000000C0  // Set the output signal to 1

#define PWM_0_GENA_ACTLOAD_ZERO 0x00000008  // Set the output signal to 0

#define PWM_0_GENB_ACTCMPBD_ONE 0x00000C00  // Set the output signal to 1

#define PWM_0_GENB_ACTLOAD_ZERO 0x00000008  // Set the output signal to 0



#define SYSCTL_RCC_USEPWMDIV    0x00100000  // Enable PWM Clock Divisor

#define SYSCTL_RCC_PWMDIV_M     0x000E0000  // PWM Unit Clock Divisor

#define SYSCTL_RCC_PWMDIV_2     0x00000000  // /2





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
void PWM0A_Init(uint16_t period, uint16_t duty){
	/*
  SYSCTL_RCGCPWM_R |= 0x01;             // 1) activate PWM0
  SYSCTL_RCGCGPIO_R |= 0x02;            // 2) activate port B
  while((SYSCTL_PRGPIO_R&0x02) == 0){};
  GPIO_PORTB_AFSEL_R |= 0x40;           // enable alt funct on PB6
  GPIO_PORTB_PCTL_R &= ~0x0F000000;     // configure PB6 as PWM0
  GPIO_PORTB_PCTL_R |= 0x04000000;
  GPIO_PORTB_AMSEL_R &= ~0x40;          // disable analog functionality on PB6
  GPIO_PORTB_DEN_R |= 0x40;             // enable digital I/O on PB6
  SYSCTL_RCC_R = 0x00100000 |           // 3) use PWM divider
      (SYSCTL_RCC_R & (~0x000E0000));   //    configure for /2 divider
  PWM0_0_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM0_0_GENA_R = 0xC8;                 // low on LOAD, high on CMPA down
  // PB6 goes low on LOAD
  // PB6 goes high on CMPA down
  PWM0_0_LOAD_R = period - 1;           // 5) cycles needed to count down to 0
  PWM0_0_CMPA_R = duty - 1;             // 6) count value when output rises
  PWM0_0_CTL_R |= 0x00000001;           // 7) start PWM0
  PWM0_ENABLE_R |= 0x00000001;          // enable PB6/M0PWM0
	*/
	
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

// change duty cycle of PB7
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM0B_Duty(uint16_t duty){
  PWM0_0_CMPB_R = duty - 1;             // 6) count value when output rises
}

// change duty cycle of PB6

// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)

void PWM0A_Duty(uint16_t duty){

  PWM0_0_CMPA_R = duty - 1;             // 6) count value when output rises}
}
