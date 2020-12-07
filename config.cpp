/****************************************************************************
 Title  :   USER CONFIGURATION STORAGE library
 Author:    Arpita Chakraborty <achakraborty@machinesense.com> 
 
 DESCRIPTION
       Basic routines for storing user data 
       
*****************************************************************************/
#include "config.h"
#include <ArduinoJson.h>

// Loads the configuration from a file
void loadConfiguration(user_config_t &config) {
  File file = SPIFFS.open(USER_CONFIG,FILE_READ);
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
     Serial.println(F("Failed to read file, using default configuration"));   
  config.threshold_temp[0]=doc["THRESHOLD_LOW"];
  config.threshold_temp[1]=doc["THRESHOLD_MED"];
  config.threshold_temp[2]=doc["THRESHOLD_HIGH"];
  strcpy(config.fever_msg,doc["FEVER_MSG"]);
  strcpy(config.normal_msg,doc["NORMAL_MSG"]);
  config.display_in_c=doc["DISPLAY_DEG"];
  config.display_temp=doc["DISPLAY_TEMP"];
  config.time_display=doc["DISPLAY_TIME"];
  config.font_size=doc["FONT_SIZE"];
  config.core_body=doc["DISPLAY_COREBODY"];
  config.calibration_param[0]=doc["CALIB_M"];
  config.calibration_param[1]=doc["CALIB_C"];
  file.close();
}

void saveConfiguration(user_config_t &config){
  File file = SPIFFS.open(USER_CONFIG,FILE_WRITE);
  StaticJsonDocument<512> doc;
  doc["THRESHOLD_LOW"]=config.threshold_temp[0];
  doc["THRESHOLD_MED"]=config.threshold_temp[1];
  doc["THRESHOLD_HIGH"]=config.threshold_temp[2];
  doc["FEVER_MSG"]=config.fever_msg;
  doc["NORMAL_MSG"]=config.normal_msg;
  doc["DISPLAY_DEG"]=config.display_in_c;
  doc["DISPLAY_TEMP"]=config.display_temp;
  doc["DISPLAY_TIME"]=config.time_display;
  doc["FONT_SIZE"]=config.font_size;
  doc["DISPLAY_COREBODY"]=config.core_body;
  doc["CALIB_M"]=config.calibration_param[0];
  doc["CALIB_C"]=config.calibration_param[1];
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  file.close();
}
