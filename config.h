#ifndef CONFIG_H
#define CONFIG_H

/*************************************************************************
 Title  :   H include file for User Configuration
 Author:    Arpita Chakraborty <achakraborty@machinesense.com>  

***************************************************************************/

#include "FS.h"
#include "SPIFFS.h"
#include <WString.h> 
#include "config.h"

#ifdef FEV_50
#define echoPin 34
#define trigPin 13
#define buzzer  12
#else
#define echoPin 34
#define trigPin 32
#define buzzer  33
#endif

#define BUZZER_CHANNEL              0
#define SERVER_PORT                 80
#define USER_CONFIG                "/user_config.txt"
#define CALIBRATION_HISTORY        "/calibration_history.txt"
#define SQL_ALL_DATA               "/spiffs/all_data.db"
#define SEND_JSON_FILE             "/send_json.txt"

typedef struct {
    unsigned long long time_data ;
    float obj_data;
    float raw_temp;
    float amb_temp;
    float corebody_temp;
    float corebody_error_pos;
    float corebody_error_neg;
    int fver_stat;
}database_param_t;

typedef struct {
    float threshold_temp[3] ;
    float calibration_param[2] ;
    char fever_msg[10];
    char normal_msg[10];
    char display_in_c ;
    char display_temp;
    float font_size ;
    char time_display ;
    char core_body;    
}user_config_t;

void loadConfiguration(user_config_t &config);
void saveConfiguration(user_config_t &config);
#endif
