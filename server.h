#ifndef SERVER_H
#define SERVER_H
/*************************************************************************
 Title  :   C include file for server (webserver.cpp)
 Author:    Arpita Chakraborty <achakraborty@machinesense.com>  

***************************************************************************/
#include "sql.h"

extern char cal_on;
extern float pulse_median;

void wifi_setup();
void server_setup();
void client_hndl();

#endif
