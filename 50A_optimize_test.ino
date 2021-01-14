
#include <Tone32.h>
#include <Adafruit_MLX90614.h>
#include "QuickStats.h"
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
#include "server.h"
#include "config.h"
#include "FS.h"
#include "SPIFFS.h"
#include "sensor.h"


//===========================================================================================//
extern user_config_t config;
TaskHandle_t Task1;
QueueHandle_t queue;
int queueSize = 1;
char detect_flag = 0;
unsigned long int start_time=0;
char db_ready=0;
char change_config=0;
/*====================================================================================================*/
 
 void Task1code( void * parameter) {
  int element;
  while(1) {
    xQueueReceive(queue, &element, portMAX_DELAY);
    if(element==1){
      Serial.println("Task  code1");
      if(db_ready==0){
        db_ready=1;
        delete_sql_limit();
        db_ready=0;
      }
      element=0;
    }
   // client_hndl();
    vTaskDelay(10);
  }
}

//============================================================================================//
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600,SERIAL_8N1,16,17);
  SPIFFS.begin();
  sensor_setup();
  //digitalWrite(buzzer, 1);
  buzzer_setup();
  //sd_init();
  loadConfiguration(config);  
  lcd_init();
  rtc_init();  
  init_database();
  rtc_init();
  queue = xQueueCreate( queueSize, sizeof( int ) );
  if(queue == NULL){
    Serial.println("Error creating the queue");
  }
  xTaskCreatePinnedToCore(Task1code,"Task1",10000,(void*)&db_ready,0,&Task1, 0); /* Core where the task should run */
  wifi_setup();
  server_setup();
  home_screen();
//  database_param_t database;
//  DateTime now = rtc.now();    
//          database.time_data=now.unixtime() - 19800;
//          database.obj_data=37.5;
//          database.raw_temp=40.0;
//          database.amb_temp=26.0;
//          database.corebody_temp=36.0;
//          database.corebody_error_pos=0.0;
//          database.corebody_error_neg=0.0;
//          database.fver_stat=1;
//  for(int i=0;i<=1000;i++){
//     store_fvrdata(database);
//     delay(10);
//  }
}

void loop() {
  int distance=proximity();
  //Serial.println(distance);
  if ((distance > 1) && (distance <= 5) && (detect_flag == 0)) {
       pulse_check(distance);
  } else if ((distance >= 20) && (detect_flag == 1)) {
       detect_flag = 0;
  }else if(detect_flag==1){
    if((millis()-start_time)>=10000){
      remove_hand();
      tone(buzzer, 1400, 1400,BUZZER_CHANNEL);
      delay(2000);
      home_screen();
      start_time=millis();
    }
  }
  if(change_config==1){
    loadConfiguration(config);
    change_config=0;
  }
  client_hndl();
  delay(100);
}
