// driver-switch.c
// Runs on  LM4F120/TM4C123
// Provide functions that initialize GPIO ports and SysTick 
// Use periodic polling
// Daniel Valvano
// modified by ziping liu
// april 30, 2018

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014

   Example 5.4, Figure 5.18, Program 5.13

Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

/** Hardware Connections **/

/* 4x4 Matrix layout: 
https://cdn.instructables.com/F1U/U8SR/IFF8A3BR/F1UU8SRIFF8A3BR.LARGE.jpg

// Rows    //
keypad 
pin     
8 			<-wire-> PE4 
7 			<------> PE3
6 			<------> PE2 
5 			<------> PE1 

// Columns //
keypad
pin
4 			<------> PD3 <--->10K Ohm<---> 3.3 VCC
3 			<------> PD2 <--->10K Ohm<---> 3.3 VCC
2 			<------> PD1 <--->10K Ohm<---> 3.3 VCC
1 			<------> PD0 <--->10K Ohm<---> 3.3 VCC




*/

#include <stdint.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "driver-switch.h"
#include "ST7735.h"
//#include "FIFO.h"

#define PE4 0x10; //0b00010000 
#define PE3 0x08; //0b00001000 
#define PE2 0x04; //0b00000100 
#define PE1 0x02; //0b00000010

#define FIFOSIZE   16         // size of the FIFOs (must be power of 2)
#define FIFOSUCCESS 1         // return value on success
#define FIFOFAIL    0         // return value on failure
                              // create index implementation FIFO (see FIFO.h)
															
															
char Key_Fifo[4];
uint8_t fifo_start = 0;
uint8_t fifo_end = 0;

char scanKeys(void);
char scanKeysMethod2(int * num);

char static lastKey;
// Records the time and adc value every 1000 Hz
void Timer0A_Handler(void){

  TIMER0_ICR_R = TIMER_ICR_TATOCINT;    // acknowledge timer0A timeout
	int keypresses = 0;
	char thisKey = scanKeysMethod2(&keypresses);
	if(thisKey != lastKey && keypresses <= 1 )
	{
		Key_Fifo[fifo_end] = thisKey;
		fifo_end = (++fifo_end) % 4;
		lastKey = thisKey;
	}
	else{
		lastKey = 0;
	}
}

// This debug function initializes Timer0A to request interrupts
// at a 1000 Hz frequency.  It is similar to FreqMeasure.c.
void Timer0A_Init(void){
  volatile uint32_t delay;
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x01;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER0_TAILR_R = 25*80000;         // 25 ms polling
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R = 1<<19;              // enable interrupt 19 in NVIC
	
}

void DelayWait10ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
    while(time){
      time--;
    }
    n--;
  }
}

void initKeypad()
{
  //SYSCTL_RCGCGPIO_R |= 0x18;        // 1) activate clock for Port F
  //while((SYSCTL_PRGPIO_R&0x18)==0){}; // allow time for clock to start
  
  SYSCTL_RCGCGPIO_R |= 0x18; //0b11000       // enable Ports E and D
					GPIO_PORTE_DATA_R &= ~0x1E;      // pin values high

  GPIO_PORTE_DEN_R |= 0x1E;        // enable digital I/O on PE1-4
  GPIO_PORTE_DIR_R &= ~0x1E;       // make PE1-4 OUT (PE1-4 columns)
  GPIO_PORTE_PCTL_R = 0xFFF0000F;
  GPIO_PORTE_AFSEL_R = 0;     // disable alternate functionality on PE
  GPIO_PORTE_AMSEL_R = 0;     // disable analog functionality on PE
 
  GPIO_PORTD_DEN_R |= 0x0F;        // enable digital I/O on PD3-0
  GPIO_PORTD_DIR_R &= ~0x0F;       // make PD3-0 IN (PD3-0 rows)
  GPIO_PORTD_PCTL_R = 0xFFFF0000;
  GPIO_PORTD_AMSEL_R = 0;     // disable analog functionality on PD
  GPIO_PORTD_AFSEL_R = 0;     // disable alternate functionality on PD
	//GPIO_PORTD_DATA_R &= ~0x0F;      // pin values LOW
		
	Timer0A_Init();
		
	
	}

// returns the columns scanned
uint8_t scanColumns()
{
	

	uint8_t column_data = GPIO_PORTD_DATA_R;
	column_data &= 0x0F;
	switch(column_data)
	{
		case 0x8: // 0b00001110
			return 0x8;
		case 0x4: // 0b00010110 
			return 0x4;
		case 0x2: // 0b00011010 
			return 0x2;
		case 0x1: // 0b00011100 
			return 0x1;
	}


	return 0x80;
}

char parseKey(uint8_t row_pressed, uint8_t column_pressed)
{
	switch(row_pressed)
	{
		case 0x10:
			switch(column_pressed)
			{
				case 0x8:
					return '1';
				case 0x4:
					return '2';
				case 0x2:
					return '3';
				case 0x1:
					return 'A';
			}
		case 0x8:
			switch(column_pressed)
			{
				case 0x8:
					return '4';
				case 0x4:
					return '5';
				case 0x2:
					return '6';
				case 0x1:
					return 'B';
			}
		case 0x4:
			switch(column_pressed)
			{
				case 0x8:
					return '7';
				case 0x4:
					return '8';
				case 0x2:
					return '9';
				case 0x1:
					return 'C';
			}
		case 0x2:
			switch(column_pressed)
			{
				case 0x8:
					return '*';
				case 0x4:
					return '0';
				case 0x2:
					return '#';
				case 0x1:
					return 'D';
			}
		default:
			return 0;
					
	}
}

// based on Matrix.c
char scanKeysMethod2(int* num)
{
	char key = 0;
 char column = 0;
	for(int i = 0x02; i != 0x20; i <<=1)
	{
		GPIO_PORTE_DIR_R = i;
		GPIO_PORTE_DATA_R &= ~0x1E;
		for(int i = 0; i <=100; i++); // delay
		column = GPIO_PORTD_DATA_R & 0x0F;
		for(int j = 1; j != 0x10; j <<= 1)
		{
			if((column & j) == 0)
			{
				GPIO_PORTF_DATA_R ^= 0x04;
				if(key == 0)
				key = parseKey(i,j);
				(*num)++;
			}
		}
	}
	GPIO_PORTE_DIR_R &= ~0x1E;

		return key;
}

// Doesn't work nearly as well, doesn't require VCC to columns
char scanKeys()
{

	uint8_t column_pressed = scanColumns();
	uint8_t row_pressed = 0x80;
	//ST7735_DrawChar(50, 0, column_pressed + 48, ST7735_GREEN, ST7735_BLACK, 2);
	if(column_pressed != 0x80)
	{
		for( uint8_t i = 0x02; i != 0x20; i<<=1)
		{
			GPIO_PORTF_DATA_R ^= 0x04;
			GPIO_PORTE_DATA_R &= ~i;
			DelayWait10ms(1);
			uint8_t data = GPIO_PORTD_DATA_R;
			data = data + 0;
			if( (~data & column_pressed) ==      column_pressed )
			{
				row_pressed = i;
				//ST7735_DrawChar(62, 0, row_pressed + 48, ST7735_GREEN, ST7735_BLACK, 2);
				GPIO_PORTE_DATA_R = 0x1E;

				break;
			}

		}
	}

	if(row_pressed != 0x80)
	{

		return parseKey(row_pressed, column_pressed);
	}
	else
	{
						GPIO_PORTE_DATA_R = 0x1E;
		return 0;
	}
}

char getKey()
{
	char letter = 0;
	if(fifo_start != fifo_end)
	{
	letter = Key_Fifo[fifo_start];
	fifo_start = (++fifo_start) % 4;
	}
	return letter;
}
