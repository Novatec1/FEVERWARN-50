/****************************************************************************
 Title  :  MLX and proximity Sensor communication
 Author:    Arpita Chakraborty <achakraborty@machinesense.com> 
 
 DESCRIPTION
       Basic routines for communication with mlx and proximity
       
*****************************************************************************/
#include <Adafruit_MLX90614.h>
#include "QuickStats.h"
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
#include "config.h"
#include "sensor.h"
#include <Tone32.h>
#include "server.h"

char cal_on=0;
float pulse_median;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
QuickStats stats;
user_config_t config;
//===========================================================================================//
void sensor_setup(){
  mlx.begin();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT  
}
//===========================================================================================//
void predict_core_body(float median,float ambobj,database_param_t &database){
 float core_body=0;
 float e1 = -0.0111;
 float e2 = 0.435;
 float coeff[2]={0.4851, 18.911};
 float compensator ,error_upper,error_lower,error_band;
 core_body= median* coeff[0] + coeff[1];
 error_band= abs(e1 * ambobj + e2);
// if(median<36.6){
//   core_body=0.4851*median+18.911;
// }else{ 
//  core_body=0.4851*median+20.911;  
// }
if(core_body<36){
 //compensator = random(0,error_band*100)/100;
 compensator = (float)(rand())/(float)(RAND_MAX/(error_band-0));
 Serial.printf("compensator:%f",compensator);
 core_body = 36 + compensator;
 error_upper = error_band-compensator;
 error_lower = (-1)*(error_band+compensator);
}else{
 error_upper = 1*error_band;
 error_lower = (-1)*error_band;
}
 database.corebody_temp = core_body ;
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
    double calib_1,calibrated,ca_bb,max_obj;
    calib_1= 568.6594235092032 - 54.87564028564181*raw + 1.8477388697972026*pow(raw,2)- 0.02041510196307671*pow(raw,3);//# if raw <= 34.43144 else 15.80113347217166 + 0.598861845190883*raw
    ca_bb= 15.80113347217166 + 0.598861845190883*raw;
    max_obj=max(calib_1,ca_bb);
    calibrated=min(40.56,max_obj);
    Serial.printf("core body temp:%f",calibrated);
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
  char relay_data[500];
  char sd_save[500];
  database_param_t database;
  if(cal_on==0){
    scan_screen();
  while (((millis() - previousMillis) <= 1000) && (distance > 1) && (distance <= 5)) {
    temp_data[i] = mlx.readObjectTempC();
    amb_data = mlx.readAmbientTempC();
    distance=proximity();
    Serial.println(temp_data[i++]);
    delay(99);
    pulse_count++;
  }
   if (pulse_count >= 10) {
    detect_flag = 1;
    pulse_median = stats.median(temp_data, i);
    pulse_mean = stats.average(temp_data, i);
    pulse_std = stats.stdev(temp_data, i);
    stats.bubbleSort(temp_data, i);
    Serial.println(temp_data[i-2]);
    pulse_median=temp_data[i-2];
    i = 0;   
    if ((pulse_std / pulse_mean) * 100 < 20) {
      Serial.print(pulse_median);
      if((config.calibration_param[0]==0.0)&&(config.calibration_param[1]==0.0)){
         obj_final = calibrate_readings(pulse_median);
      }else{
        obj_final = (config.calibration_param[0] * pulse_median) + config.calibration_param[1];
        Serial.print(obj_final);
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
          
          database.fver_stat=1;
          if(db_ready==0){
            db_ready=1;
            store_fvrdata(database);
            sprintf(&sd_save[0],"%d,%f,%f,%f,%f,%f,%f,%d\n", database.time_data,database.obj_data,database.raw_temp, database.amb_temp, database.corebody_temp, database.corebody_error_pos, database.corebody_error_neg, database.fver_stat);
            //sd_write(SD_DATA,sd_save );
            db_ready=0;
          }
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
          Serial2.printf("{\"Feverwarn\":\"false\",\"Temp\":%f,\"TS\":%ld,\"AmbT\":%f}",database.obj_data,database.time_data,database.amb_temp);      
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
          sprintf(&relay_data[0],"{\"Feverwarn\":\"true\",\"Temp\":%f,\"TS\":%ld,\"AmbT\":%f}",database.obj_data,database.time_data,database.amb_temp);
          Serial2.write(relay_data);
          //Serial.write(relay_data);
          change_screen(config,obj_final,config.normal_msg);
          tone(buzzer, 2000, 1000,BUZZER_CHANNEL );
          database.fver_stat=0;
          if(db_ready==0){
            db_ready=1;
            store_fvrdata(database);
            db_ready=0;
          }
          sprintf(&sd_save[0],"%d,%f,%f,%f,%f,%f,%f,%d\n", database.time_data,database.obj_data,database.raw_temp, database.amb_temp, database.corebody_temp, database.corebody_error_pos, database.corebody_error_neg, database.fver_stat);
          //sd_write(SD_DATA,sd_save );
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
          tft.fillScreen(TFT_GREENYELLOW);
          tft.setTextColor(TFT_BLACK, TFT_GREENYELLOW); //TFT_YELLOW
          tft.setCursor(35, 100);
          tft.setTextSize(2.0);
          tft.println("TEMPERATURE IS");
          tft.setCursor(70, 130);
          tft.setTextSize(2.0);
          tft.println("TOO LOW.");
          tft.setCursor(35, 170);
          tft.println("INACCURACIES IN");
          tft.setCursor(50, 195);
          tft.println("RESULT SHOULD");
          tft.setCursor(50, 225);
          tft.println("BE EXPECTED!");
          delay(2000);
          tft.fillScreen(TFT_GREENYELLOW);
          tft.setTextColor(TFT_BLACK, TFT_GREENYELLOW); //TFT_YELLOW
          change_screen(config,obj_final,"LOW_TEMP");
          for(int i=0;i<2;i++){
            tone(buzzer, 2000, 100, BUZZER_CHANNEL );
            delay(1100);
            noTone(buzzer, BUZZER_CHANNEL );
          }
          digitalWrite(buzzer,0);    
         // Serial2.printf("{\"Feverwarn\":\"false\",\"Temp\":%f,\"TS\":%ld,\"AmbT\":%f}",obj_finaldatabase.obj_data,database.time_data,database.amb_temp);
          //delay(2000);
          home_screen();
        } else if (obj_final > config.threshold_temp[2]) {
          //----------------too high case------------------------//
          tft.fillScreen(TFT_YELLOW);
          tft.setTextColor(TFT_BLACK, TFT_YELLOW);
          tft.setCursor(40, 120);
          tft.setTextSize(2.0);
          tft.println("Inconsistent");
          tft.setCursor(70, 150);
          tft.setTextSize(2.0);
          tft.println("Reading");
          tft.setCursor(14, 180);
          tft.setTextSize(2.0);
          tft.println("Please Scan Again");
          for(int i=0;i<2;i++){
          tone(buzzer, 2000, 100,BUZZER_CHANNEL );
            delay(1100);
            noTone(buzzer, BUZZER_CHANNEL );
          }
          digitalWrite(buzzer,0);
          delay(2000);
//          Serial2.printf("{\"Feverwarn\":\"false\",\"Temp\":%f,\"TS\":%ld,\"AmbT\":%f}",obj_finaldatabase.obj_data,database.time_data,database.amb_temp);
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
}
//====================================================================//
float sensor_calibration(unsigned long duration){
  unsigned long previousMillis = millis();
  float temp_data[100]={0.0};
  int distance=0;
  int pulse_count=0;
  while ((millis() - previousMillis) <= duration){ 
    distance=proximity();
    if ((distance > 1) && (distance <= 5)) {
    scan_screen();
    temp_data[pulse_count++] = mlx.readObjectTempC();    
    Serial.println(mlx.readObjectTempC());
    delay(100);    
   }
  }
  pulse_median = stats.median(temp_data, pulse_count); 
  calibration_screen();
  return pulse_median;
}
//======================================================================//
float get_data(){
  float value=mlx.readObjectTempC();
  return value;
}
