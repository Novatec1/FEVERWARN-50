#ifndef LCD_H
#define LCD_H
/*************************************************************************
 Title  :   C include file for the ili9341 LCD library (lcd.cpp)
 Author:    Arpita Chakraborty <achakraborty@machinesense.com>  

***************************************************************************/

#include "FS.h"
#include "SPIFFS.h"
#include "TFT_eSPI.h"
#include "RTClib.h"

extern float obj_final;
extern float amb_data;
extern int time_display;
extern TFT_eSPI tft;
extern RTC_DS3231 rtc;
void home_screen();
void change_screen(String display_data, float font_size, char unit, char show_display);
#endif
