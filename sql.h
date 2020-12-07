#ifndef SQL_H
#define SQL_H
/*************************************************************************
 Title  :   C include file for sql(sql.cpp)
 Author:    Arpita Chakraborty <achakraborty@machinesense.com>  

***************************************************************************/
#include "config.h"

void init_database();
void store_fvrdata(database_param_t &data);
void delete_sql_limit();
void all_data_fetch(const char * sql);
#endif
