/****************************************************************************
 Title  :  webserver communication library
 Author:    Arpita Chakraborty <achakraborty@machinesense.com> 
 
 DESCRIPTION
       Basic routines for communication with mobile app
       
*****************************************************************************/
#include "server.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WString.h> 
char ssid[50];
String ssid1;

WebServer server(80);

void createSSID() {
  String chipid = String(WiFi.macAddress()); // The chip ID is essentially its MAC address(length: 6 bytes).
  ssid1 = "FW50_" + chipid;
//  Serial.println(ssid1);
}

void wifi_setup(){
  WiFi.softAP((const char * ) ssid1.c_str(), (const char * ) String(WiFi.macAddress()).c_str());
  IPAddress myIP = WiFi.softAPIP();
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
    tft.fillScreen(TFT_MAGENTA);
    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK, TFT_MAGENTA);
    tft.setCursor(30, 150);
    tft.println("  SAVED ");
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
    tft.fillScreen(TFT_MAGENTA);
    tft.setTextSize(4);
    tft.setTextColor(TFT_BLACK, TFT_MAGENTA);
    tft.setCursor(40, 150);
    tft.println(" SAVED ");
    threshold_set(th_l.toFloat(), th_m.toFloat());
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    home_screen();
  }
  if (server.method() == HTTP_GET) {
    char th_data[500];
    sprintf( & th_data[0], "{\"TH_T2LL\":%f,\"TH_T2M\":%f}", EEPROM.readFloat(Threshold_LOW), EEPROM.readFloat(Threshold_MED));
    server.send(200, "text/plain", String(th_data));
  }
}
void store_cal_factor() {
  if (server.method() == HTTP_POST) {
    String a = server.arg("a");
    String b = server.arg("b");
    String m = server.arg("c");
    String c = server.arg("d");
    tft.fillScreen(TFT_MAGENTA);
    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK, TFT_MAGENTA);
    tft.setCursor(30, 150);
    tft.println(" SAVED ");
    calib_set(a.toFloat(), b.toFloat(), m.toFloat(), c.toFloat());
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    home_screen();
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
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(30, 150);
  tft.setTextSize(3);
  tft.println("CALIBRATING");
  server.send(200, "text/plain", "Success\n" );
}
void display_string() {
  if (server.method() == HTTP_POST) {
    String fvr = server.arg("fever");
    String nrml = server.arg("normal");
    display_string_set(fvr, nrml);
    tft.fillScreen(TFT_MAGENTA);
    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK, TFT_MAGENTA);
    tft.setCursor(40, 150);
    tft.println("  SAVED ");
    delay(500);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    home_screen();
  }
  if (server.method() == HTTP_GET) {
    char th_data[500];
    sprintf( & th_data[0], "{\"fever\":\"%s\",\"normal\":\"%s\"}", fvr_screen, nrml_screen);
    server.send(200, "text/plain", String(th_data));
  }
}

void core_fist_save(int data_switch)
{
  File all_history = SPIFFS.open("/core_switch.txt", FILE_WRITE);
  if (!all_history) {
    //Serial.println("Failed to open file for writing");
    return;
  }
  all_history.println(data_switch);
  all_history.close();
}

int core_fist_get()
{
  int ret_data;
  File all_history = SPIFFS.open("/core_switch.txt", FILE_READ);
  ret_data=all_history.read();  
  all_history.close();
  return ret_data;
}
//=================================================================================//
void core_fist(){
  if (server.method() == HTTP_POST) {
    String fvr = server.arg("num");
    core_fist_save(fvr.toInt());
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  if (server.method() == HTTP_GET) {
    char th_data[500];
    sprintf( & th_data[0], "{\"SWITCH\":\"%d\"}",core_fist_get()-48);
    server.send(200, "text/plain", String(th_data));
  }
  
}

void all_data() {  
  File myFile = SPIFFS.open("/all_data_history.txt");
   server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); 
  if(myFile)
  {
    server.sendContent("[");
//    server.streamFile(myFile, "text/html");
   while(myFile.available())
   {
    server.sendContent(myFile.readString());
   }
   server.sendContent("]");
   myFile.close();
    
  }
  else
  {
   // Serial.println("error opening test.txt");
  }
  

}
void page_data() {
  String page, page_size;
  //if (server.method() == HTTP_POST){
  page = server.arg("page");
  page_size = server.arg("size");
  //Serial.println(page_size);
  // }
  if (db_open("/spiffs/all_data.db", & db1))
    return;
  net_connect = 1;
  char q_ery[500] = {
    0
  };
//  Serial.println(page_size);
  sprintf( & q_ery[0], "SELECT * FROM fvr_data order by detection_time desc limit %d offset %d", page_size.toInt(), (page_size.toInt() * (page.toInt() - 1)));
  //sprintf(&q_ery[0],"SELECT * FROM fvr_data order by detection_time desc limit 10 offset 0");

  sprintf( & sql_data_send[0], "[{");
  int rc = db_exec(db1, q_ery);
  if (rc != SQLITE_OK) {
    sqlite3_close(db1);
    return;
  }
  sprintf( & sql_data_send[strlen(sql_data_send)-3], "}]");
 // Serial.println(sql_data_send);
  server.send(200, "text/plain", sql_data_send);
  sqlite3_close(db1);
  net_connect = 0;

}

void del_history(){
  SPIFFS.remove("/all_data.db");
  SPIFFS.remove("/all_data_history.txt");
  delay(500);
  server.send(200, "text/plain", sql_data_send);
  ESP.restart();
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
  char q_ery[500] = {0};
//  Serial.println(start_time);
  sprintf( & q_ery[0], "SELECT * from fvr_data where detection_time >= %ld and detection_time <= %ld order by detection_time desc limit 1000", start_time.toInt(), end_time.toInt());
  sprintf( & sql_data_send[0], "[{");
  int rc = db_exec(db1, q_ery);
  if (rc != SQLITE_OK) {
    sqlite3_close(db1);
    return;
  }
  sprintf( & sql_data_send[strlen(sql_data_send)], "}]");
  //Serial.println(sql_data_send);
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
 // Serial.println(sql_data_send);
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
   // Serial.println("Failed to open file for writing");
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

void all_data_store(unsigned long long time_data, float obj_data, float raw_temp, float amb_temp, float corebody_temp, float corebody_error_pos, float corebody_error_neg, int fver_stat){
   char c;unsigned long count=0;
  File all_history = SPIFFS.open("/all_data_history.txt", FILE_APPEND);
  if (!all_history) {
//    Serial.println("Failed to open file for writing");
    return;
  }
  char data_all[500]={0};
  count=all_history.size();
//  Serial.println(count);
  if(count>=73000){
    SPIFFS.remove("all_data_history.txt");
    delay(500);
    all_history = SPIFFS.open("/all_data_history.txt", FILE_APPEND);
  }else if(count==0){
  sprintf(&data_all[0],"{\"detection_time\":%lld,\"temperature\":%0.2f,\"raw_temp\":%0.2f,\"amb_temp\":%0.2f,\"corebody_temp\":%0.2f,\"corebody_error_pos\":%f,\"corebody_error_neg\":%f,\"is_fever\":%d}",time_data,obj_data,raw_temp,amb_temp,corebody_temp,corebody_error_pos,corebody_error_neg,fver_stat);
  all_history.println(data_all);
  }else{
   sprintf(&data_all[0],",{\"detection_time\":%lld,\"temperature\":%0.2f,\"raw_temp\":%0.2f,\"amb_temp\":%0.2f,\"corebody_temp\":%0.2f,\"corebody_error_pos\":%f,\"corebody_error_neg\":%f,\"is_fever\":%d}",time_data,obj_data,raw_temp,amb_temp,corebody_temp,corebody_error_pos,corebody_error_neg,fver_stat);
   all_history.println(data_all);
  }
  all_history.close();
}


void handleRoot() {
   server.send(200, "text/html", "Arpita");
}

void server_setup(){
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
  server.on("/core_fist_switch", core_fist);
}

void client_hndl(){
  server.handleClient();
}
