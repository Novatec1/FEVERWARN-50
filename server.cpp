/****************************************************************************
 Title  :  webserver communication library
 Author:    Arpita Chakraborty <achakraborty@machinesense.com> 
 
 DESCRIPTION
       Basic routines for communication with mobile app
       
*****************************************************************************/
#include <sqlite3.h>
#include <ArduinoJson.h>
#include "server.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <Wstring.h>
#include "lcd.h"
#include "sensor.h"
float cal1_temp,cal2_temp,log1_temp,log2_temp;

WebServer server(80);
sqlite3 * db1;
String adjust_timezone="0";
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
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

int db_open(const char * filename, sqlite3 ** db) {
  int rc = sqlite3_open(filename, db);
  if (rc) {
     Serial.printf("Can't open database: %s\n", sqlite3_errmsg( * db));
  }
  return rc;
}

void init_database() {
  sqlite3_initialize();
  char * zErrMsg = 0;
  if (db_open(SQL_ALL_DATA, & db1))
    return;
  const char *query = "CREATE TABLE IF NOT EXISTS fvr_data (detection_time INTEGER, temperature REAL,raw_temp REAL,amb_temp real,corebody_temp REAL,corebody_error_pos REAL,corebody_error_neg REAL,is_fever INTEGER);";
  int rc =sqlite3_exec(db1,query,0,0,&zErrMsg);
  if (rc != SQLITE_OK) {
    Serial.printf("SQL error in initialisation: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);        
    sqlite3_close(db1);
    return;
  }
  sqlite3_close(db1);
}

void store_fvrdata(database_param_t &data) {
  char sql_string[500];char * zErrMsg = 0;
  if (db_open(SQL_ALL_DATA, & db1))
    return;
  sprintf(sql_string, "INSERT INTO fvr_data VALUES (%d,%f,%f,%f,%f,%f,%f,%d);", data.time_data,data.obj_data,data.raw_temp, data.amb_temp, data.corebody_temp, data.corebody_error_pos, data.corebody_error_neg, data.fver_stat);
  int rc =sqlite3_exec(db1,sql_string,0,0,&zErrMsg);
  if (rc != SQLITE_OK) {
    Serial.printf("SQL error in insert: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    sqlite3_close(db1);
    return;
  }
  sqlite3_close(db1);
}

void delete_sql_limit(){
  int row_count=0;
  int min_time=0;
  sqlite3_stmt *res;
  if (db_open(SQL_ALL_DATA, & db1))
    return;
  char *sql = "SELECT COUNT(*) FROM fvr_data";
  int rc_b = sqlite3_prepare_v2(db1,sql, -1, &res, 0);  
  int step_res = sqlite3_step(res);
  if (step_res == SQLITE_ROW) {        
       row_count=atoi((const char*)sqlite3_column_text(res, 0));
       Serial.println(row_count);
   }
  sqlite3_finalize(res); 
  if(row_count>=1000){
  char *qsql = "SELECT min(detection_time) FROM fvr_data";
  rc_b = sqlite3_prepare_v2(db1,qsql, -1, &res, 0);
  step_res = sqlite3_step(res);
  if (step_res == SQLITE_ROW) {        
       min_time=atoi((const char*)sqlite3_column_text(res, 0));
       Serial.println(min_time);
   }
   sqlite3_finalize(res);
   String qsql1 = "DELETE FROM fvr_data WHERE detection_time=";
   qsql1 += String(min_time);
   rc_b = sqlite3_exec(db1, qsql1.c_str(),0,0,0);
   if(rc_b != SQLITE_OK )
   {
     Serial.println("ERROr in DELETe");
     sqlite3_close(db1);
   }  
  }
  sqlite3_close(db1);
  delay(100); 
 }

int db_exec(sqlite3 * db, const char * sql) {
  sqlite3_stmt *res;
  int rec_count = 0;
  const char *tail;
  char buff_send[512];
  int rc = sqlite3_prepare_v2(db1, sql, 1000, &res, &tail);
  if (rc != SQLITE_OK) {
    Serial.printf("SQL error");
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  //server.sendHeader("Content-Type", "application/json"); 
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", "["); 
  int step_res = sqlite3_step(res); 
  while (step_res != SQLITE_DONE && step_res != SQLITE_ERROR) {
         if (step_res == SQLITE_ROW) {
          StaticJsonDocument<200> doc;
          for(int i=0;i<8;i++){
             doc[sqlite3_column_name(res, i)]=sqlite3_column_text(res, i);                         
          }
          serializeJsonPretty(doc,buff_send);
          server.sendContent(buff_send);
          memset(buff_send,0,512);
          step_res = sqlite3_step(res);
          Serial.println(step_res);
          if(step_res == SQLITE_DONE){
            server.sendContent("]");
          }else
            server.sendContent(",");          
        }
        
      } 
      sqlite3_finalize(res);
      sqlite3_close(db1);
  return rc;
}

void all_data_fetch(const char * sql){
  if(db_ready==0){
    db_ready=1;
  if (db_open(SQL_ALL_DATA, & db1))
    return;
  int rc = db_exec(db1,sql);
  if (rc != SQLITE_OK) {
    sqlite3_close(db1);
    return;
  }
  db_ready=0; 
 }
}


void getmac() {
  String val = String(WiFi.macAddress());
  server.send(200, "text/plain", val);
}

void o_data() {
  String val = String(get_data());
  char data_cap[50];
  sprintf( & data_cap[0], "{\"obj_temp\":%s}", val);
  server.send(200, "text/plain", data_cap);
}

void set_display_type() {
  user_config_t config;
  if (isPost()) {
    String a = server.arg("type");
    saveConfiguration("DISPLAY_DEG",a);
    change_config=1;
    save_screen();
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    delay(500);
    home_screen();
  }
  else {
    loadConfiguration(config);
    server.send(200, "text/plain", config.display_in_c == 0 ? "0" : "1");
  }
}

void display_temp() {
  user_config_t config;
  if (isPost()) {
    String a = server.arg("temp");
    saveConfiguration("DISPLAY_TEMP",a);
    save_screen();
    change_config=1;
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    delay(500);
    home_screen();
  }
  else {
    loadConfiguration(config);
    server.send(200, "text/plain",config.display_temp == 0 ? "0" : "1");
  }
}


void display_font() {
  user_config_t config;
  if (isPost()) {
    String a = server.arg("font");
    saveConfiguration("FONT_SIZE",a);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    save_screen();
    change_config=1;
    delay(500);
    home_screen();
  }
  else {
    loadConfiguration(config);
    server.send(200, "text/plain", String(config.font_size));
  }
}

void set_time() { // need to implement
  user_config_t config;
  if (isPost()) {
    String time_d = server.arg("time");
    saveConfiguration("DISPLAY_TIME",time_d);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    change_config=1;
    save_screen();
    delay(500);
    home_screen();
  }
  else {
    loadConfiguration(config);
    server.send(200, "text/plain",config.time_display == 0 ? "0" : "1");
  }
}

void store_threshold() {
  user_config_t config;
  if (isPost()) {
    String th_l = server.arg("lower");
    String th_m = server.arg("medium");
    saveConfiguration("THRESHOLD_LOW",th_l);
    saveConfiguration("THRESHOLD_MED",th_m);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    //save_screen();
    display_threshold(th_l,th_m);
    change_config=1;
    delay(500);
    home_screen();
  }
  else {
    char th_data[500];
    loadConfiguration(config);
    Serial.println(config.threshold_temp[0]);
    sprintf( & th_data[0], "{\"TH_T2LL\":%f,\"TH_T2M\":%f}", config.threshold_temp[0],config.threshold_temp[1]);
    server.send(200, "text/plain", String(th_data));
  }
}

void store_cal_factor() {
  user_config_t config;
  if (isPost()) {
    String m = server.arg("c");
    String c = server.arg("d");
    saveConfiguration("CALIB_M",m);
    saveConfiguration("CALIB_C",c);
    change_config=1;
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    save_screen();
    delay(500);
    home_screen();
  }
  else {
    char th_data[500];    
    loadConfiguration(config);
    sprintf( & th_data[0], "{\"M\":%f,\"C\":%f}",config.calibration_param[0],config.calibration_param[1]);
    server.send(200, "text/plain", String(th_data));
  }
}

void time_adj() {
  if (isPost()) {
    user_config_t config;
    String date = server.arg("date");
    String mnth = server.arg("month");
    String yr = server.arg("year");
    String hr = server.arg("hour");
    String mins = server.arg("min");
    String sec = server.arg("sec");
    adjust_timezone= server.arg("timezone");
    saveConfiguration("TIMEZONE",adjust_timezone);
    change_config=1;
    Serial.printf("%s",adjust_timezone);
    rtc.adjust(DateTime(yr.toInt(), mnth.toInt(), date.toInt(), hr.toInt(), mins.toInt(), sec.toInt()));
    delay(500);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
  }
}

void display_string() {
  user_config_t config;
  if (isPost()) {
    String fvr = server.arg("fever");
    String nrml = server.arg("normal");
    saveConfiguration("FEVER_MSG",fvr);
    saveConfiguration("NORMAL_MSG",nrml);
    save_screen();
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    save_screen();
    change_config=1;
    delay(500);
    home_screen();
  }
  else {
    char th_data[500];
    loadConfiguration(config);
    sprintf( & th_data[0], "{\"fever\":\"%s\",\"normal\":\"%s\"}", config.fever_msg, config.normal_msg);
    server.send(200, "text/plain", String(th_data));
  }
}


void core_fist(){
  user_config_t config;
  if (isPost()) {
    String fvr = server.arg("num");
    saveConfiguration("DISPLAY_COREBODY",fvr);
    server.send(200, "text/plain", "Success\n" + server.arg("plain"));
    save_screen();
    change_config=1;
    delay(500);
    home_screen();
  }
  else {
    char th_data[500];
    loadConfiguration(config);
    sprintf( & th_data[0], "{\"SWITCH\":\"%d\"}",config.core_body);
    server.send(200, "text/plain", String(th_data));
  }
}

void all_data() {
  char *sql_data="SELECT * FROM fvr_data";
  all_data_fetch(sql_data);  
}

void page_data() {
  String page, page_size;
  char q_ery[500];
  page = server.arg("page");
  page_size = server.arg("size");
  sprintf( & q_ery[0], "SELECT * FROM fvr_data order by detection_time desc limit %d offset %d", page_size.toInt(), (page_size.toInt() * (page.toInt() - 1)));
  all_data_fetch(q_ery);
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
}

void calibration_start() {
  cal_on=1;
  calibration_screen();
  server.send(200, "text/plain", "Success\n" );
}

void calibration_config(){
  String y1 = server.arg("duration");
  String x1 = server.arg("log_temp");
  String d1 = server.arg("data_point");
  Serial.println(y1);
  if(d1.toInt()==1){
    log1_temp= x1.toFloat();
    cal1_temp=sensor_calibration(y1.toInt());
   Serial.print(cal1_temp);
  }
  else if(d1.toInt()==2){
    log2_temp= x1.toFloat();
    cal2_temp=sensor_calibration(y1.toInt());
    Serial.print(cal2_temp);
  }
   server.send(200, "text/plain", "Success\n" );
}

void calibration() {
  user_config_t config;
  float m = 0;
  float c = 0;
  Serial.printf("log1_temp:%f,log2_temp:%f,cal1_temp:%f,cal2_temp:%f",log1_temp,log2_temp,cal1_temp,cal2_temp);
  m = ((log1_temp - log2_temp) / (cal1_temp - cal2_temp));
  //c = ((cal2_temp * log1_temp) - (cal1_temp * log2_temp)) / (cal2_temp - cal1_temp);
  c=(log1_temp-m*cal1_temp);
  if ((String(m), "nan")==0||(String(c), "nan") == 0)
  { 
     String val = "{\"m_value\":" + String(m) + ", \"c_value\":" + String(c) + "}";
      server.send(200, "application/json", val);
     calibration_fail_screen();
     delay(500);
    }
  else{
  saveConfiguration("CALIB_M",String(m));
  saveConfiguration("CALIB_C",String(c));
  change_config=1;
  String val = "{\"m_value\":" + String(m) + ", \"c_value\":" + String(c) + "}";
  server.send(200, "application/json", val);
  File history = SPIFFS.open(CALIBRATION_HISTORY , FILE_APPEND);
  if (!history) {
    Serial.println("Failed to open file for writing");
    return;
  }
  DateTime now = rtc.now();
  String cal_data = "{\"Time\":" + String(now.unixtime() ) + "," + "\"m_value\":" + String(m) + ",\"c_value\":" + String(c) + "}";
  history.println(cal_data);
  history.close();
  cal_on = 0;
  calibration_done_screen();
  delay(500);
  }
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
  server.on("/time_adj", time_adj);
  server.on("/page_data", page_data);
  server.on("/calibration_config", calibration_config);
  server.on("/calibration", calibration);
  server.on("/calibration_start", calibration_start);
  server.on("/display_string", display_string);
  server.on("/calibration_history", cal_history);
  server.on("/delete_history", del_history);
  server.on("/core_fist_switch", core_fist);
  server.on("/ota", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverIndex);
    });
    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin()) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      } else {
        Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
      }
    });
  server.begin();
}

void client_hndl(){
  server.handleClient();
}
