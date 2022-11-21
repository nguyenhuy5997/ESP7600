/*
 * json_user.c
 *
 *  Created on: 10 Jun 2022
 *      Author: nguyenphuonglinh
 */

#include "json_user.h"

int json_add_obj(cJSON *parent, const char *str, cJSON *item)
{
    cJSON_AddItemToObject(parent, str, item);

    return 0;
}

int json_add_str(cJSON *parent, const char *str, const char *item)
{
    cJSON *json_str;

    json_str = cJSON_CreateString(item);
    if (json_str == NULL)
    {
        //return -ENOMEM;
    	return 0;
    }

    return json_add_obj(parent, str, json_str);
}

int json_add_num(cJSON *parent, const char *str, const double item)
{
    cJSON *json_num;

    json_num = cJSON_CreateNumber(item);
    if (json_num == NULL)
    {
    	return 0;
        //return -ENOMEM;
    }

    return json_add_obj(parent, str, json_num);
}

int json_add_ap(cJSON *aps, const char * bssid, int rssi)
{
    int err = 0;
    cJSON *ap_obj = cJSON_CreateObject();
    err |= json_add_str(ap_obj, "M", bssid);
    err |= json_add_num(ap_obj, "S", rssi);
    cJSON_AddItemToArray(aps, ap_obj);
    return err;
}
int json_add_sensor_data(cJSON *aps, sensor_data_t sensor)
{
	int err = 0;
	cJSON *ap_obj = cJSON_CreateObject();
	char MAC_buf[25];
	sprintf(MAC_buf, "%x:%x:%x:%x:%x:%x", sensor.mac[0], sensor.mac[1], sensor.mac[2], \
										  sensor.mac[3], sensor.mac[4], sensor.mac[5]);
	err |= json_add_str(ap_obj, "mac", MAC_buf);
	err |= json_add_num(ap_obj, "frame", sensor.frame_cnt);
	err |= json_add_num(ap_obj, "temp", sensor.temp);
	err |= json_add_num(ap_obj, "pres", sensor.pres);
	err |= json_add_num(ap_obj, "velo", sensor.velo);
	err |= json_add_num(ap_obj, "bat", sensor.bat);
	cJSON_AddItemToArray(aps, ap_obj);
	return err;
}
void JSON_analyze_sub(char* my_json_string, long* Timestamp)
{
	cJSON *root = cJSON_Parse(my_json_string);
	cJSON *current_element = NULL;
	cJSON_ArrayForEach(current_element, root)
	{
		if (current_element->string)
		{
			const char* string = current_element->string;
			if(strcmp(string, "T") == 0)
			{
				*Timestamp = atol(current_element->valuestring);
				struct timeval epoch = {*Timestamp, 0};
				struct timezone utc = {0,0};
				settimeofday(&epoch, &utc);
			}
		}
	}
	cJSON_Delete(root);
}
