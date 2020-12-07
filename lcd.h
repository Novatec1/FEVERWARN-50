#ifndef LCD_H
#define LCD_H
/*************************************************************************
 Title  :   C include file for the ili9341 LCD library (lcd.cpp)
 Author:    Arpita Chakraborty <achakraborty@machinesense.com>  

***************************************************************************/

#include "TFT_eSPI.h"
#include "RTClib.h"
#include "config.h"

extern TFT_eSPI tft;
extern RTC_DS3231 rtc;
void home_screen();
void change_screen(user_config_t &config,float obj_final, String display_data);
void lcd_init();
void rtc_init();
void calibration_screen();
void save_screen();
void scan_screen();
#endif
