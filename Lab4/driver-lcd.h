#ifndef DRIVER_LCD_H
#define DRIVER_LCD_H

#include <stdbool.h>
void draw_digital_time( int8_t sec, int8_t min, int8_t hour, bool AM, int color );
void cover_digital_time(int8_t place);
void draw_digital_time_edit( int8_t sec, int8_t min, int8_t hour, bool AM, int color, int place, char edit );
void draw_clock_markers(uint16_t color);
void draw_hundreth_hand(uint8_t min, uint16_t color, uint8_t minute);

void draw_clock_face(void);
void draw_minute_hand(uint8_t min, uint16_t color, uint8_t divisor, uint8_t minute);
#endif
