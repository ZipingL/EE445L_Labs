#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include "fixed.h"
#include <stdbool.h>
#include "ST7735.h"

int32_t saved_minX;
int32_t saved_maxX;
int32_t saved_minY;
int32_t saved_maxY;
#define screen_maxX 127
#define screen_maxY 127
#define screen_yBuffer 30
#define screen_maxCharWidth 21



/****************ST7735_sDecOut2***************
 converts fixed point number to LCD
 format signed 32-bit with resolution 0.01
 range -99.99 to +99.99
 Inputs:  signed 32-bit integer part of fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
 12345    " **.**"
  2345    " 23.45"  
 -8100    "-81.00"
  -102    " -1.02" 
    31    "  0.31" 
-12345    "-**.**"
 */ 
void ST7735_sDecOut2(int32_t n) {
	
  int32_t n_array[4];
	bool positive = true;
	// Check if input is valid
	if( n >= 10000)
	{
		printf("**.**");
		return;
	}
	else if (n <= -10000)
	{
		printf("-**.**");
		return;
	}
	
	// determine if n is negative 
	if (n < 0)
	{
		positive = false;
		n = n* -1;
	}
	
	// valid input, so print to LCD the number
	n_array[3] = n / 1000;
	n_array[2] = (n % 1000) / 100;
	n_array[1] = (n % 100) / 10;
	n_array[0] = (n % 10) / 1;

	if(positive == false)
	{
		printf("-");
	}
	
	for(int32_t i = 3; i > -1; i--)
	{
		if( i == 1)
			printf(".");
		
		if(i == 3 && n_array[i] == 0)
		{
			continue;
		}
			printf("%d", n_array[i]);


	}
		
	


		
		
}

/**************ST7735_uBinOut6***************
 unsigned 32-bit binary fixed-point with a resolution of 1/64. 
 The full-scale range is from 0 to 999.99. 
 If the integer part is larger than 63999, it signifies an error. 
 The ST7735_uBinOut6 function takes an unsigned 32-bit integer part 
 of the binary fixed-point number and outputs the fixed-point value on the LCD
 Inputs:  unsigned 32-bit integer part of binary fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
     0	  "  0.00"
     1	  "  0.01"
    16    "  0.25"
    25	  "  0.39"
   125	  "  1.95"
   128	  "  2.00"
  1250	  " 19.53"
  7500	  "117.19"
 63999	  "999.99"
 64000	  "***.**"
*/
void ST7735_uBinOut6(uint32_t n) {
	
		
  int32_t n_array[5];
	int32_t round_off = 0;
	unsigned long long big_n = n;
	
	// Check if input is valid
	if( n >= 64000)
	{
		printf("***.**");
		return;
	}
	
	// check if rounding needed
	big_n = ((big_n* 1000000) / 64000) % 10;
	if(big_n >= 5)
		round_off = 1;
	
	// valid input, so print to LCD the number
	big_n = n;
	big_n = (big_n*100000) / 64000;
	
	n_array[4] =  big_n / 10000;
	n_array[3] = (big_n % 10000) / 1000;
	n_array[2] = (big_n % 1000) / 100;
	n_array[1] = (big_n % 100) / 10;
	n_array[0] = (big_n % 10) / 1;

	if(n_array[0] != 0)
		n_array[0] += round_off;
	
	for(int32_t i = 4; i >= 0; i--)
	{
		if( i == 1)
			printf(".");
		
		if((i == 4 && n_array[i] == 0) ||
			 (i == 3 && n_array[i] == 0))
		{
			continue;
		}
		

			printf("%d", n_array[i]);

	}
		
	

}

/**************ST7735_XYplotInit***************
 Specify the X and Y axes for an x-y scatter plot
 Draw the title and clear the plot area
 Inputs:  title  ASCII string to label the plot, null-termination
          minX   smallest X data value allowed, resolution= 0.001
          maxX   largest X data value allowed, resolution= 0.001
          minY   smallest Y data value allowed, resolution= 0.001
          maxY   largest Y data value allowed, resolution= 0.001
 Outputs: none
 assumes minX < maxX, and miny < maxY
*/
void ST7735_XYplotInit(char *title, int32_t minX, int32_t maxX, 
	int32_t minY, int32_t maxY){
		
		size_t title_len = 0;
		
		// check if within resolution of 0.001
		if(minX <= -100000)
			return;
		if(maxX >= 100000)
			return;
		if(minY <= -100000)
			return;
		if(maxY >= 100000)
			return;
		
		// center to printing of the title
		title_len = strlen(title);
		size_t center_text = (screen_maxCharWidth - title_len) / 2;
		
		Output_Clear();
		ST7735_SetCursor(center_text,0);
		printf("%s\n", title);
		saved_minX = minX;
		saved_maxX = maxX;
		saved_minY = minY;
		saved_maxY = maxY;
		
	}
	
	/**************ST7735_XYplot***************
 Plot an array of (x,y) data
 Inputs:  num    number of data points in the two arrays
          bufX   array of 32-bit fixed-point data, resolution= 0.001
          bufY   array of 32-bit fixed-point data, resolution= 0.001
 Outputs: none
 assumes ST7735_XYplotInit has been previously called
 neglect any points outside the minX maxY minY maxY bounds
*/
void ST7735_XYplot(uint32_t num, int32_t bufX[], int32_t bufY[]){
	
	long long y;
	long long x;
	int32_t positive_minX = 0;
	int32_t positive_minY = 0;
	int32_t negative_modifier = -1;
	int32_t round_x = 0;
	int32_t round_y = 0;
	
  // find a flipped value of the minY
	// used for sake of taking into account how the screen
	// has pixels only from 0 to max, even though x or y 
	// values can be negative
		positive_minX = saved_minX *negative_modifier;
		positive_minY = saved_minY *negative_modifier;
	
	ST7735_FillRect(0, screen_yBuffer, screen_maxX, screen_maxY, ST7735_MAGENTA);
	
	// plot the points
	for( int i = 0; i < num; i++)
	{
		round_x = 0;
		round_y = 0;
		
		// check if within min max values
		if(bufX[i] > saved_maxX)
			continue;
		if(bufX[i] < saved_minX)
			continue;
		
		if(bufY[i] > saved_maxY)
			continue;
		if(bufY[i] < saved_minY)
			continue;
		

		// calculate proportional pixel coordinate from the given x y value
		x = ((long long) bufX[i]  + positive_minX );
		y = (long long) bufY[i]  + positive_minY;
		
		x *= screen_maxX*10;
		y *= screen_maxY*10;
		
		// round if needed
		if( (x % 10) >= 5)
			round_x = 1;
		if( (y % 10) >= 5)
			round_y = 1;
		x /= 10;
		y /= 10;
		
		x /= (positive_minX + saved_maxX);
		y /= (positive_minY + saved_maxY);
		
    x+= round_x;
		y+= round_y;
		
		// if we don't do this the plot will be upside down
		y = (screen_maxY - y) + screen_yBuffer;
		
		// Draw 4 pixels for each x,y coord
		ST7735_DrawPixel (x, y, ST7735_GREEN);
		ST7735_DrawPixel (x+1<=screen_maxX? x+1: x-1, 
											y+1<=screen_maxY? y+1: y-1, ST7735_GREEN);
	  ST7735_DrawPixel (x+1<=screen_maxX? x+1: x-1, y, ST7735_GREEN);
		ST7735_DrawPixel (x, y+1<=screen_maxY? y+1: y-1, ST7735_GREEN);
	}
		
}
