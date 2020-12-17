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


//===========================================================================================//
user_config_t config;
char cal_on=0;
float pulse_median;
char change_config=0;
TaskHandle_t Task1;
QueueHandle_t queue;
int queueSize = 1;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
QuickStats stats;
char detect_flag = 0;
unsigned long int start_time=0;
//===========================================================================================//
void predict_core_body(float median,float ambobj,database_param_t &database){
 float core_body=0;
 float e1 = -0.0111;
 float e2 = 0.435;
 float coeff[2]={0.4851, 18.911};
 float compensator ,error_upper,error_lower,error_band;
 //core_body= median* coeff[0] + coeff[1];
 error_band= abs(e1 * ambobj + e2);
 if(median<36.6){
   core_body=0.4851*median+18.911;
 }else{ 
  core_body=0.4851*median+20.911;  
 }
if(core_body<36){
 compensator = random(0,error_band*100)/100;
 Serial.println(compensator);
 core_body = 36 + compensator;
 error_upper = error_band-compensator;
 error_lower = (-1)*(error_band+compensator);
}else{
 error_upper = 1*error_band;
 error_lower = (-1)*error_band;
}
 database.corebody_temp = max(core_body, median);
 database.corebody_error_pos=error_upper;
 database.corebody_error_neg=error_lower;
}
//===========================================================================================//
float percentile(float arr[], int n) { 
    int i, j, count;
    float percent; 
    for (i = 0; i < n; i++) {   
        count = 0; 
        for (int j = 0; j < n; j++) {   
            // Comparing the marks of student i 
            // with all other students 
            if (arr[i] > arr[j]) { 
                count++; 
            } 
        } 
        percent = (count * 100) / (n - 1); 
    }
    return percent; 
} 

//===========================================================================================//
float calibrate_readings(float raw){    
    float calib_1,calibrated,ca_bb;
    calib_1 =( 0.0015*pow(raw, 3)) - (0.1308*pow(raw,2)) + (4.0697*raw) - 7.8276;
    calib_1 = 0.7375*calib_1 + 8.7458;
    ca_bb = 0.6088*raw + 16.949;
    if (calib_1>=36)
            calibrated = max(ca_bb, calib_1);
    else
            calibrated = calib_1-0.5;
    return calibrated;

}

//============================================================================================//
int proximity(){
  int distance=0;
  long duration=0;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  return distance;
}
/*====================================================================================================*/
 
 void Task1code( void * parameter) {
  int element;
  while(1) {
    xQueueReceive(queue, &element, portMAX_DELAY);
    if(element==1){
      Serial.println("Task  code1");
      delete_sql_limit();
      element=0;
    }
   // client_hndl();
    vTaskDelay(10);
  }
}
/*====================================================================================================*/

void pulse_check(int distance) { // function for temperature check
  unsigned long previousMillis = millis();
  char s_witch=0;
  char i = 0;
  float amb_data=0;
  char pulse_count = 0;
  float temp_data[10];
  float pulse_mean, pulse_std;
  int k=0;
  float obj_final,obj_final_c;
  database_param_t database;
  scan_screen();
  while (((millis() - previousMillis) <= 1000) && (distance > 1) && (distance <= 5)) {
    temp_data[i++] = mlx.readObjectTempC();
    amb_data = mlx.readAmbientTempC();
    distance=proximity();
    Serial.println(distance);
    delay(99);
    pulse_count++;
  }
  if(cal_on == 1){
    if (pulse_count >= 10) {
       pulse_median = stats.median(temp_data, i);
       Serial.println(pulse_median);
    }
  }
  else if ((cal_on == 0) && (pulse_count >= 10)) {
    detect_flag = 1;
    pulse_median = stats.median(temp_data, i);
    pulse_mean = stats.average(temp_data, i);
    pulse_std = stats.stdev(temp_data, i);
    i = 0;   
    if ((pulse_std / pulse_mean) * 100 < 20) {
      Serial.print(pulse_median);
      if((config.calibration_param[0]==0.0)&&(config.calibration_param[1]==0.0)){
         obj_final = calibrate_readings(pulse_median);
      }else{
        obj_final = (config.calibration_param[0] * pulse_median) + config.calibration_param[1];
      }   
       predict_core_body(obj_final,amb_data,database);    
       obj_final_c =obj_final;
       s_witch=config.core_body;
      if(s_witch==0){
        obj_final=database.corebody_temp;
        Serial.println("core_body");
      }
          DateTime now = rtc.now();    
          database.time_data=now.unixtime();
          database.obj_data=obj_final_c;
          database.raw_temp=pulse_median;
          database.amb_temp=amb_data;
          
        if ((obj_final > config.threshold_temp[1]) && (obj_final <= config.threshold_temp[2])) {
          //----------------fever case----------------------------//
          Serial2.println("{\"Feverwarn\":1}");
          database.fver_stat=1;
          store_fvrdata(database);
          for (int j = 0; j <=2; j++) {
            tft.fillScreen(TFT_RED);           
            change_screen(config,obj_final,config.fever_msg);
            tone(buzzer, 2000, 400, BUZZER_CHANNEL);
            //delay(500);
            tone(buzzer, 500, 400, BUZZER_CHANNEL );
            tft.fillScreen(TFT_BLACK);
            //delay(50);
          }
          digitalWrite(buzzer,0);         
         home_screen();
         k=1;
         xQueueSend(queue, &k, portMAX_DELAY);
         //delete_sql_limit();
        } else if ((obj_final >= config.threshold_temp[0]) && (obj_final <= config.threshold_temp[1])) {
          //----------------normal case----------------------------//
          //        ledcWriteTone(channel, 500);
          //        ledcWrite(channel, 200);  
          tft.fillScreen(TFT_GREEN);
          tft.setTextColor(TFT_BLACK, TFT_GREEN);
          Serial2.println("{\"Feverwarn\":0}");
          change_screen(config,obj_final,config.normal_msg);
          tone(buzzer, 2000, 1000,BUZZER_CHANNEL );
          database.fver_stat=0;
          store_fvrdata(database);
          noTone(buzzer);
          digitalWrite(buzzer,0);
          home_screen();
          k=1;   
         xQueueSend(queue, &k, portMAX_DELAY);
         //delete_sql_limit();
        } 
        else if (obj_final < config.threshold_temp[0]) {
          //----------------too low case--------------------------//
          database.fver_stat=2;
          store_fvrdata(database);
          tft.fillScreen(TFT_YELLOW);
          tft.setTextColor(TFT_BLACK, TFT_YELLOW);
          change_screen(config,obj_final,"LOW_BODY");   
          for(int i=0;i<2;i++){
            tone(buzzer, 2000, 100, BUZZER_CHANNEL );
            delay(1100);
            noTone(buzzer, BUZZER_CHANNEL );
          }
          digitalWrite(buzzer,0);    
          //delay(2000);
          home_screen();
        } else if (obj_final > config.threshold_temp[2]) {
          //----------------too high case------------------------//
          tft.fillScreen(TFT_YELLOW);
          tft.setTextColor(TFT_BLACK, TFT_YELLOW);
          tft.setCursor(20, 120);
          tft.setTextSize(2.0);
          tft.println("Incosistent Reading");
          tft.setCursor(20, 140);
          tft.setTextSize(2.0);
          tft.println("Please Check Again");
          for(int i=0;i<2;i++){
          tone(buzzer, 2000, 100,BUZZER_CHANNEL );
            delay(1100);
            noTone(buzzer, BUZZER_CHANNEL );
          }
          digitalWrite(buzzer,0);
          delay(2000);
          home_screen();
        }
      }else{
        home_screen();
      }
    }else
       home_screen();
    pulse_count = 0;
    start_time=millis();
}
//============================================================================================//
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600,SERIAL_8N1,16,17);
  SPIFFS.begin();
  mlx.begin();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT  
  //digitalWrite(buzzer, 1);
  buzzer_setup();
  loadConfiguration(config);  
  lcd_init();
  rtc_init();  
  init_database();
  rtc_init();
  queue = xQueueCreate( queueSize, sizeof( int ) );
  if(queue == NULL){
    Serial.println("Error creating the queue");
  }
  xTaskCreatePinnedToCore(Task1code,"Task1",10000,NULL,0,&Task1, 0); /* Core where the task should run */
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
  if ((distance > 1) && (distance <= 5) && (detect_flag == 0)) {
       pulse_check(distance);
  } else if ((distance >= 30) && (detect_flag == 1)) {
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
