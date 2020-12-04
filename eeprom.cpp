/****************************************************************************
 Title  :   EEPROM STORAGE library
 Author:    Arpita Chakraborty <achakraborty@machinesense.com> 
 
 DESCRIPTION
       Basic routines for storing user data in eeprom
       
*****************************************************************************/
#include "eeprom.h"
 

void eprom_init() {
  if (!EEPROM.begin(1000)) {
//    Serial.println("Failed to initialise EEPROM");
//    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
}

void thrshld_get() {
  EEPROM.get(Threshold_LOW, Threshold_T2L);
  EEPROM.get(Threshold_MED, Threshold_T2M);
  Serial.println(Threshold_T2L);
  Serial.println(Threshold_T2M);
  if (isnan(Threshold_T2L)) {
    Threshold_T2L = 30.0;
    EEPROM.put(Threshold_LOW, Threshold_T2L);
    EEPROM.commit();
    delay(500);
  }
  if (isnan(Threshold_T2M)) {
    Threshold_T2M = 37.5;
    EEPROM.put(Threshold_MED, Threshold_T2M);
    EEPROM.commit();
    delay(500);
  }
  Threshold_T2H = 42.0;
}

void threshold_set(float low, float med) //set threshold values
{
  EEPROM.put(Threshold_LOW, low);
  EEPROM.put(Threshold_MED, med);
  EEPROM.commit();
  delay(500);
  thrshld_get();
  delay(1500);
}

void display_string_get() {
  EEPROM.get(display_fvr, fvr_screen);
  EEPROM.get(display_nofvr, nrml_screen);  
  if (EEPROM.read(display_fvr)==255) {
    fvr_screen = "STOP";
    EEPROM.put(display_fvr, fvr_screen);
    EEPROM.commit();
    delay(500);   
  }
  if (EEPROM.read(display_nofvr)==255) {
    nrml_screen = "GO";
    EEPROM.writeString(display_nofvr,nrml_screen);
    EEPROM.commit();    
  }
  delay(500);
  Serial.println(fvr_screen);
  Serial.println(nrml_screen);
}

void display_string_set(String display_fvrmsg, String display_nofvrmsg) {
  EEPROM.put(display_fvr, display_fvrmsg);
  EEPROM.put(display_nofvr, display_nofvrmsg);
  EEPROM.commit();
  delay(500);
  display_string_get();
  delay(1500);
}
char display_deg_get() {
  EEPROM.get(display_deg, display_flag);
  if (display_flag == 255) {
    display_flag = 0;
    EEPROM.put(display_deg, display_flag);
    EEPROM.commit();
    delay(500);
  } else {
  }
  return display_flag;
}

void display_deg_set(char display_flag) {
  EEPROM.put(display_deg, display_flag);
  EEPROM.commit();
  delay(500);
  display_deg_get();
  delay(500);
}

void display_screen_get() {
  EEPROM.get(display_screen, screen);
  if (screen == 255) {
    screen = 0;
    EEPROM.put(display_screen, screen);
    EEPROM.commit();
  } 
}

void display_screen_set(char screen) {
  EEPROM.put(display_screen, screen);
  EEPROM.commit();
  delay(500);
  display_screen_get();
  delay(500);
}

void font_get() {
  EEPROM.get(Font_size, font_size);
  if (isnan(font_size)) {
    font_size = 2.5;
    EEPROM.put(Font_size, font_size);
    EEPROM.commit();
  }
}

void font_set(float font_size) {
  EEPROM.put(Font_size, font_size);
  EEPROM.commit();
  delay(500);
  font_get();
  delay(500);
}

void display_time_get() {
  EEPROM.get(Time_display, time_display);
  if (time_display == 255) {
    time_display = 0;
    EEPROM.put(Time_display, time_display);
    EEPROM.commit();
  } 
}

void display_time_set(int time_display) {
  EEPROM.put(Time_display, time_display);
  EEPROM.commit();
  delay(500);
  display_time_get();
  delay(1500);
  
}

void calib_get() {
  EEPROM.get(calib_A, calib_a);
  EEPROM.get(calib_B, calib_b);
  EEPROM.get(calib_M, calib_m);
  EEPROM.get(calib_C, calib_c);
  if (isnan(calib_a)) {
    calib_a = 0.0;
    EEPROM.put(calib_A, calib_a);
    EEPROM.commit();
  }
  if (isnan(calib_b)) {
    calib_b = 0.0;
    EEPROM.put(calib_B, calib_b);
    EEPROM.commit();
  }
  if (isnan(calib_m)) {
    calib_m = 0.46;
    EEPROM.put(calib_M, calib_m);
    EEPROM.commit();
  }
  if (isnan(calib_c)) {
    calib_c = 21.02;
    EEPROM.put(calib_C, calib_c);
    EEPROM.commit();
  }
  
}

void calib_set(float a, float b, float m, float c) {
  EEPROM.put(calib_A, a);
  EEPROM.put(calib_B, b);
  EEPROM.put(calib_M, m);
  EEPROM.put(calib_C, c);
  EEPROM.commit();
  delay(500);
  calib_get();
  delay(1500);
}
