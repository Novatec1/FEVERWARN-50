/****************************************************************************
 Title  :   ILI9341 LCD library
 Author:    Arpita Chakraborty <achakraborty@machinesense.com> 
 
 DESCRIPTION
       Basic routines for lcd pages using TFT_eSPI library
       
*****************************************************************************/

#include "lcd.h"

TFT_eSPI tft = TFT_eSPI();
RTC_DS3231 rtc;

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
  tft.setCursor(70, 150);
  tft.setTextSize(3);
  tft.println("READY");
  tft.setCursor(40, 200);
  tft.println("TO SCAN");
}

void change_screen(user_config_t &config,float obj_final, String display_data) {
  tft.setTextSize(config.font_size);
  tft.drawCentreString(display_data, 120, 120, 4);
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
    DateTime now = rtc.now();
    tft.setCursor(50, 230);
    tft.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());
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
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(50, 150);
    tft.print("power lost");
     tft.fillScreen(TFT_BLUE);
  }
}

void calibration_screen(){
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(30, 150);
  tft.setTextSize(3);
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

void scan_screen(){
  tft.fillScreen(TFT_CYAN);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(30, 150);
  tft.setTextSize(3);
  tft.println("SCANNING...");
}
 
