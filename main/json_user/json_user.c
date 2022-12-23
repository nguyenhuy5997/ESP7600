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
	sprintf(MAC_buf, "%02x:%02x:%02x:%02x:%02x:%02x", sensor.mac[0], sensor.mac[1], sensor.mac[2], \
										  sensor.mac[3], sensor.mac[4], sensor.mac[5]);
	err |= json_add_str(ap_obj, "mac", MAC_buf);
	err |= json_add_num(ap_obj, "pos", sensor.pos);
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
void JSON_analyze_MAC_init( char *  my_json_string, smartbox_data_t* smartbox)
{
	memset(smartbox, 0, sizeof(smartbox_data_t));
	cJSON *monitor_json = cJSON_Parse(my_json_string);
	int mac1, mac2, mac3, mac4, mac5, mac6;
	cJSON * const macs = cJSON_GetObjectItem(monitor_json, "MAC");
	cJSON *mac;
	cJSON_ArrayForEach(mac, macs) {
		sscanf(mac->valuestring, "%x:%x:%x:%x:%x:%x", &mac1,&mac2,&mac3,&mac4,&mac5,&mac6);
		smartbox->ble_data[smartbox->white_list_cnt].mac[0] = mac1;
		smartbox->ble_data[smartbox->white_list_cnt].mac[1] = mac2;
		smartbox->ble_data[smartbox->white_list_cnt].mac[2] = mac3;
		smartbox->ble_data[smartbox->white_list_cnt].mac[3] = mac4;
		smartbox->ble_data[smartbox->white_list_cnt].mac[4] = mac5;
		smartbox->ble_data[smartbox->white_list_cnt].mac[5] = mac6;
		smartbox->white_list_cnt++;
	}
	cJSON_Delete(monitor_json);
}
void JSON_analyze_MAC_add( char *  my_json_string, smartbox_data_t* smartbox)
{
	cJSON *monitor_json = cJSON_Parse(my_json_string);
	int mac1, mac2, mac3, mac4, mac5, mac6;
	cJSON * const macs = cJSON_GetObjectItem(monitor_json, "MAC");
	cJSON *mac;
	cJSON_ArrayForEach(mac, macs) {
		int dup = 0;
		sscanf(mac->valuestring, "%x:%x:%x:%x:%x:%x", &mac1,&mac2,&mac3,&mac4,&mac5,&mac6);
		for(int i = 0; i < smartbox->white_list_cnt; i++)
		{
			if(smartbox->ble_data[i].mac[0] == mac1 && \
				smartbox->ble_data[i].mac[1] == mac2 &&\
				smartbox->ble_data[i].mac[2] == mac3 &&\
				smartbox->ble_data[i].mac[3] == mac4 &&\
				smartbox->ble_data[i].mac[4] == mac5 &&\
				smartbox->ble_data[i].mac[5] == mac6)
				{
					dup = 1;
					break;
				}
		}
		if(dup == 0)
		{
			smartbox->ble_data[smartbox->white_list_cnt].mac[0] = mac1;
			smartbox->ble_data[smartbox->white_list_cnt].mac[1] = mac2;
			smartbox->ble_data[smartbox->white_list_cnt].mac[2] = mac3;
			smartbox->ble_data[smartbox->white_list_cnt].mac[3] = mac4;
			smartbox->ble_data[smartbox->white_list_cnt].mac[4] = mac5;
			smartbox->ble_data[smartbox->white_list_cnt].mac[5] = mac6;
			smartbox->white_list_cnt++;
		}
	}
	cJSON_Delete(monitor_json);
}
void JSON_analyze_MAC_rmv( char *  my_json_string, smartbox_data_t* smartbox)
{
	cJSON *monitor_json = cJSON_Parse(my_json_string);
	int mac1, mac2, mac3, mac4, mac5, mac6;
	cJSON * const macs = cJSON_GetObjectItem(monitor_json, "MAC");
	cJSON *mac;
	int index = 0;
	cJSON_ArrayForEach(mac, macs) {
		sscanf(mac->valuestring, "%x:%x:%x:%x:%x:%x", &mac1,&mac2,&mac3,&mac4,&mac5,&mac6);
		for(int i = 0; i < smartbox->white_list_cnt; i++)
		{
			if(smartbox->ble_data[i].mac[0] == mac1 && \
				smartbox->ble_data[i].mac[1] == mac2 &&\
				smartbox->ble_data[i].mac[2] == mac3 &&\
				smartbox->ble_data[i].mac[3] == mac4 &&\
				smartbox->ble_data[i].mac[4] == mac5 &&\
				smartbox->ble_data[i].mac[5] == mac6)
			{
				for(int j = i; j < smartbox->white_list_cnt; j++)
				{
					smartbox->ble_data[j].mac[0] = smartbox->ble_data[j+1].mac[0];
					smartbox->ble_data[j].mac[1] = smartbox->ble_data[j+1].mac[1];
					smartbox->ble_data[j].mac[2] = smartbox->ble_data[j+1].mac[2];
					smartbox->ble_data[j].mac[3] = smartbox->ble_data[j+1].mac[3];
					smartbox->ble_data[j].mac[4] = smartbox->ble_data[j+1].mac[4];
					smartbox->ble_data[j].mac[5] = smartbox->ble_data[j+1].mac[5];
				}
				smartbox->white_list_cnt--;
				break;
			}
		}
		index++;
	}
	cJSON_Delete(monitor_json);
}
