/*
 * string_parse.c
 *
 *  Created on: 27 Jun 2022
 *      Author: nguyenphuonglinh
 */
#include "string_parse.h"
void getSubStrig(char *source, char *start, char *end, char *out)
{
	int j = 0, k = 0, index_start = 0, index_end = 0;
	for(int i = 0; i < strlen(source); i++)
	{
		if (source[i] == start[j])
		{
			j++;
			if (j == strlen(start))
			{
			    index_start = i + 1;
			}
		}
		else j = 0;

		if( source[i] == end[k])
		{
			k++;
			if (k == strlen(end))
			{
				index_end = i;
			}
		}
		else
		{
		    k = 0;
		}
	}
	strncpy(out, source + index_start, index_end - index_start);
}
void parse_ble_msg(uint8_t data_byte[31], sensor_data_t * sensor_data)
{
    memcpy(sensor_data->data, data_byte, 31);
    sensor_data->data_len = data_byte[7];
    sensor_data->frame_cnt = data_byte[8] << 16 | data_byte[9] << 8 | data_byte[10];
    sensor_data->alarm = data_byte[11];
    sensor_data->temp = data_byte[12];
    sensor_data->pres =data_byte[13] << 8 | data_byte[14];
    sensor_data->velo = data_byte[15] << 8 | data_byte[16];
    sensor_data->bat =data_byte[17];
}

void conver_message_send(char* msg, smartbox_data_t smartbox_data)
{
	cJSON *root = cJSON_CreateObject();
	cJSON *sensor_obj = cJSON_CreateArray();
	cJSON_AddNumberToObject(root, "lat", smartbox_data.lat);
	cJSON_AddNumberToObject(root, "lon", smartbox_data.lon);
	cJSON_AddNumberToObject(root, "speed", smartbox_data.speed);
	cJSON_AddNumberToObject(root, "acc", smartbox_data.acc);
	cJSON_AddNumberToObject(root, "time", smartbox_data.epoch);
	for(int i = 0; i < 4; i++)
	{
		json_add_sensor_data(sensor_obj, smartbox_data.ble_data[i]);
	}
	json_add_obj(root, "sensor", sensor_obj);
	strcpy(msg, cJSON_PrintUnformatted(root));
}
