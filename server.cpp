/****************************************************************************
 Title  :  webserver communication library
 Author:    Arpita Chakraborty <achakraborty@machinesense.com> 
 
 DESCRIPTION
       Basic routines for communication with mobile app
       
*****************************************************************************/
#include "server.h"
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"
#include "lcd.h"

WebServer server(80);
int isPost(){
  if (server.method() == HTTP_POST)
    return 1;
  else
    return 0;   
}
void wifi_setup(){
  String ssid1 = "FW50_" + String(WiFi.macAddress());
  WiFi.softAP((const char * ) ssid1.c_str(), (const char * ) String(WiFi.macAddress()).c_str());
  IPAddress myIP = WiFi.softAPIP();
}

void getmac() {
  String val = String(WiFi.macAddress());
  server.send(200, "text/plain", val);
}

void o_data() {
  String val = String(pulse_median);
  char data_cap[50];
  sprintf( & data_cap[0], "{\"obj_temp\":%s}", val);
  server.send(200, "text/plain", data_cap);
}

void set_display_type() {
  user_config_t config_server;
  if (isPost()) {
    String a = server.arg("type");
    config_server.display_in_c=a.toInt();
    saveConfiguration(config_server);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  else {
    server.send(200, "text/plain", config_server.display_in_c == 0 ? "0" : "1");
  }
}

void display_temp() {
  user_config_t config_server;
  if (isPost()) {
    String a = server.arg("temp");
    config_server.display_temp=a.toInt();
    saveConfiguration(config_server);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  else {
    server.send(200, "text/plain",config_server.display_temp == 0 ? "0" : "1");
  }
}


void display_font() {
  user_config_t config_server;
  if (isPost()) {
    String a = server.arg("font");
    config_server.font_size=a.toFloat();
    saveConfiguration(config_server);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  else {
    server.send(200, "text/plain", String(config_server.font_size));
  }
}

void set_time() { // need to implement
  user_config_t config_server;
  if (isPost()) {
    String time_d = server.arg("time");
    config_server.time_display=time_d.toInt();
    saveConfiguration(config_server);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  else {
    server.send(200, "text/plain", String(config_server.time_display));
  }
}

void store_threshold() {
  user_config_t config_server;
  if (isPost()) {
    String th_l = server.arg("lower");
    String th_m = server.arg("medium");
    config_server.threshold_temp[0]=th_l.toFloat();
    config_server.threshold_temp[1]=th_m.toFloat();
    saveConfiguration(config_server);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    home_screen();
  }
  else {
    char th_data[500];
    sprintf( & th_data[0], "{\"TH_T2LL\":%f,\"TH_T2M\":%f}", config_server.threshold_temp[0],config_server.threshold_temp[1]);
    server.send(200, "text/plain", String(th_data));
  }
}

void store_cal_factor() {
  user_config_t config_server;
  if (isPost()) {
    String m = server.arg("c");
    String c = server.arg("d");
    config_server.calibration_param[0]= m.toFloat();
    config_server.calibration_param[1]= c.toFloat();
    saveConfiguration(config_server);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    home_screen();
  }
  else {
    char th_data[500];
    sprintf( & th_data[0], "{\"M\":%f,\"C\":%f}",config_server.calibration_param[0],config_server.calibration_param[1]);
    server.send(200, "text/plain", String(th_data));
  }
}

void time_adj() {
  if (isPost()) {
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
  calibration_screen();
  server.send(200, "text/plain", "Success\n" );
}

void display_string() {
  user_config_t config_server;
  if (isPost()) {
    String fvr = server.arg("fever");
    String nrml = server.arg("normal");
    strncpy(config_server.fever_msg,fvr.c_str(),10);
    strncpy(config_server.normal_msg,nrml.c_str(),10);
    saveConfiguration(config_server);
    save_screen();
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    home_screen();
  }
  else {
    char th_data[500];
    sprintf( & th_data[0], "{\"fever\":\"%s\",\"normal\":\"%s\"}", config_server.fever_msg, config_server.normal_msg);
    server.send(200, "text/plain", String(th_data));
  }
}


void core_fist(){
  user_config_t config_server;
  if (isPost()) {
    String fvr = server.arg("num");
    config_server.core_body=fvr.toInt();
    saveConfiguration(config_server);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
  else {
    char th_data[500];
    sprintf( & th_data[0], "{\"SWITCH\":\"%d\"}",config_server.core_body);
    server.send(200, "text/plain", String(th_data));
  }
}

void all_data() {
  char *sql_data="SELECT * FROM fvr_data";
  all_data_fetch(sql_data);  
  File myFile = SPIFFS.open(SEND_JSON_FILE);
  server.streamFile(myFile, "text/html");
  myFile.close();
}

void page_data() {
  String page, page_size;
  char q_ery[500];
  page = server.arg("page");
  page_size = server.arg("size");
  sprintf( & q_ery[0], "SELECT * FROM fvr_data order by detection_time desc limit %d offset %d", page_size.toInt(), (page_size.toInt() * (page.toInt() - 1)));
  all_data_fetch(q_ery);
  File myFile = SPIFFS.open(SEND_JSON_FILE);
  server.streamFile(myFile, "text/html");
  myFile.close();
}

void del_history(){
  SPIFFS.remove("/all_data.db");
  SPIFFS.remove("/all_data_history.txt");
  delay(500);
  server.send(200, "text/plain", "Success");
  ESP.restart();
}

void last_data() {  
  char q_ery[500] = {0};
  sprintf( & q_ery[0], "SELECT * from fvr_data order by detection_time desc limit 1");
  all_data_fetch(q_ery);
  File myFile = SPIFFS.open(SEND_JSON_FILE);
  server.streamFile(myFile, "text/html");
  myFile.close();
}

void calibration() {
  user_config_t config_server;
  String y1 = server.arg("key1");
  String x1 = server.arg("o1");
  String y2 = server.arg("key2");
  String x2 = server.arg("o2");
  float m = 0;
  float c = 0;
  m = ((y1.toFloat() - y2.toFloat()) / (x1.toFloat() - x2.toFloat()));
  c = ((x2.toFloat() * y1.toFloat()) - (x1.toFloat() * y2.toFloat())) / (x2.toFloat() - x1.toFloat());
  config_server.calibration_param[0]= m;
  config_server.calibration_param[0]= c;
  saveConfiguration(config_server);
  String val = "{\"m_value\":" + String(m) + ", \"c_value\":" + String(c) + "}";
  server.send(200, "application/json", val);
  File history = SPIFFS.open(CALIBRATION_HISTORY , FILE_APPEND);
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
  File history = SPIFFS.open(CALIBRATION_HISTORY );
  server.streamFile(history, "text/html");
  history.close();
}
void handleRoot() {
   server.send(200, "text/html", "Arpita");
}

void server_setup(){
  server.on("/odata", o_data);
  server.on("/last_data", last_data);
  server.on("/all_data", all_data);
  server.on("/getmac", getmac);
  server.on("/threshold", store_threshold);
  server.on("/cal_factor", store_cal_factor);
  server.on("/display_type", set_display_type);
  server.on("/display_temp", display_temp);
  server.on("/display_font", display_font);
  server.on("/display_time", set_time);
  server.begin();
  server.on("/time_adj", time_adj);
  server.on("/page_data", page_data);
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
