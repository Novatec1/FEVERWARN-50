/****************************************************************************
 Title  :   ILI9341 LCD library
 Author:    Arpita Chakraborty <achakraborty@machinesense.com> 
 
 DESCRIPTION
       Basic routines for lcd pages using TFT_eSPI library
       
*****************************************************************************/

#include "lcd.h"

TFT_eSPI tft = TFT_eSPI();
RTC_DS3231 rtc;

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

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
//====================================================================================
//image rendering function

void drawBmp(const char *filename, int16_t x, int16_t y) {

  if ((x >= tft.width()) || (y >= tft.height())) return;

  fs::File bmpFS;

  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");

  if (!bmpFS)
  {
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

  uint32_t startTime = millis();

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
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
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      }
      tft.setSwapBytes(oldSwapBytes);
      Serial.print("Loaded in "); Serial.print(millis() - startTime);
      Serial.println(" ms");
    }
    else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}


/*----------------------------------------------------------------------------------*/
void home_screen() {
  tft.fillScreen(TFT_BLUE);
  //tft.setFreeFont(LABEL2_FONT);
  //  tft.setFreeFont(FF45);
  tft.setTextColor(TFT_ORANGE);
  // tft.setCursor(70, 70);
  tft.setTextSize(4);
  drawBmp("/fvrwrn1.bmp", 20, 50);
  drawBmp("/mslogo.bmp", 20, 250);
  // tft.println("FEVERWARN");
  //drawJpeg("/fvrwrn1.jpg", 20, 50);
  //drawJpeg("/mslogo.jpg", 20, 250);
  tft.setCursor(70, 150);
  //  tft.setFreeFont(LABEL2_FONT);
  tft.setTextSize(3);
  tft.println("READY");
  tft.setCursor(40, 200);
  tft.println("FOR SCAN");
}

/*---------------------------------------------------------------------------------*/

void change_screen(String display_data, float font_size, char unit, char show_display) {
  tft.setTextSize(font_size);
  tft.drawCentreString(display_data, 120, 120, 4);
  tft.setCursor(60, 50);
 // tft.setCursor(60, 150)
  tft.setTextSize(3);
  if (unit == 1) {
    obj_final = (obj_final * 9 / 5) + 32;
    amb_data = (amb_data * 9 / 5) + 32;
  }
  if (show_display == 1) { // 0=>C,1=>F
    if (unit == 1) {
      tft.print(obj_final);
      tft.println(" F");
    } else {
      tft.print(obj_final);
      tft.println(" C");
    }
  }
  if (time_display == 1) {
    tft.setTextSize(3);
    DateTime now = rtc.now();
//    tft.setCursor(60, 210);
//    tft.printf("%02d/%02d/%02d", now.year(), now.month(), now.day());
    tft.setCursor(50, 230);
    tft.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  }

}
