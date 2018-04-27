// ADCTestMain.c
// Runs on TM4C123
// This program periodically samples ADC channel 0 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.
// Daniel Valvano
// September 5, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015

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

// center of X-ohm potentiometer connected to PE3/AIN0
// bottom of X-ohm potentiometer connected to ground
// top of X-ohm potentiometer connected to +3.3V 
#include <stdio.h>
#include <stdint.h>
#include "ADCSWTrigger.h"
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "Timer1.h"
#include "Timer2.h"
#include "Timer3.h"
#include "stdbool.h"
#include "ST7735.h"
void DumpDebugData(bool text);
void partE();

#define PF2             (*((volatile uint32_t *)0x40025010))
#define PF1             (*((volatile uint32_t *)0x40025008))
	#define PF4   (*((volatile uint32_t *)0x40025040))

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile uint32_t ADCvalue;

// global arrays
volatile uint32_t Timer0Values[1000] = {0};
volatile uint32_t ADCvalues[1000] = {0};
volatile uint32_t timeDifferences[999] = {0};
volatile uint32_t ADCPMF[4096] = {0};
uint32_t hardWareAverageVals[4] = {1, 4, 16, 64};
//volatile uint32_t ADCxValues[4096] = {0};
//global values
 volatile uint32_t timeDiff;
 volatile uint32_t timeMax;
 volatile uint32_t timeMin; 
 volatile uint32_t time_stamp_counter = 0;
	int hardware_index = -1;
  volatile  bool data_processed = false;

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


void Pause(void){
  while(PF4==0x00){ 
    DelayWait10ms(10);
  }
  while(PF4==0x10){
    DelayWait10ms(10);
  }
}

uint32_t Find_Extreme(uint32_t* array, uint32_t size, bool max){
	
	uint32_t extreme = max==true?0:0xFFFFFFFF;
	for(int i = 0; i < size; i++)
	{
		if(array[i] > extreme && max == true)
		 extreme = array[i];
		else if(array[i] < extreme && max == false)
			extreme = array[i];
	}
	
	return extreme;
}

uint32_t Process_Times() {
  timeDiff = 0;

timeMax = timeDifferences[0];
timeMin = 0xFFFFFFFF;
	//FILE *fp = fopen("timeDebug.dat", "rb");
	//printf("Time differences:\n");
	//fprintf (fp, "Time differences\n");
  for(uint32_t i = 0; i < 999; i++)
  {
    timeDifferences[i] = Timer0Values[i] - Timer0Values[i+1];
		//printf("%3d: %d\n", i, timeDifferences[i]);
		//fprintf (fp, "%3d: %d\n", i, timeDifferences[i]);
  }
  
  for(uint32_t i = 1; i < 999; i++)
  {
    if(timeDifferences[i] > timeMax)
      timeMax = timeDifferences[i];
    if(timeDifferences[i] < timeMin)
      timeMin = timeDifferences[i];
  }
  
  timeDiff = timeMax - timeMin;
	//printf("Time jitter: %d", timeDiff);
			//fprintf (fp, "Time jitter: %d", timeDiff);
  return timeDiff;
}
void Process_ADCValues() {
  for(int32_t i = 0; i < 1000; i++)
	{
		ADCPMF[ ADCvalues[i] ] += 1;
	}
}

void Process_Data() {

  
  if(!data_processed){
    Process_Times();
    Process_ADCValues();
    data_processed = true;
  }
}




// This debug function initializes Timer0A to request interrupts
// at a 1000 Hz frequency.  It is similar to FreqMeasure.c.
void Timer0A_Init100HzInt(void){
  volatile uint32_t delay;
  DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x01;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER0_TAILR_R = 799999;         // start value for 1000Hz interrupts
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R = 1<<19;              // enable interrupt 19 in NVIC
}
void Timer0A_Handler(void){

  TIMER0_ICR_R = TIMER_ICR_TATOCINT;    // acknowledge timer0A timeout
  PF2 ^= 0x04;                   // profile
  PF2 ^= 0x04;                   // profile
  //ADCvalue = ADC0_InSeq3();
	ADCvalue = 0;
	for(int i = 0; i < hardWareAverageVals[hardware_index]; i++)
	{
		ADCvalue += ADC0_InSeq3();
	}
   
	ADCvalue /= hardWareAverageVals[hardware_index];
  PF2 ^= 0x04;                   // profile
	
	  // get the time and record it
  
  if(time_stamp_counter < 1000) {

  Timer0Values[time_stamp_counter] = TIMER1_TAR_R;
  ADCvalues[time_stamp_counter++] = ADCvalue;
  }

}
int main(void){
	hardware_index = -1;
	DisableInterrupts();
  PLL_Init(Bus80MHz);                   // 80 MHz
	Output_Init();
  SYSCTL_RCGCGPIO_R |= 0x20;            // activate port F
  ADC0_InitSWTriggerSeq3_Ch9();         // allow time to finish activating
    Timer1_Init();
		//Timer2_Init(0, 8100);
		//Timer3_Init(0, 8100);
  Timer0A_Init100HzInt();               // set up Timer0A for 100 Hz interrupts
GPIO_PORTF_PUR_R |= 0x10;         // 5) pullup for PF4
  GPIO_PORTF_DIR_R |= 0x06;             // make PF2, PF1 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x16;          // disable alt funct on PF2, PF1
  GPIO_PORTF_DEN_R |= 0x16;             // enable digital I/O on PF2, PF1
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF00F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
  PF2 = 0;                      // turn off LED
	LETSDOITAGAIN:
	data_processed = false;
	hardware_index = hardware_index + 1;
	hardware_index %= 4;
	time_stamp_counter = 0;
	EnableInterrupts();
  while(time_stamp_counter < 1000){
    
	   // Parts AB
    PF1 ^= 0x02;  // toggles when running in main
		// Part C
		//GPIO_PORTF_DATA_R ^= 0x02;
		// Part D
//PF1 = (PF1*12345678) / 1234567 + 0x02; // this line causes jitter
	}
	DisableInterrupts();
	Process_Data();
	for(int i = 0; i < 4096; i++)
	{
		//ADCxValues[i] = i+1;
	}

	//ST7735_XYplot(4096, ADCxValues, ADCPMF);
  //DumpDebugData(false);

 partE();
 Pause();
 goto LETSDOITAGAIN;
}


void partE() {
			Output_Clear();
	int tens = hardWareAverageVals[hardware_index] / 10;
	int ones = hardWareAverageVals[hardware_index] %10;
	ST7735_DrawChar(0,  0, 'x', ST7735_WHITE, ST7735_BLACK,2);
	ST7735_DrawChar(tens == 0? 10:20,  0, 48+ones, ST7735_WHITE, ST7735_BLACK,2);
	if(tens != 0)
ST7735_DrawChar(0,  0, 48+tens, ST7735_WHITE, ST7735_BLACK,2);

	uint32_t x_value = 15;
	for(int i = 0; i < 4096; i++)
	{
		
		
	if(ADCPMF[i] > 0)
	{
		long long  y = 0;
		uint32_t round_y = 0;
		// calculate proportional pixel coordinate from the given x y value
		y = ((long long) ADCPMF[i]  + 0 );
		
		y *= 159*10;
		
		// round if needed

		if( (y % 10) >= 5)
			round_y = 1;

		y /= 10;
		
		y /= (0 + Find_Extreme(ADCPMF, 4096, true));
		
		y+= round_y;
		
	
		
		ST7735_DrawFastVLine(x_value++, 0, y, i%2 == 0? ST7735_MAGENTA: ST7735_CYAN);
		ST7735_DrawFastVLine(x_value++, 0, y, i%2 == 0? ST7735_MAGENTA: ST7735_CYAN);
		ST7735_DrawFastVLine(x_value++, 0, y, i%2 == 0? ST7735_MAGENTA: ST7735_CYAN);
		ST7735_DrawFastVLine(x_value++, 0, y, i%2 == 0? ST7735_MAGENTA: ST7735_CYAN);

	}
	}
}


//************* ST7735_Line********************************************
//  Draws one line on the ST7735 color LCD
//  Inputs: (x1,y1) is the start point
//          (x2,y2) is the end point
// x1,x2 are horizontal positions, columns from the left edge
//               must be less than 128
//               0 is on the left, 126 is near the right
// y1,y2 are vertical positions, rows from the top edge
//               must be less than 160
//               159 is near the wires, 0 is the side opposite the wires
//        color 16-bit color, which can be produced by ST7735_Color565() 
// Output: none
void ST7735_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, 
                 uint16_t color) {
									 
								 }


/*
void DumpDebugData(bool text) {
	
	//remove("timeDebug.dat");
	//FILE *fp = NULL;
  //if(text)
	//fp = fopen("timeDebug.dat", "rb");
	//else
	printf("Time differences:\n");
	//if(text)
	//fprintf (fp, "Time differences\n");
  for(uint32_t i = 0; i < 999; i++)
  {
		if(!text)
		printf("%3d: %d\n", i, timeDifferences[i]);
		//else
		//fprintf (fp, "%3d: %d\n", i, timeDifferences[i]);
  }
  
  		if(!text)
	printf("Time jitter: %d", timeDiff);
			//else
  //fprintf (fp, "Time jitter: %d", timeDiff); 
}*/