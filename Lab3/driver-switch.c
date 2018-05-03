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

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
															
char Key_Fifo[4];
uint8_t fifo_start = 0;
uint8_t fifo_end = 0;

char scanKeys(void);
char scanKeysMethod2(int * num);

char static lastKey;

// ***************** TIMER1_Init ****************
// Activate TIMER1 interrupts to run 
//          periodic keypad scanning (not used)
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none
/*
void Timer1_Init(uint32_t period){
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER1_TAILR_R = period-1;    // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
  TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00008000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 37, interrupt number 21
  NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
}*/

void scanKeyPad(void);

/*
void Timer1A_Handler(void){
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
	scanKeyPad();
}*/

// Scan the keypad for any keypresses
// and store it to the FIFO
void scanKeyPad()
{
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
/*

void DelayWait10ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
    while(time){
      time--;
    }
    n--;
  }
} */

void initKeypad()
{
  SYSCTL_RCGCGPIO_R |= 0x18;        // 1) activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x18)==0){}; // allow time for clock to start
  
  //SYSCTL_RCGCGPIO_R |= 0x18; //0b11000       // enable Ports E and D
					GPIO_PORTE_DATA_R &= ~0x1E;      // pin values high

  GPIO_PORTE_DEN_R |= 0x1E;        // enable digital I/O on PE1-4
  GPIO_PORTE_DIR_R &= ~0x1E;       // make PE1-4 IN (PE1-4 columns)
  GPIO_PORTE_PCTL_R = 0xFFF0000F;
  GPIO_PORTE_AFSEL_R = 0;     	// disable alternate functionality on PE
  GPIO_PORTE_AMSEL_R = 0;     // disable analog functionality on PE
 
  GPIO_PORTD_DEN_R |= 0x0F;        // enable digital I/O on PD3-0
  GPIO_PORTD_DIR_R &= ~0x0F;       // make PD3-0 IN (PD3-0 rows)
  GPIO_PORTD_PCTL_R = 0xFFFF0000;
  GPIO_PORTD_AMSEL_R = 0;     // disable analog functionality on PD
  GPIO_PORTD_AFSEL_R = 0;     // disable alternate functionality on PD
  //GPIO_PORTD_DATA_R &= ~0x0F;      // pin values LOW
		
  //Timer1_Init(25*80000);
		
	
  }

	// returns the columns scanned
	uint8_t scanColumns()
	{
		uint8_t column_data = GPIO_PORTD_DATA_R;
		column_data &= 0x0F;
		switch(column_data)
		{
			case 0x8: 
				return 0x8;
			case 0x4: 
				return 0x4;
			case 0x2: 
				return 0x2;
			case 0x1: 
				return 0x1;
		}
		return 0x80;
	}

	// Inputs: row_pressed represented by the pin value
	//         column_pressed represented by the pin value
	//         e.g. row_pressed = 0x02 would be pin PE2
	//         e.g. column_pressed = 0x01 would be pin PD0
	//         therefore representing keypress:'D'
	//
	// Outputs: The character based on the row column coordinates
	// 
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
// Inputs: num to store number of presses
// Assumes that keypad has its columns
// connected to pull up resistors
// and 3.3 vcc.
// therefore all the pins (PD0-3) 
// connected to columns of keypad will read 1,
// and, all the pins connected to the rows 
// will read  0.
// How a keypad works:
// When a user presses a key
// the column and row of that key
// and instantly connected
// therefore the column pin will read
// 0 instead of its previous value 1
// since the row pins are grounded
// 
char scanKeysMethod2(int* num)
{
	char key = 0;
	char column = 0;
	DisableInterrupts();
	// Scan through each row for the key
	for(int i = 0x02; i != 0x20; i <<=1)
	{
		// make a pin as OUT
		GPIO_PORTE_DIR_R = i;
		// output 0 to the pin,
		// therefore grounding it
		GPIO_PORTE_DATA_R &= ~0x1E;
		// short delay, wait for signal to settle
		for(int i = 0; i <=100; i++);
		// read the column pins
		column = GPIO_PORTD_DATA_R & 0x0F;
		// in the case that there is one keypress
		// one of the column pins should be
		// 0, due to the connection made
		// with the grounded row
		// in the case that there is a keypress
		for(int j = 1; j != 0x10; j <<= 1)
		{
			// if one of the column pins is 0...
			if((column & j) == 0)
			{
				// ...then we found the keypress!
				GPIO_PORTF_DATA_R ^= 0x04;
				if(key == 0)
				key = parseKey(i,j);
				(*num)++;
			}
		}
	}
	GPIO_PORTE_DIR_R &= ~0x1E;
	EnableInterrupts();
		return key;
}

// Doesn't work nearly as well, doesn't require VCC to columns
/*
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
*/

// Pop a key from the FIFO
// Returns NULL if FIFO is empty
// Can be modified with a while loop
// to wait for user keypress 
// but not needed for this lab
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
