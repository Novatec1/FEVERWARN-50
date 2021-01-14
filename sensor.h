#ifndef SENSOR_H
#define SENSOR_H
/*************************************************************************
 Title  :   C include file for the sensor (sensor.cpp)
 Author:    Arpita Chakraborty <achakraborty@machinesense.com>  

***************************************************************************/
extern char cal_on;
extern char detect_flag;
extern char db_ready;
extern unsigned long int start_time;
extern QueueHandle_t queue;
extern int queueSize;
void sensor_setup();
int proximity();
float get_data();
void pulse_check(int distance);
float sensor_calibration(unsigned long duration);
#endif
