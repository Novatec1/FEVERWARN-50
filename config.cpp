/****************************************************************************
 Title  :   USER CONFIGURATION STORAGE library
 Author:    Arpita Chakraborty <achakraborty@machinesense.com> 
 
 DESCRIPTION
       Basic routines for storing user data 
       
*****************************************************************************/
#include "config.h"
#include <ArduinoJson.h>
#include <Wstring.h>

void buzzer_setup(){
  int freq = 2000;
  int channel = 0;
  int resolution = 8;
  //digitalWrite(buzzer, 1);
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(33, channel);
}
// Loads the configuration from a file
void loadConfiguration(user_config_t &config) {
  File file = SPIFFS.open(USER_CONFIG,FILE_READ);
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
     Serial.println(F("Failed to read file, using default configuration"));   
  config.threshold_temp[0]=doc["THRESHOLD_LOW"].as<float>();
  config.threshold_temp[1]=doc["THRESHOLD_MED"].as<float>();
  config.threshold_temp[2]=doc["THRESHOLD_HIGH"].as<float>();
  strcpy(config.fever_msg,doc["FEVER_MSG"]);
  strcpy(config.normal_msg,doc["NORMAL_MSG"]);
  config.display_in_c=doc["DISPLAY_DEG"].as<char>();
  config.display_temp=doc["DISPLAY_TEMP"].as<char>();
  config.time_display=doc["DISPLAY_TIME"].as<char>();
  config.font_size=doc["FONT_SIZE"].as<float>();
  config.core_body=doc["DISPLAY_COREBODY"].as<char>();
  config.calibration_param[0]=doc["CALIB_M"].as<float>();
  config.calibration_param[1]=doc["CALIB_C"].as<float>();
  config.timezone=doc["TIMEZONE"].as<int>();
  file.close();
}

void saveConfiguration(String object,String data_new){
  user_config_t config;
  File file = SPIFFS.open(USER_CONFIG,FILE_READ);
  String buffer_json;
  while(file.available()){
      buffer_json+=char(file.read());
    }
   
   Serial.println(buffer_json);
  file.close();
  StaticJsonDocument<512> json;
  deserializeJson(json,buffer_json);
  json[object]=data_new;
  serializeJsonPretty(json, Serial);
  file = SPIFFS.open(USER_CONFIG,FILE_WRITE);
   serializeJsonPretty(json, file);
   file.close();
}
