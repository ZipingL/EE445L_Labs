#include <stdio.h>
#include <stdint.h>
#include "ST7735.h"
#include "../inc/tm4c123gh6pm.h"
#include "stdbool.h"
#include "driver-lcd.h"


#define CHAR_PIXEL_LEN 12
#define CHAR_PIXEL_LEN_SMALL 6
#define DIGITAL_DISPLAY_HEIGHT -6

char  s_AM[3] = {'A', 'M', 0};
char  s_PM[3] = {'P', 'M', 0};

char* time_to_string(int8_t sec, int8_t min, int8_t hour, int8_t hundreth_sec, char* string);

void draw_digital_time_edit( int8_t sec, int8_t min, int8_t hour, bool AM, int color, int place, char edit)
{
	int8_t hundreth_sec = -1;
	char string[12];
	char * s_am_pm = AM == true ? s_AM : s_PM;
	
	if(place == 0)
	for(int i = 0; s_am_pm[i] != NULL; i++)
	{
			ST7735_DrawChar(64 - 4*CHAR_PIXEL_LEN + i*CHAR_PIXEL_LEN_SMALL, 76 + DIGITAL_DISPLAY_HEIGHT, edit, 
			color, ST7735_BLACK, 1);
	}
	else 
	for(int i = 0; s_am_pm[i] != NULL; i++)
	{
			ST7735_DrawChar(64 - 4*CHAR_PIXEL_LEN + i*CHAR_PIXEL_LEN_SMALL, 76 + DIGITAL_DISPLAY_HEIGHT, s_am_pm[i], 
			color, ST7735_BLACK, 1);
	}
	
	time_to_string(sec, min, hour, hundreth_sec, string);
	
	for(int i = 0; i < 3; i++)
	{
		if(place == (i+1) && i != 2)
		ST7735_DrawChar(64 - 5*CHAR_PIXEL_LEN_SMALL + i*CHAR_PIXEL_LEN, 76 + DIGITAL_DISPLAY_HEIGHT, edit, 
			color, ST7735_BLACK, 2);
		else
		ST7735_DrawChar(64 - 5*CHAR_PIXEL_LEN_SMALL + i*CHAR_PIXEL_LEN, 76 + DIGITAL_DISPLAY_HEIGHT, string[i], 
			color, ST7735_BLACK, 2);
	}
  for(int i = 3; i < 5; i++)
	{
		if(place == (i))
		ST7735_DrawChar(64 - 5*CHAR_PIXEL_LEN_SMALL + i*CHAR_PIXEL_LEN, 76 + DIGITAL_DISPLAY_HEIGHT, edit, 
			color, ST7735_BLACK, 2);
		else
		ST7735_DrawChar(64 - 5*CHAR_PIXEL_LEN_SMALL + i*CHAR_PIXEL_LEN, 76 + DIGITAL_DISPLAY_HEIGHT, string[i], 
			color, ST7735_BLACK, 2);
	}
	
	for(int i = 5; string[i] != NULL; i++)
	{
		if(place == i)
		ST7735_DrawChar(64 - 2*CHAR_PIXEL_LEN + 4*CHAR_PIXEL_LEN + (i-3)*CHAR_PIXEL_LEN_SMALL, 83 + DIGITAL_DISPLAY_HEIGHT, 
		edit, color, ST7735_BLACK, 1);
		else
		{
		ST7735_DrawChar(64 - 2*CHAR_PIXEL_LEN + 4*CHAR_PIXEL_LEN + (i-3)*CHAR_PIXEL_LEN_SMALL, 83 + DIGITAL_DISPLAY_HEIGHT, 
		string[i], color, ST7735_BLACK, 1);
		}
	}
}


void draw_digital_time_hundreth( int8_t sec, int8_t min, int8_t hour, int8_t hundreth_sec, bool AM, int color)
{
	char string[12];
	char * s_am_pm = AM == true ? s_AM : s_PM;
	for(int i = 0; s_am_pm[i] != NULL; i++)
	{
			ST7735_DrawChar(64 - 4*CHAR_PIXEL_LEN + i*CHAR_PIXEL_LEN_SMALL, 76 + DIGITAL_DISPLAY_HEIGHT, s_am_pm[i], 
			color, ST7735_BLACK, 1);
	}
	
	time_to_string(sec, min, hour, hundreth_sec, string);
	
	for(int i = 0; i < 5; i++)
	{
		ST7735_DrawChar(64 - 5*CHAR_PIXEL_LEN_SMALL + i*CHAR_PIXEL_LEN, 76 + DIGITAL_DISPLAY_HEIGHT, string[i], 
			color, ST7735_BLACK, 2);
	}
	for(int i = 5; string[i] != NULL; i++)
	{
		ST7735_DrawChar(64 - 2*CHAR_PIXEL_LEN + 4*CHAR_PIXEL_LEN + (i-3)*CHAR_PIXEL_LEN_SMALL, 83 + DIGITAL_DISPLAY_HEIGHT, 
		string[i], color, ST7735_BLACK, 1);
	}
}

void cover_digital_time(int8_t place)
{
	switch(place)
	{
		case 0:
		{
			ST7735_FillRect(64-4*CHAR_PIXEL_LEN, 76+DIGITAL_DISPLAY_HEIGHT, CHAR_PIXEL_LEN_SMALL*2, CHAR_PIXEL_LEN_SMALL+1, ST7735_BLACK);
			break;
		}
		case 1:
		{
		  ST7735_FillRect(64 - 5*CHAR_PIXEL_LEN_SMALL, 76 + DIGITAL_DISPLAY_HEIGHT, CHAR_PIXEL_LEN, CHAR_PIXEL_LEN+2, ST7735_BLACK);
			break;
		}
		case 2:
		{
		  ST7735_FillRect(64 - 5*CHAR_PIXEL_LEN_SMALL + CHAR_PIXEL_LEN, 76 + DIGITAL_DISPLAY_HEIGHT, CHAR_PIXEL_LEN, CHAR_PIXEL_LEN+2, ST7735_BLACK);
			break;
		}
		case 3:
		{
			ST7735_FillRect(64 - 5*CHAR_PIXEL_LEN_SMALL + CHAR_PIXEL_LEN*3, 76 + DIGITAL_DISPLAY_HEIGHT, CHAR_PIXEL_LEN, CHAR_PIXEL_LEN+2, ST7735_BLACK);
			break;
		}
		case 4:
		{
			ST7735_FillRect(64 - 5*CHAR_PIXEL_LEN_SMALL + CHAR_PIXEL_LEN*4, 76 + DIGITAL_DISPLAY_HEIGHT, CHAR_PIXEL_LEN, CHAR_PIXEL_LEN+2, ST7735_BLACK);
			break;
		}
		case 5:
		{
		  ST7735_DrawChar(64 - 2*CHAR_PIXEL_LEN + 4*CHAR_PIXEL_LEN + (5-3)*CHAR_PIXEL_LEN_SMALL, 83 + DIGITAL_DISPLAY_HEIGHT, 
		  CHAR_PIXEL_LEN_SMALL, CHAR_PIXEL_LEN_SMALL+1, ST7735_BLACK, 1);
			break;
		}
		case 6:
		{
		  ST7735_DrawChar(64 - 2*CHAR_PIXEL_LEN + 4*CHAR_PIXEL_LEN + (6-3)*CHAR_PIXEL_LEN_SMALL, 83 + DIGITAL_DISPLAY_HEIGHT, 
		  CHAR_PIXEL_LEN_SMALL, CHAR_PIXEL_LEN_SMALL+1, ST7735_BLACK, 1);
			break;
		}
			
			
	}
}

void draw_digital_time( int8_t sec, int8_t min, int8_t hour, bool AM , int color)
{
	draw_digital_time_hundreth(sec, min, hour, -1, AM, color);
}

char* time_to_string(int8_t sec, int8_t min, int8_t hour, int8_t hundreth_sec, char* string) 
{
	string[0] = hour / 10 +48;
	string[1] = hour % 10+48;
	string[2] = ':';
	string[3] = min / 10+48;
	string[4] = min % 10+48;

	string[5] = sec / 10+48;
	string[6] = sec % 10+48;
	string[7] = NULL;
	
	if(hundreth_sec != -1)
	{
		string[7] = ':';
		string[8] = hundreth_sec / 10+48;
		string[9] = hundreth_sec % 10+48;
		string[10] = NULL;
	}
	


	
	return string;
}
