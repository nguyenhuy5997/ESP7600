/*
 * json_user.h
 *
 *  Created on: 10 Jun 2022
 *      Author: nguyenphuonglinh
 */
#include "cJSON.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "common.h"
#include <sys/time.h>
int json_add_num(cJSON *parent, const char *str, const double item);
int json_add_str(cJSON *parent, const char *str, const char *item);
int json_add_ap(cJSON *aps, const char * bssid, int rssi);
int json_add_obj(cJSON *parent, const char *str, cJSON *item);
int json_add_sensor_data(cJSON *aps, sensor_data_t sensor);
void JSON_analyze_sub(char* my_json_string, long* Timestamp);
