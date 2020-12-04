/*************************************************************************
 Title  :   INO file for FEVERWARN-50A
 Author:    Arpita Chakraborty <achakraborty@machinesense.com>  

***************************************************************************/
#include <Tone32.h>
#include <Adafruit_MLX90614.h>
#include "QuickStats.h"
#include <stdio.h>
#include <stdlib.h>
#include "server.h"
//=========================ultrasonic============================================//
#define echoPin 34
#define trigPin 32

/*---------------------library global variables-------------------------------------*/
#define FORMAT_SPIFFS_IF_FAILED true
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
QuickStats stats;
char sql_data_send[5000] = {0};
float ot_array[60] = {0.0};
static int ot_array_count = 0;
/*----------------------------------------------------------------------------------*/
String fvr_screen;
String nrml_screen;
const int buzzer = 33;
float obj_final = 0;
float obj_final_c = 0;
char cal_on = 0;
float pulse_median = 0;
float amb_data;
char display_flag = 0;
char screen = 0;
float font_size = 0.0;
int time_display = 0;
int freq = 2000;
int channel = 0;
int resolution = 8;
char net_connect = 0;
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement
char count = 0;
int ledPin = 19;
char detect_flag = 0;
float Threshold_T2L, Threshold_T2M, Threshold_T2H, calib_a, calib_b, calib_m, calib_c;
unsigned long int start_time=0;
TaskHandle_t Task1;
QueueHandle_t queue;
int queueSize = 1;
//=================================core body==============================================//
float core_body,core_body_upper,core_body_lower;
void predict_core_body(float median,float ambobj)
{
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
 core_body = max(core_body, median);
 core_body_upper=error_upper;
 core_body_lower=error_lower;
 
}
//===========================================================================================//
float percentile(float arr[], int n) 
{ 
    int i, j, count;
    float percent; 
  
    // Start of the loop that calculates percentile 
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
/*----------------------------------------pulse_check--------------------------------------*/
void pulse_check() { // function for temperature check
  unsigned long previousMillis = millis();
  char time_data[50];
  int s_witch=0;
  char i = 0;
  char pulse_count = 0;
  float temp_data[50];
  float pulse_mean, pulse_std;float median_95,median_90,median_50;
  int k=0;
  tft.fillScreen(TFT_CYAN);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(30, 150);
  tft.setTextSize(3);
  tft.println("SCANNING...");
 Serial.println(fvr_screen);
//  tft.setCursor(40, 200);
//  tft.println("FOR SCAN");
  while (((millis() - previousMillis) <= 1000) && (distance > 1) && (distance <= 5)) {
    // omron.data_omron();
    temp_data[i++] = mlx.readObjectTempC();
//    Serial.println(mlx.readObjectTempC());
    amb_data = mlx.readAmbientTempC();
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    delay(99);
    pulse_count++;
  }
  if(cal_on == 1){
    if (pulse_count >= 10) {
       pulse_median = stats.median(temp_data, i);
       Serial.println(pulse_median);
    }
  }
  else if (cal_on == 0) {
  if (pulse_count >= 10) {
    detect_flag = 1;
    pulse_median = stats.median(temp_data, i);
    pulse_mean = stats.average(temp_data, i);
    pulse_std = stats.stdev(temp_data, i);
    i = 0;
   
    if ((pulse_std / pulse_mean) * 100 < 20) {
      Serial.print(pulse_median);
      if((calib_m==0.0)&&(calib_c==0.0)){
         obj_final = calibrate_readings(pulse_median);
      }else{
        obj_final = (calib_m * pulse_median) + calib_c;
      }   
       predict_core_body(obj_final,amb_data);	  
      obj_final_c =obj_final;
      s_witch=core_fist_get()-48;
      if(s_witch==0){
        obj_final=core_body;
        Serial.println("core_body");
      }
        if ((obj_final > Threshold_T2M) && (obj_final <= Threshold_T2H)) {
          //----------------fever case----------------------------//
          //        ledcWriteTone(channel, 500);
          //        ledcWrite(channel, 150);
          DateTime now = rtc.now();      
          all_data_store(now.unixtime() - 19800, obj_final_c, pulse_median, amb_data,core_body,core_body_upper,core_body_lower, 1);
          Serial2.println("{\"Feverwarn\":1}");
           //change_screen(fvr_screen, font_size, display_flag, screen);
          store_fvrdata(now.unixtime() - 19800, obj_final_c, pulse_median, amb_data,core_body,core_body_upper,core_body_lower, 1);
          for (int j = 0; j <=2; j++) {
            tft.fillScreen(TFT_RED);
            change_screen(fvr_screen, font_size, display_flag, screen);
            tone(buzzer, 2000, 400, channel);
            //delay(500);
            tone(buzzer, 500, 400, channel);
            tft.fillScreen(TFT_BLACK);
            //delay(50);
          }
          digitalWrite(buzzer,0);         
         home_screen();
         k=1;
         xQueueSend(queue, &k, portMAX_DELAY);
         //delete_sql_limit();
        } else if ((obj_final >= Threshold_T2L) && (obj_final <= Threshold_T2M)) {
          //----------------normal case----------------------------//
          //        ledcWriteTone(channel, 500);
          //        ledcWrite(channel, 200);  
          DateTime now = rtc.now();        
          all_data_store(now.unixtime() - 19800, obj_final_c, pulse_median, amb_data,core_body,core_body_upper,core_body_lower, 0);
          tft.fillScreen(TFT_GREEN);
          tft.setTextColor(TFT_BLACK, TFT_GREEN);
          Serial2.println("{\"Feverwarn\":0}");
          //Serial.write(0);
          change_screen(nrml_screen, font_size, display_flag, screen);
          tone(buzzer, 2000, 1000, channel);
          store_fvrdata(now.unixtime() - 19800, obj_final_c, pulse_median, amb_data, core_body,core_body_upper,core_body_lower, 0);
          noTone(buzzer);
          digitalWrite(buzzer,0);
          home_screen();
          k=1;   
         xQueueSend(queue, &k, portMAX_DELAY);
         //delete_sql_limit();
        } 
        else if (obj_final < Threshold_T2L) {
          //----------------too low case--------------------------//
          tft.fillScreen(TFT_YELLOW);
          tft.setTextColor(TFT_BLACK, TFT_YELLOW);
          change_screen("LOW TEMP",1.5, display_flag, screen);
   
          for(int i=0;i<2;i++){
            tone(buzzer, 2000, 100, channel);
            delay(1100);
            noTone(buzzer, channel);
          }
          digitalWrite(buzzer,0);    
          //delay(2000);
          home_screen();
        } else if (obj_final > Threshold_T2H) {
          //----------------too high case------------------------//
          tft.fillScreen(TFT_YELLOW);
          tft.setTextColor(TFT_BLACK, TFT_YELLOW);
          tft.setCursor(40, 120);
          tft.setTextSize(2.0);
          tft.println("Incosistent Reading");
          tft.setCursor(40, 140);
          tft.setTextSize(2.0);
          tft.println("Please Check Again");
          for(int i=0;i<2;i++){
            tone(buzzer, 2000, 100, channel);
            delay(1100);
            noTone(buzzer, channel);
          }
          digitalWrite(buzzer,0);
          delay(2000);
          home_screen();
        }
      }else{
        home_screen();
      }
    }
    else
    home_screen();
  }
  pulse_count = 0;
  start_time=millis();
}
/*====================================================================================================*/
 
 void Task1code( void * parameter) {
  int element;
  while(1) {
    xQueueReceive(queue, &element, portMAX_DELAY);
    if(element==1){
      Serial.println("core body 1");
      delete_sql_limit();
      element=0;
    }
   // client_hndl();
    vTaskDelay(10);
  }
}
/*====================================================================================================*/
void setup() {
  Serial.begin(9600);
  int rc;
  delay(500);
  Serial2.begin(9600,SERIAL_8N1,16,17);
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("Failed to mount file system");
    return;
  }
  sqlite3_initialize();
  init_database();
  tft.init();
  home_screen();
  tft.setRotation(2);
  tft.setTextSize(2);
  //#ifndef ESP8266
  //  while (!Serial); // wait for serial port to connect. Needed for native USB
  //#endif

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)) + TimeSpan(0, 0, 2, 30));
    //rtc.adjust(DateTime(2020, 9, 28, 16, 21, 20));
    tft.setCursor(50, 50);
    tft.print("power lost");
  }
  //rtc.adjust(DateTime(2020, 9, 28, 16,23, 20));
  mlx.begin();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT  
  digitalWrite(buzzer, 1);
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(33, channel);
  eprom_init();
  thrshld_get();
  display_deg_get();
  font_get();
  calib_get();
  display_screen_get();
  display_string_get();
  display_time_get();
  core_fist_save(1);
  createSSID();
  home_screen();
  queue = xQueueCreate( queueSize, sizeof( int ) );
  if(queue == NULL){
    Serial.println("Error creating the queue");
  }
  xTaskCreatePinnedToCore(Task1code,"Task1",10000,NULL,0,&Task1, 0); /* Core where the task should run */
  wifi_setup();
  server_setup();
  home_screen();

}
void loop() {
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  if ((distance > 1) && (distance <= 5) && (detect_flag == 0)) {
       pulse_check();
  } else if ((distance >= 30) && (detect_flag == 1)) {
       detect_flag = 0;
  }else if(detect_flag==1){
    if((millis()-start_time)>=10000){
      tft.fillScreen(TFT_BROWN);
      tft.setTextColor(TFT_WHITE);
      tft.setCursor(20, 140);
      tft.setTextSize(3);
      tft.println("REMOVE YOUR ");
       tft.setCursor(20, 180);
      tft.println("   HAND");
      tone(buzzer, 1400, 1400, channel);
      delay(2000);
      home_screen();
      start_time=millis();
    }
  }
  client_hndl();
  delay(100);
}
