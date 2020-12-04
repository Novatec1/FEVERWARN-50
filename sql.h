#ifndef SQL_H
#define SQL_H
/*************************************************************************
 Title  :   C include file for server (webserver.cpp)
 Author:    Arpita Chakraborty <achakraborty@machinesense.com>  

***************************************************************************/
#include <sqlite3.h>
extern char net_connect;
extern char sql_data_send[5000];
extern sqlite3 *db1;

int db_open(const char * filename, sqlite3 ** db);
int db_exec(sqlite3 * db, const char * sql);
void init_database();
void store_fvrdata(unsigned long long time_data, float obj_data, float raw_temp, float amb_temp, float corebody_temp, float corebody_error_pos, float corebody_error_neg, int fver_stat);
void delete_sql_limit();

#endif
