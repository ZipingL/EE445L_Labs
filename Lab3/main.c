/* Lab 3 Alarm Clock */

/**
The clock must be able to perform five functions. 

1) 	It will display hours and minutes in both graphical 
		and numeric forms on the LCD. The graphical output will 
		include the 12 numbers around a circle, the hour hand, 
		and the minute hand. The numerical output will be easy 
		to read. 
		
-2) 	It will allow the operator to set the current time 
		using switches or a keypad. 
		
-3) 	It will allow the operator to set the alarm time
		including enabling/disabling alarms. 
		
-4) 	It will make a sound at the alarm time. 

-5) 	It will allow the operator to stop the sound. 
		An LED heartbeat will show when the system is running.
**/

/* Hardware Connections */
// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected 
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO)
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include "ST7735.h"
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"

#include "driver-lcd.h"
#include "driver-sound.h"
#include "driver-switch.h"
#include "driver-systick.h"
#include "stdbool.h"

void DelayWait10ms(uint32_t n);
void PortF_Init(void);
#define PF2   (*((volatile uint32_t *)0x40025010))



void SystemInit(void){
}

int main(void){ 

	PLL_Init(Bus80MHz); 
	PortF_Init();
	Output_Init();
	ClockTimerInit();
	
	while(true){}
}


// Subroutine to wait 10 msec
// Inputs: None
// Outputs: None
// Notes: ...
void DelayWait10ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}

// PF4 is input
// Make PF2 an output, enable digital I/O, ensure alt. functions off
void PortF_Init(void){ 
  SYSCTL_RCGCGPIO_R |= 0x20;        // 1) activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20)==0){}; // allow time for clock to start
                                    // 2) no need to unlock PF2, PF4
  GPIO_PORTF_PCTL_R &= ~0x000F0F00; // 3) regular GPIO
  GPIO_PORTF_AMSEL_R &= ~0x14;      // 4) disable analog function on PF2, PF4
  GPIO_PORTF_PUR_R |= 0x10;         // 5) pullup for PF4
  GPIO_PORTF_DIR_R |= 0x04;         // 5) set direction to output
  GPIO_PORTF_AFSEL_R &= ~0x14;      // 6) regular port function
  GPIO_PORTF_DEN_R |= 0x14;         // 7) enable digital port
}
