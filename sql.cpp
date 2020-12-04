/****************************************************************************
 Title  :  sql communication library
 Author:    Arpita Chakraborty <achakraborty@machinesense.com> 
 
 DESCRIPTION
       Basic routines for storing data in database
       
*****************************************************************************/
//#include "server.h"
#include "FS.h"
#include "SPIFFS.h"
#include <WString.h> 
#include "sql.h"
/*=======================================database=========================================*/
sqlite3 * db1;
const char * data = "Callback function called";
static int callback(void * data, int argc, char ** argv, char ** azColName) {
  int i;
  //Serial.printf("%s: ", (const char * ) data);
  for (i = 0; i < argc; i++) {
    if (net_connect == 1) {
      sprintf( & sql_data_send[strlen(sql_data_send)], "\"%s\":%s,", azColName[i], argv[i]);
    }
    //Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  //Serial.printf("\n");
  if (net_connect == 1) {
    sprintf( & sql_data_send[strlen(sql_data_send) - 1], "},{");
  }
  return 0;
}

int db_open(const char * filename, sqlite3 ** db) {
  int rc = sqlite3_open(filename, db);
  if (rc) {
   // Serial.printf("Can't open database: %s\n", sqlite3_errmsg( * db));
    return rc;
  } else {
    //Serial.printf("Opened database successfully\n");
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
   // Serial.printf("Operation done successfully\n");
  }
  //Serial.print(F("Time taken:"));
  //Serial.println(micros() - start);
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
void delete_sql_limit(){
  int row_count=0;
  int min_time=0;
  if (db_open("/spiffs/all_data.db", & db1))
  return;
  char *sql = "SELECT COUNT(*) FROM fvr_data";
  int rc_b = sqlite3_prepare_v2(db1,sql, -1, &res, 0);  
  int step_res = sqlite3_step(res);
  if (step_res == SQLITE_ROW) {        
       row_count=atoi((const char*)sqlite3_column_text(res, 0));
       Serial.println(row_count);
   }
  sqlite3_finalize(res);
  //delay(100); 
  if(row_count>=40){
//  char *qsql = "SELECT min(detection_time) FROM fvr_data";
//  rc_b = sqlite3_prepare_v2(db1,qsql, -1, &res, 0);
//  step_res = sqlite3_step(res);
//  if (step_res == SQLITE_ROW) {        
//       min_time=atoi((const char*)sqlite3_column_text(res, 0));
//       Serial.println(min_time);
//   }
//   sqlite3_finalize(res);
   String qsql1 = "DELETE FROM fvr_data WHERE rowid=min(rowid)";
   //qsql1 += String(min_time);
    //Serial.println(qsql1);
   rc_b = db_exec(db1, qsql1.c_str());
   if(rc_b != SQLITE_OK )
   {
     //Serial.println("ERROr in DELETe");
     sqlite3_close(db1);
   }
  
  }
  sqlite3_close(db1);
  delay(100); 
 }
