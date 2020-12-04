#ifndef EEPROM_H
#define EEPROM_H
/*************************************************************************
 Title  :   C include file for the eeprom library (eeprom.cpp)
 Author:    Arpita Chakraborty <achakraborty@machinesense.com>  

***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <EEPROM.h>
#include <WString.h>
#define Threshold_LOW 0
#define Threshold_MED 5
#define display_deg 15
#define Font_size 17
#define calib_A 21
#define calib_B 25
#define calib_M 30
#define calib_C 35
#define display_screen 41
#define display_fvr 60
#define display_nofvr 100
#define Time_display 200

extern String fvr_screen;
extern String nrml_screen;
extern char display_flag;
extern char screen;
extern float font_size;
extern int time_display;
extern float Threshold_T2L;
extern float Threshold_T2M;
extern float Threshold_T2H;
extern float calib_a;
extern float calib_b;
extern float calib_m;
extern float calib_c;

void eprom_init(); //initializing eeprom
void thrshld_get();//getting the threshold value already set in the device
void threshold_set(float low, float med);//setting the threshold values
void display_string_get();//getting the display string value already set in the device
void display_string_set(String display_fvrmsg, String display_nofvrmsg);//setting the display string value 
char display_deg_get();//getting the flag for showing temperature in C/F
void display_deg_set(char display_flag);//storing the flag for showing temperature in C/F
void display_screen_get();//getting the flag for showing temperature on screen or not
void display_screen_set(char screen);//storing the flag for showing temperature on screen
void font_get();//getting display font size
void font_set(float font_size);//storing display font size
void display_time_get();//getting the flag for showing time on screen or not
void display_time_set(int time_display);//storing the flag for showing time on screen or not
void calib_get();
void calib_set(float a, float b, float m, float c) ;
#endif
