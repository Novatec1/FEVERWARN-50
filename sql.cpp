/****************************************************************************
 Title  :  sql communication library
 Author:    Arpita Chakraborty <achakraborty@machinesense.com> 
 
 DESCRIPTION
       Basic routines for storing data in database
       
*****************************************************************************/
#include <sqlite3.h>
#include <ArduinoJson.h>
#include "sql.h"

/*=======================================database=========================================*/
sqlite3 * db1;

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
  const char *query = "CREATE TABLE IF NOT EXISTS fvr_data (detection_time INTEGER, temperature REAL,raw_temp REAL,amb_temp real,corebody_temp REAL,corebody_error_pos REAL,corebody_error_neg,is_fever INTEGER);";
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
   Serial.printf("SQL error");
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
 
int callback(void *data_not,int argc, char ** argv, char ** azColName,) {
  int i;
  data_not=0;
  File file=SPIFFS.open(SEND_JSON_FILE,FILE_APPEND);
  StaticJsonDocument<512> doc;
  for (i = 0; i < argc; i++) {
    doc[azColName[i]]=argv[i];
   }
   if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  file.close();
  return 0;
}
int db_exec(sqlite3 * db, const char * sql) {
  char *zErrMsg = 0;
  int rc = sqlite3_exec(db, sql, callback,0, &zErrMsg);
  if (rc != SQLITE_OK) {
    Serial.printf("SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return rc;
}

void all_data_fetch(const char * sql){
  if (db_open(SQL_ALL_DATA, & db1))
    return;
  int rc = db_exec(db1,sql);
  if (rc != SQLITE_OK) {
    sqlite3_close(db1);
    return;
  }
  sqlite3_close(db1);  
}
