#ifndef DRIVER_LCD_H
#define DRIVER_LCD_H

#include <stdbool.h>
void draw_digital_time( int8_t sec, int8_t min, int8_t hour, bool AM, int color );
void cover_digital_time(int8_t place);
void draw_digital_time_edit( int8_t sec, int8_t min, int8_t hour, bool AM, int color, int place, char edit );

#endif
