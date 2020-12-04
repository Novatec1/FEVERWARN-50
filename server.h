#ifndef SERVER_H
#define SERVER_H
/*************************************************************************
 Title  :   C include file for server (webserver.cpp)
 Author:    Arpita Chakraborty <achakraborty@machinesense.com>  

***************************************************************************/

#include "eeprom.h"
#include "lcd.h"
#include "sql.h"

extern char cal_on;
extern float pulse_median;

void createSSID();
void wifi_setup();
void set_type();
void display_temp();
void display_font();
void set_time();
void o_data();
void a_data();
void getmac();
void store_threshold();
void store_cal_factor();
void time_adj() ;
void calibration_start();
void display_string() ;
void core_fist_save(int data_switch);
void core_fist();
int core_fist_get();
void all_data();
void page_data();
void del_history();
void time_data();
void last_data();
void calibration() ;
void cal_history();
void all_data_store(unsigned long long time_data, float obj_data, float raw_temp, float amb_temp, float corebody_temp, float corebody_error_pos, float corebody_error_neg, int fver_stat);
void handleRoot();
void server_setup();
void client_hndl();
#endif
