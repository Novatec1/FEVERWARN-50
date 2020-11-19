#include <Tone32.h>

#include "RTClib.h"

#include "TFT_eSPI.h"

#include <Adafruit_MLX90614.h>

#include <WiFi.h>

#include <WebServer.h>

#include "QuickStats.h"

#include "EEPROM.h"

#include <stdio.h>

#include <stdlib.h>

#include <sqlite3.h>

#include <SPI.h>

#include <FS.h>

#include "SPIFFS.h"

#include "bitmap.h"

#include <JPEGDecoder.h>

#define Threshold_LOW 0
#define Threshold_MED 5
#define Threshold_HIGH 10

#define display_deg 15
#define Font_size 17
#define calib_A 21
#define calib_B 25
#define calib_M 30
#define calib_C 35
#define display_screen 41
#define display_fvr 60
#define display_nofvr 100
#define Time_display 200
#define echoPin 34
#define trigPin 13

//TaskHandle_t Task1;
SemaphoreHandle_t baton;
char count = 0;
int ledPin = 19;
char detect_flag = 0;
/*---------------------library global variables-------------------------------------*/
RTC_DS3231 rtc;
TFT_eSPI tft = TFT_eSPI();
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
WebServer server(80);
sqlite3 * db1;
QuickStats stats;

char ssid[50];
char sql_data_send[5000] = {0};
float ot_array[60] = {
  0.0
};
static int ot_array_count = 0;
static float last_trig;
static float trig = 0.0;

/*----------------------------------------------------------------------------------*/
const int interruptPin = 34;
String fvr_screen;
String nrml_screen;
const int buzzer = 12;
float obj_final = 0;
float obj_final_c = 0;
char cal_on = 0;
float pulse_median = 0;
static float amb_data;
char display_flag = 0;
char screen = 0;
float font_size = 0.0;
int time_display = 0;
String ssid1;
int freq = 2000;
int channel = 0;
int resolution = 8;
char net_connect = 0;
float Threshold_T2L, Threshold_T2M, Threshold_T2H, calib_a, calib_b, calib_m, calib_c;
/*==================================EEPROM=================================================*/
void eprom_init() {
  if (!EEPROM.begin(1000)) {
    //Serial.println("Failed to initialise EEPROM");
    //Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
}

void thrshld_get() {
  EEPROM.get(Threshold_LOW, Threshold_T2L);
  EEPROM.get(Threshold_MED, Threshold_T2M);
  EEPROM.get(Threshold_HIGH, Threshold_T2H);
  Serial.println(Threshold_T2L);
  Serial.println(Threshold_T2M);
  Serial.println(Threshold_T2H);
  if (isnan(Threshold_T2L)) {
    Threshold_T2L = 30.0;
    EEPROM.put(Threshold_LOW, Threshold_T2L);
  }

  if (isnan(Threshold_T2M)) {
    Threshold_T2M = 37.5;
    EEPROM.put(Threshold_MED, Threshold_T2M);
  }

  if (isnan(Threshold_T2H)) {
    Threshold_T2H = 42.0;
    EEPROM.put(Threshold_HIGH, Threshold_T2H);
  }

}

void threshold_set(float low, float med) //set threshold values
{
  EEPROM.put(Threshold_LOW, low);
  EEPROM.put(Threshold_MED, med);
  // EEPROM.put(Threshold_HIGH,high);
  EEPROM.commit();
  tft.fillScreen(TFT_MAGENTA);
  tft.setCursor(40, 50);
  tft.setTextSize(4);
  tft.setTextColor(TFT_BLACK, TFT_MAGENTA);
  tft.println(" New ");
  tft.setCursor(40, 60);
  tft.println(" Threshold ");
  tft.setCursor(40, 70);
  tft.println(" SET ");
  delay(500);
  thrshld_get();
  delay(500);
  home_screen();
}

void display_string_get() {
  EEPROM.get(display_fvr, fvr_screen);
  EEPROM.get(display_nofvr, nrml_screen);
  
  if (isnan(display_fvr)) {
    fvr_screen = "STOP";
    EEPROM.put(display_fvr, fvr_screen);
    delay(500);
  }
  if (isnan(display_nofvr)) {
    nrml_screen = "GO";
//    EEPROM.put(display_nofvr,"GO");
    EEPROM.writeString(display_nofvr,nrml_screen);
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
  delay(500);
}

char display_deg_get() {
  EEPROM.get(display_deg, display_flag);
  if (display_flag == 255) {
    display_flag = 0;
    EEPROM.put(display_deg, display_flag);
    delay(500);
    //Serial.print("display-");
    //Serial.println(display_flag,1);
  } else {
    //Serial.print("display1-");
    //Serial.println(display_flag,1);
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
    //Serial.print("screen-");
    //Serial.println(screen,1);
  } else {
    //Serial.print("screen-");
    //Serial.println(screen,1);
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
    //Serial.print("font-");
    //Serial.println(font_size,3);
  } else {
    //Serial.print("font-");
    //Serial.println(font_size,3);
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
    //Serial.print("font-");
    //Serial.println(font_size,3);
  } else {
    //Serial.print("font-");
    //Serial.println(font_size,3);
  }
}

void display_time_set(int time_display) {
  EEPROM.put(Time_display, time_display);
  EEPROM.commit();
  delay(500);
  display_time_get();
  delay(500);
}

void calib_get() {
  EEPROM.get(calib_A, calib_a);
  EEPROM.get(calib_B, calib_b);
  EEPROM.get(calib_M, calib_m);
  EEPROM.get(calib_C, calib_c);
  if (isnan(calib_a)) {
    calib_a = 0.0;
    EEPROM.put(calib_A, calib_a);
  }
  if (isnan(calib_b)) {
    calib_b = 0.0;
    EEPROM.put(calib_B, calib_b);
  }
  if (isnan(calib_m)) {
    calib_m = 0.46;
    EEPROM.put(calib_M, calib_m);
  }
  if (isnan(calib_c)) {
    calib_c = 21.02;
    EEPROM.put(calib_C, calib_c);
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
  delay(500);
}
/*==========================================================================================*/
/*==================================================================================*/
void createSSID() {
  // char ssid[23];
  String chipid = String(WiFi.macAddress()); // The chip ID is essentially its MAC address(length: 6 bytes).
  ssid1 = "FW50_" + chipid;
  Serial.println(ssid1);

}
void set_type() {
  if (server.method() == HTTP_POST) {
    String a = server.arg("type");
    display_deg_set(a.toInt());
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  if (server.method() == HTTP_GET) {
    server.send(200, "text/plain", display_flag == 0 ? "0" : "1");
  }
}
void display_temp() {
  if (server.method() == HTTP_POST) {
    String a = server.arg("temp");
    display_screen_set(a.toInt());
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  if (server.method() == HTTP_GET) {
    server.send(200, "text/plain", screen == 0 ? "0" : "1");
  }
}

void display_font() {
  if (server.method() == HTTP_POST) {
    String a = server.arg("font");
    font_set(a.toFloat());
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  if (server.method() == HTTP_GET) {
    server.send(200, "text/plain", String(font_size));
  }
}
void set_time() { // need to implement
  if (server.method() == HTTP_POST) {
    String time_d = server.arg("time");
    display_time_set(time_d.toInt());
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  if (server.method() == HTTP_GET) {
    server.send(200, "text/plain", String(time_display));
  }
}

void o_data() {
  String val = String(pulse_median);
  char data_cap[50];
  sprintf( & data_cap[0], "{\"obj_temp\":%s}", val);
  server.send(200, "text/plain", data_cap);
}

void a_data() {
  String val = String(amb_data);
  char data_cap[50];
  sprintf( & data_cap[0], "{\"amb_temp\":%s}", val);
  server.send(200, "text/plain", data_cap);
}
void getmac() {
  String val = String(WiFi.macAddress());
  //Serial.println(val);
  server.send(200, "text/plain", val);
}
void store_threshold() {
  if (server.method() == HTTP_POST) {
    String th_l = server.arg("lower");
    String th_m = server.arg("medium");
    //String th_h=server.arg("high");
    threshold_set(th_l.toFloat(), th_m.toFloat());
    //   Serial.println("t2l:%f,t2h:%f,t2h:%f",Threshold_T2LL, Threshold_T2M,Threshold_T2H);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  if (server.method() == HTTP_GET) {
    char th_data[500];
    sprintf( & th_data[0], "{\"TH_T2LL\":%f,\"TH_T2M\":%f,\"TH_T2H\":%f}", EEPROM.readFloat(Threshold_LOW), EEPROM.readFloat(Threshold_MED), EEPROM.readFloat(Threshold_HIGH));
    server.send(200, "text/plain", String(th_data));
  }
}
void store_cal_factor() {
  if (server.method() == HTTP_POST) {
    String a = server.arg("a");
    String b = server.arg("b");
    String m = server.arg("c");
    String c = server.arg("d");
    calib_set(a.toFloat(), b.toFloat(), m.toFloat(), c.toFloat());
    //   Serial.println("t2l:%f,t2h:%f,t2h:%f",Threshold_T2LL, Threshold_T2M,Threshold_T2H);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  if (server.method() == HTTP_GET) {
    char th_data[500];
    sprintf( & th_data[0], "{\"A\":%f,\"B\":%f,\"M\":%f,\"C\":%f}", EEPROM.readFloat(calib_A), EEPROM.readFloat(calib_B), EEPROM.readFloat(calib_M), EEPROM.readFloat(calib_C));
    server.send(200, "text/plain", String(th_data));
  }
}
void time_adj() {
  if (server.method() == HTTP_POST) {
    String date = server.arg("date");
    String mnth = server.arg("month");
    String yr = server.arg("year");
    String hr = server.arg("hour");
    String mins = server.arg("min");
    String sec = server.arg("sec");
    rtc.adjust(DateTime(yr.toInt(), mnth.toInt(), date.toInt(), hr.toInt(), mins.toInt(), sec.toInt()));
    delay(500);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
}

void calibration_start() {
  cal_on = 1;
  tft.fillScreen(TFT_YELLOW);
  tft.setCursor(60, 150);
  tft.setTextSize(2.5);
  tft.println("CALIBRATING");
  server.send(200, "text/plain", "Success\n" );
}
void display_string() {
  if (server.method() == HTTP_POST) {
    String fvr = server.arg("fever");
    String nrml = server.arg("normal");
    display_string_set(fvr, nrml);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  if (server.method() == HTTP_GET) {
    char th_data[500];
    sprintf( & th_data[0], "{\"fever\":\"%s\",\"normal\":\"%s\"}", fvr_screen, nrml_screen);
    server.send(200, "text/plain", String(th_data));
  }
}
/*----------------------------------------------------------------------------------*/
void home_screen() {

  tft.fillScreen(TFT_BLUE);
  //tft.setFreeFont(LABEL2_FONT);
  //  tft.setFreeFont(FF45);
  tft.setTextColor(TFT_ORANGE);
  // tft.setCursor(70, 70);
  tft.setTextSize(4);
  // tft.println("FEVERWARN");
  drawJpeg("/fvrwrn1.jpg", 20, 50);
  drawJpeg("/mslogo.jpg", 20, 250);
  tft.setCursor(70, 150);
  //  tft.setFreeFont(LABEL2_FONT);
  tft.setTextSize(3);
  tft.println("READY");
  tft.setCursor(40, 200);
  tft.println("FOR SCAN");
}
/*=======================================database=========================================*/
#define FORMAT_SPIFFS_IF_FAILED true
const char * data = "Callback function called";
static int callback(void * data, int argc, char ** argv, char ** azColName) {
  int i;
  Serial.printf("%s: ", (const char * ) data);
  for (i = 0; i < argc; i++) {
    if (net_connect == 1) {
      sprintf( & sql_data_send[strlen(sql_data_send)], "\"%s\":%s,", azColName[i], argv[i]);
    }
    Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  Serial.printf("\n");
  if (net_connect == 1) {
    sprintf( & sql_data_send[strlen(sql_data_send) - 1], "},{");
  }
  return 0;
}

int db_open(const char * filename, sqlite3 ** db) {
  int rc = sqlite3_open(filename, db);
  if (rc) {
    Serial.printf("Can't open database: %s\n", sqlite3_errmsg( * db));
    return rc;
  } else {
    Serial.printf("Opened database successfully\n");
  }
  return rc;
}
char * zErrMsg = 0;
int db_exec(sqlite3 * db, const char * sql) {
  Serial.println(sql);
  long start = micros();
  int rc = sqlite3_exec(db, sql, callback, (void * ) data, & zErrMsg);
  if (rc != SQLITE_OK) {
    Serial.printf("SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    Serial.printf("Operation done successfully\n");
  }
  Serial.print(F("Time taken:"));
  Serial.println(micros() - start);
  return rc;
}

void init_database() {
  if (db_open("/spiffs/all_data.db", & db1))
    return;
  int rc = db_exec(db1, "CREATE TABLE IF NOT EXISTS fvr_data (detection_time INTEGER, temperature REAL,raw_temp REAL,amb_temp real,corebody_temp REAL,corebody_error_pos REAL,corebody_error_neg,is_fever INTEGER);");
  if (rc != SQLITE_OK) {
    sqlite3_close(db1);
    return;
  }
  sqlite3_close(db1);
}
/*
 * fver_stat=1-->fever condition
 * fver_stat=0-->normal condition
 */
void store_fvrdata(unsigned long long time_data, float obj_data, float raw_temp, float amb_temp, float corebody_temp, float corebody_error_pos, float corebody_error_neg, int fver_stat) {
  char sql_string[150];
  if (db_open("/spiffs/all_data.db", & db1))
    return;
  sprintf( & sql_string[0], "INSERT INTO fvr_data VALUES (%d,%f,%f,%f,%f,%f,%f,%d);", time_data, obj_data, raw_temp, amb_temp, corebody_temp, corebody_error_pos, corebody_error_neg, fver_stat);
  int rc = db_exec(db1, sql_string);
  if (rc != SQLITE_OK) {
    sqlite3_close(db1);
    return;
  }
  sqlite3_close(db1);
}

int rc;
sqlite3_stmt *res;
int rec_count = 0;
const char *tail;
void all_data() {
  String resp;
  if (db_open("/spiffs/all_data.db", &db1))
       return;
  String sql = F("SELECT * FROM fvr_data");
  int step_res;
  if (sql.length() > 0) {      
      rc = sqlite3_prepare_v2(db1, sql.c_str(), 1000, &res, &tail);
      if (rc != SQLITE_OK) {
        Serial.println("Failed to fetch data");
        return;
      }
      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server.sendContent("[");
      step_res = sqlite3_step(res);
      while (step_res != SQLITE_DONE && step_res != SQLITE_ERROR) {
        step_res = sqlite3_step(res);        
        if (step_res == SQLITE_ROW) {
          int cols = sqlite3_column_count(res);          
          resp += "{";
          for (int i = 0; i < cols; i++) {
          resp += (const char *) sqlite3_column_text(res, i);
          if(i==(cols-1)){
            resp += "},";
          }else{
              resp += ",";
          }
        }
       }
       server.sendContent(resp);
       }
       sprintf(&resp[resp.length()-1],"]");
       server.sendContent(resp);
       sqlite3_finalize(res);
    }
}


//void all_data() {
//  if (db_open("/spiffs/all_data.db", & db1))
//    return;
//  net_connect = 1;
//  sprintf( & sql_data_send[0], "[{");
//  int rc = db_exec(db1, "SELECT * FROM fvr_data");
//  if (rc != SQLITE_OK) {
//    sqlite3_close(db1);
//    return;
//  }
//  sprintf( & sql_data_send[strlen(sql_data_send)], "}]");
//  Serial.println(sql_data_send);
//  server.send(200, "text/plain", sql_data_send);
//  sqlite3_close(db1);
//  net_connect = 0;
//
//}
void page_data() {
  String page, page_size;
  //if (server.method() == HTTP_POST){
  page = server.arg("page");
  page_size = server.arg("size");
  Serial.println(page_size);
  // }
  if (db_open("/spiffs/all_data.db", & db1))
    return;
  net_connect = 1;
  char q_ery[500] = {
    0
  };
  Serial.println(page_size);
  sprintf( & q_ery[0], "SELECT * FROM fvr_data order by detection_time desc limit %d offset %d", page_size.toInt(), (page_size.toInt() * (page.toInt() - 1)));
  //sprintf(&q_ery[0],"SELECT * FROM fvr_data order by detection_time desc limit 10 offset 0");

  sprintf( & sql_data_send[0], "[{");
  int rc = db_exec(db1, q_ery);
  if (rc != SQLITE_OK) {
    sqlite3_close(db1);
    return;
  }
  sprintf( & sql_data_send[strlen(sql_data_send)-3], "}]");
  Serial.println(sql_data_send);
  server.send(200, "text/plain", sql_data_send);
  sqlite3_close(db1);
  net_connect = 0;

}

void del_history(){
  SPIFFS.remove("/all_data.db");
  delay(500);
}

void time_data() {
  String start_time, end_time;
  //if (server.method() == HTTP_POST){
  start_time = server.arg("start");
  end_time = server.arg("end");

  //  }
  if (db_open("/spiffs/all_data.db", & db1))
    return;
  net_connect = 1;
  char q_ery[500] = {
    0
  };
  Serial.println(start_time);
  sprintf( & q_ery[0], "SELECT * from fvr_data where detection_time >= %ld and detection_time <= %ld order by detection_time desc limit 1000", start_time.toInt(), end_time.toInt());
  sprintf( & sql_data_send[0], "[{");
  int rc = db_exec(db1, q_ery);
  if (rc != SQLITE_OK) {
    sqlite3_close(db1);
    return;
  }
  sprintf( & sql_data_send[strlen(sql_data_send)], "}]");
  Serial.println(sql_data_send);
  server.send(200, "text/plain", sql_data_send);
  sqlite3_close(db1);
  net_connect = 0;

}

void last_data() {
  if (db_open("/spiffs/all_data.db", & db1))
    return;
  net_connect = 1;
  char q_ery[500] = {0};
  sprintf( & q_ery[0], "SELECT * from fvr_data order by detection_time desc limit 1");
  sprintf( & sql_data_send[0], "[{");
  int rc = db_exec(db1, q_ery);
  if (rc != SQLITE_OK) {
    sqlite3_close(db1);
    return;
  }
  sprintf( & sql_data_send[strlen(sql_data_send)-3], "}]");
  Serial.println(sql_data_send);
  server.send(200, "text/plain", sql_data_send);
  sqlite3_close(db1);
  net_connect = 0;

}

void calibration() {
  String y1 = server.arg("key1");
  String x1 = server.arg("o1");
  String y2 = server.arg("key2");
  String x2 = server.arg("o2");
  float m = 0;
  float c = 0;
  m = ((y1.toFloat() - y2.toFloat()) / (x1.toFloat() - x2.toFloat()));
  c = ((x2.toFloat() * y1.toFloat()) - (x1.toFloat() * y2.toFloat())) / (x2.toFloat() - x1.toFloat());
  calib_set(0.0, 0.0, m, c);
  String val = "{\"m_value\":" + String(m) + ", \"c_value\":" + String(c) + "}";
  server.send(200, "application/json", val);
  File history = SPIFFS.open("/Calibration_history.txt", FILE_APPEND);
  if (!history) {
    Serial.println("Failed to open file for writing");
    return;
  }
  DateTime now = rtc.now();
  String cal_data = "{\"Time\":" + String(now.unixtime() - 19800) + "," + "\"m_value\":" + String(m) + ",\"c_value\":" + String(c) + "}";
  history.println(cal_data);
  history.close();
  cal_on = 0;
  home_screen();
}

void cal_history() {
  File history = SPIFFS.open("/Calibration_history.txt");
  server.streamFile(history, "text/html");
  history.close();
}
//void all_data() {
//   if (db_open("/spiffs/all_data.db", &db1))
//       return;
//  net_connect=1;
//  sprintf(&sql_data_send[0],"[{");
//  int rc = db_exec(db1, "SELECT * FROM fvr_data");
//  if (rc != SQLITE_OK) {
//       sqlite3_close(db1);
//       return;
//   }
//   sprintf(&sql_data_send[strlen(sql_data_send)],"]");
//   Serial.println(sql_data_send);
//   server.send(200, "text/plain",sql_data_send);
//   sqlite3_close(db1);
//   net_connect=0;
//
//}
/*======================================================================================*/
void handleRoot() {
  // server.sendHeader("Location", "/index.html",true);   //Redirect to our html web page
  //server.send(302, "text/plane","");
  //Serial.println("GPIO4 Status: OFF | GPIO5 Status: OFF");
  server.send(200, "text/html", "Arpita");
}

void change_screen(String display_data, float font_size, char unit, char show_display) {

  tft.setTextSize(font_size);
  tft.drawCentreString(display_data, 120, 40, 4);
  tft.setCursor(60, 150);
  tft.setTextSize(3);
  if (unit == 1) {
    obj_final = (obj_final * 9 / 5) + 32;
    amb_data = (amb_data * 9 / 5) + 32;
  }
  if (show_display == 1) { // 0=>C,1=>F
    if (unit == 1) {
      tft.print(obj_final);
      tft.println("F");
    } else {
      tft.print(obj_final);
      tft.println("C");
    }
  }
  if (time_display == 1) {
    tft.setTextSize(2.5);
    DateTime now = rtc.now();
//    tft.setCursor(60, 210);
//    tft.printf("%02d/%02d/%02d", now.year(), now.month(), now.day());
    tft.setCursor(60, 230);
    tft.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  }

}
//=================================core body==============================================//
float core_body,core_body_upper,core_body_lower;
void predict_core_body(float median,float ambobj)
{
 float e1 = -0.0111;
 float e2 = 0.435;
 float coeff[2]={0.4851, 18.911};
 float compensator ,error_upper,error_lower,error_band;
 core_body= median* coeff[0] + coeff[1];
 error_band= abs(e1 * ambobj + e2);
 if(median<36.6){
   core_body=0.4851*median+18.911;
 }else{ 
  core_body=0.4851*median+20.911;  
 }
if(core_body<36){
 compensator = random(0,error_band*100)/100;
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
  char i = 0;
  char pulse_count = 0;
  float temp_data[50];
  float pulse_mean, pulse_std;float median_95,median_90,median_50;
  //if((ot_nw>tr_nw)&&(detect_flag==0)){
  while (((millis() - previousMillis) <= 1000) && (digitalRead(interruptPin) == 0)) {
    // omron.data_omron();
    temp_data[i++] = mlx.readObjectTempC();
//    Serial.println(mlx.readObjectTempC());
    amb_data = mlx.readAmbientTempC();
    delay(100);
    pulse_count++;
  }
  if(cal_on == 1){
    if (pulse_count >= 10) {
       pulse_median = stats.median(temp_data, i);
    }
  }
  else if (cal_on == 0) {
  if (pulse_count >= 10) {
    detect_flag = 1;
    pulse_median = stats.median(temp_data, i);
    pulse_mean = stats.average(temp_data, i);
    pulse_std = stats.stdev(temp_data, i);
    i = 0;
    predict_core_body(pulse_median,amb_data);
    if ((pulse_std / pulse_mean) * 100 < 20) {
      Serial.print(pulse_median);
      if((calib_m==0.0)&&(calib_c==0.0)){
         obj_final = calibrate_readings(pulse_median);
      }else{
        obj_final = (calib_m * pulse_median) + calib_c;
      }      
      obj_final_c =obj_final;
      
        if ((obj_final > Threshold_T2M) && (obj_final <= Threshold_T2H)) {
          //----------------fever case----------------------------//
          //        ledcWriteTone(channel, 500);
          //        ledcWrite(channel, 150);
          tft.fillScreen(TFT_RED);
          change_screen(fvr_screen, font_size, display_flag, screen);
          for (int j = 0; j <=2; j++) {
            tone(buzzer, 2000, 1500, channel);
            delay(500);
            noTone(buzzer, channel);
          }
          DateTime now = rtc.now();         
          // tft.print(now.timestamp());
          //Serial.printf("%d/%d/%d,%d:%d:%d,%f,Fever",now.year(),now.month(),now.day(),now.hour(),now.minute(),now.second(),obj_final);
          store_fvrdata(now.unixtime() - 19800, obj_final_c, pulse_median, amb_data,core_body,core_body_upper,core_body_lower, 1);
          delay(2000);
          ledcWriteTone(channel, 2000);
          ledcWrite(channel, 0);
          home_screen();
        } else if ((obj_final >= Threshold_T2L) && (obj_final <= Threshold_T2M)) {
          //----------------normal case----------------------------//
          //        ledcWriteTone(channel, 500);
          //        ledcWrite(channel, 200);     
          Serial.println("NORMAL");
          tft.fillScreen(TFT_GREEN);
          tft.setTextColor(TFT_BLACK, TFT_GREEN);
          change_screen(nrml_screen, font_size, display_flag, screen);
          tone(buzzer, 2000, 1500, channel);
          noTone(buzzer, channel);

          DateTime now = rtc.now();
          // tft.print(now.timestamp());
          //        Serial.printf(now.unixtime(),obj_final);
          store_fvrdata(now.unixtime() - 19800, obj_final_c, pulse_median, amb_data, core_body,core_body_upper,core_body_lower, 0);
          delay(2000);
          ledcWriteTone(channel, 2000);
          ledcWrite(channel, 0);
          home_screen();
        } else if (obj_final < Threshold_T2L) {
          //----------------too low case--------------------------//
          tft.fillScreen(TFT_YELLOW);
          tft.setTextColor(TFT_BLACK, TFT_YELLOW);
          tft.setCursor(40, 120);
          tft.setTextSize(2.0);
          change_screen("LOW BODY",2.5, display_flag, screen); {
            tone(buzzer, 2000, 100, channel);
            delay(1100);
            noTone(buzzer, channel);
          }
          tft.setCursor(100, 190);
          tft.setTextSize(2.5);
          //        tft.println(obj_final);        
          delay(2000);
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
          delay(2000);
          home_screen();
        }
      }
    }
  }
  pulse_count = 0;
}
/*====================================================================================================*/
void setup() {
  Serial.begin(115200);
  int rc;
  delay(500);

  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("Failed to mount file system");
    return;
  }

  //   // list SPIFFS contents
  //   File root = SPIFFS.open("/");
  //   if (!root) {
  //       Serial.println("- failed to open directory");
  //       return;
  //   }
  //   if (!root.isDirectory()) {
  //       Serial.println(" - not a directory");
  //       return;
  //   }
  //   File file = root.openNextFile();
  //   while (file) {
  //       if (file.isDirectory()) {
  //           Serial.print("  DIR : ");
  //           Serial.println(file.name());
  //       } else {
  //           Serial.print("  FILE: ");
  //           Serial.print(file.name());
  //           Serial.print("\tSIZE: ");
  //           Serial.println(file.size());
  //       }
  //       file = root.openNextFile();
  //   }
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
    rtc.adjust(DateTime(2020, 9, 28, 16, 21, 20));
    tft.setCursor(50, 50);
    tft.print("power lost");
  }
  //rtc.adjust(DateTime(2020, 9, 28, 16,23, 20));
  mlx.begin();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT  
  //  pinMode(interruptPin,INPUT_PULLUP);
  //  pinMode(ledPin,OUTPUT);
  //  digitalWrite(ledPin,1);
  digitalWrite(buzzer, 1);
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(12, channel);
  eprom_init();
  thrshld_get();
  display_deg_get();
  font_get();
  calib_get();
  display_screen_get();
  display_string_get();
  display_time_get();
  createSSID();
  home_screen();
  WiFi.softAP((const char * ) ssid1.c_str(), (const char * ) String(WiFi.macAddress()).c_str());
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.print("SSID: ");
  Serial.println(ssid1);
  server.on("/odata", o_data);
  server.on("/last_data", last_data);
  server.on("/all_data", all_data);
  server.on("/adata", a_data);
  server.on("/getmac", getmac);
  server.on("/threshold", store_threshold);
  server.on("/cal_factor", store_cal_factor);
  server.on("/display_type", set_type);
  server.on("/display_temp", display_temp);
  server.on("/display_font", display_font);
  server.on("/display_time", set_time);
  server.begin();
  server.on("/time_adj", time_adj);
  server.on("/page_data", page_data);
  server.on("/time_data", time_data);
  server.on("/calibration", calibration);
  server.on("/calibration_start", calibration_start);
  server.on("/display_string", display_string);
  server.on("/calibration_history", cal_history);
  server.on("/delete_history", del_history);
  home_screen();

}
float orig_ot = 0.0;
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

void loop() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  //  Serial.print("Distance: ");
  //  Serial.print(distance);
  //  Serial.println(" cm");
  //    Serial.println("amb_data");
  if ((distance > 3) && (distance <= 10) && (detect_flag == 0)) {
    Serial.println("arp");
    pulse_check();
  } else if ((distance >= 100) && (detect_flag == 1)) {
    detect_flag = 0;
  }else if((distance <=3)){
      tft.fillScreen(TFT_CYAN);
      tft.setCursor(40,150);
      tft.setTextSize(3);
      tft.setTextColor(TFT_BLACK, TFT_CYAN);
      tft.println("Scan Again");
      delay(2000);
  }
  server.handleClient();
}
