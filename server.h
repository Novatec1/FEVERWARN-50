#ifndef SERVER_H
#define SERVER_H
/*************************************************************************
 Title  :   C include file for server (webserver.cpp)
 Author:    Arpita Chakraborty <achakraborty@machinesense.com>  

***************************************************************************/
#include "config.h"

extern char cal_on;
extern float pulse_median;
extern char change_config;

void wifi_setup();
void server_setup();
void client_hndl();
void init_database();
void store_fvrdata(database_param_t &data);
void delete_sql_limit();

#endif
