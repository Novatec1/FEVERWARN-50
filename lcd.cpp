/****************************************************************************
 Title  :   ILI9341 LCD library
 Author:    Arpita Chakraborty <achakraborty@machinesense.com> 
 
 DESCRIPTION
       Basic routines for lcd pages using TFT_eSPI library
       
*****************************************************************************/

#include "lcd.h"

TFT_eSPI tft = TFT_eSPI();
RTC_DS3231 rtc;
extern String adjust_timezone;
uint16_t read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

void drawBmp(const char *filename, int16_t x, int16_t y) {
  if ((x >= tft.width()) || (y >= tft.height())) return;
  fs::File bmpFS;
  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;
  bmpFS = SPIFFS.open(filename, "r");
  if (!bmpFS){
    Serial.print("File not found");
    return;
  }
  if (read16(bmpFS) == 0x4D42){
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);
    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0)){
      y += h - 1;
      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);
      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];
      for (row = 0; row < h; row++) {        
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++){
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }
        tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      }
      tft.setSwapBytes(oldSwapBytes);
    }
    else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}

void home_screen() {
  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_ORANGE);
  tft.setTextSize(4);
  drawBmp("/fvrwrn1.bmp", 20, 50);
  drawBmp("/mslogo.bmp", 20, 250);
  tft.setCursor(75, 150);
  tft.setTextSize(3);
  tft.println("READY");
  tft.setCursor(40, 200);
  tft.println(" TO SCAN");
}

void change_screen(user_config_t &config,float obj_final, String display_data) {
  int res=strcmp(display_data.c_str(),"LOW_TEMP");
  Serial.println(res);
  if(res==0){
    tft.setTextSize(1.5);
    tft.drawCentreString(display_data, 120, 140,4);
  }else{
     tft.setTextSize(config.font_size);
     tft.drawCentreString(display_data, 120, 140, 4);
  }   
  
  tft.setCursor(60, 50);
  tft.setTextSize(3);
  if(config.display_temp==1){
  if(config.display_in_c==1){
    obj_final=(obj_final * 9/5) + 32;
    tft.print(obj_final);
    tft.println(" F");
  }else{
    tft.print(obj_final);
    tft.println(" C");
  }
  }
  if (config.time_display == 1) {
    tft.setTextSize(3);
    tft.setCursor(50, 230);    
    DateTime now = rtc.now();
    unsigned long long n =now.unixtime()+config.timezone;
    n = n % (24 * 3600); 
    int hr_adj= n / 3600; 
    n %= 3600; 
    int min_adj = n / 60 ;
    n %= 60; 
    int sec_adj = n; 
    tft.printf("%02d:%02d:%02d",hr_adj,min_adj,sec_adj);
  }
}

void lcd_init(){
  tft.init();
  home_screen();
  tft.setRotation(2);
  tft.setTextSize(2);
}

void rtc_init(){
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)) + TimeSpan(0, 0, 2, 30));
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(30, 150);
    tft.print("RTC power lost");
    tft.setCursor(30, 170);
    tft.print("SYNC WITH MOBILE");
    delay(2000);
    tft.fillScreen(TFT_BLUE);
  }
}

void calibration_screen(){
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(27, 150);
  tft.setTextSize(3);
  tft.setTextColor(TFT_MAGENTA,TFT_BLACK);
  tft.println("CALIBRATING");
}

void save_screen(){
  tft.fillScreen(TFT_MAGENTA);
  tft.setTextSize(3);
  tft.setTextColor(TFT_BLACK, TFT_MAGENTA);
  tft.setCursor(40, 150);
  tft.println("  SAVED ");
  delay(500);
}

void display_threshold(String low,String med){
  tft.fillScreen(TFT_MAGENTA);
  tft.setTextColor(TFT_BLACK, TFT_MAGENTA);
  tft.setTextSize(2);
  tft.setCursor(30, 110);
  tft.print("Lower Threshold");
  tft.setCursor(70, 140);
  tft.print(low);
  tft.setCursor(30, 170);
  tft.print("Fever Threshold");
  tft.setCursor(70, 195);
  tft.print(med);
  delay(2000);
}

void scan_screen(){
  tft.fillScreen(TFT_CYAN);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(30, 150);
  tft.setTextSize(3);
  tft.println("SCANNING...");
}

void calibration_done_screen(){
  tft.fillScreen(TFT_MAGENTA);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK, TFT_MAGENTA);
  tft.setCursor(30, 150);
  tft.println("CALIBRATION DONE");
  delay(2000);
}

void calibration_fail_screen(){
  tft.fillScreen(TFT_MAGENTA);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK, TFT_MAGENTA);
  tft.setCursor(30, 150);
  tft.println("CALIBRATION FAIL");
  delay(2000);
}

void remove_hand(){
  tft.fillScreen(TFT_BROWN);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(20, 140);
  tft.setTextSize(3);
  tft.println("REMOVE YOUR ");
  tft.setCursor(20, 180);
  tft.println("   HAND");
}
