/*
 * string_parse.c
 *
 *  Created on: 27 Jun 2022
 *      Author: nguyenphuonglinh
 */
#include "string_parse.h"
#include "string_parse.h"
#include "esp_log.h"
#include "../common.h"
#define JSTAG "JSON"
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
	int err = 0;
	cJSON *root = cJSON_CreateObject();
	if (root == NULL)
	{
		ESP_LOGE(JSTAG, "Create JSON object root fail!\r\n");
	}
	cJSON *sensor_obj = cJSON_CreateArray();
	if (sensor_obj == NULL)
	{
		ESP_LOGE(JSTAG, "Create JSON object sensor_obj fail!\r\n");
	}
	err |= json_add_str(root, "ver", VER);
	if(smartbox_data.lat != 0 && smartbox_data.lon != 0)
	{
		err |= json_add_num(root, "lat", smartbox_data.lat);
		err |= json_add_num(root, "lon", smartbox_data.lon);
		err |= json_add_num(root, "speed", smartbox_data.speed);
		err |= json_add_num(root, "acc", smartbox_data.acc);
		err |= json_add_num(root, "time", smartbox_data.epoch);
	}
	for(int i = 0; i < smartbox_data.white_list_cnt; i++)
	{
		err |= json_add_sensor_data(sensor_obj, smartbox_data.ble_data[i]);
	}
	err |= json_add_obj(root, "sensor", sensor_obj);
	if (err)
	{
		ESP_LOGE(JSTAG,"There are some error when making JSON object!\r\n");
	}
	char *cjsonUn = cJSON_PrintUnformatted(root);
	if (cjsonUn == NULL) return;
	strcpy(msg, cjsonUn);
	free(cjsonUn);
	cJSON_Delete(root);
}
void conver_message_send_tb(char* msg, smartbox_data_t smartbox_data)
{
	int err = 0;
	cJSON *root = cJSON_CreateObject();
	char MAC_buf[25];
	char key_str[20];
	if (root == NULL)
	{
		ESP_LOGE(JSTAG, "Create JSON object root fail!\r\n");
	}
	if(smartbox_data.lat != 0 && smartbox_data.lon != 0)
	{
		err |= json_add_num(root, "lat", smartbox_data.lat);
		err |= json_add_num(root, "lon", smartbox_data.lon);
		err |= json_add_num(root, "speed", smartbox_data.speed);
		err |= json_add_num(root, "acc", smartbox_data.acc);
		err |= json_add_num(root, "time", smartbox_data.epoch);
	}
	for(int i = 0; i < smartbox_data.white_list_cnt; i++)
	{
		sprintf(MAC_buf, "%x:%x:%x:%x:%x:%x", smartbox_data.ble_data[i].mac[0], smartbox_data.ble_data[i].mac[1], smartbox_data.ble_data[i].mac[2], \
				smartbox_data.ble_data[i].mac[3], smartbox_data.ble_data[i].mac[4], smartbox_data.ble_data[i].mac[5]);
		sprintf(key_str,"mac_%d", i);
		err |= json_add_str(root, key_str, MAC_buf);
		sprintf(key_str,"frame_%d", i);
		err |= json_add_num(root, key_str, smartbox_data.ble_data[i].frame_cnt);
		sprintf(key_str,"temp_%d", i);
		err |= json_add_num(root, key_str, smartbox_data.ble_data[i].temp);
		sprintf(key_str,"pres_%d", i);
		err |= json_add_num(root, key_str, smartbox_data.ble_data[i].pres);
		sprintf(key_str,"velo_%d", i);
		err |= json_add_num(root, key_str, smartbox_data.ble_data[i].velo);
		sprintf(key_str,"bat_%d", i);
		err |= json_add_num(root, key_str, smartbox_data.ble_data[i].bat);
	}
	char *cjsonUn = cJSON_PrintUnformatted(root);
	if (cjsonUn == NULL) return;
	strcpy(msg, cjsonUn);
	free(cjsonUn);
	cJSON_Delete(root);
}

void JSON_analyze_MAC_update( char *  my_json_string, smartbox_data_t* smartbox)
{
	memset(smartbox, 0, sizeof(smartbox_data_t));
	smartbox->white_list_cnt = 0;
	int mac1, mac2, mac3, mac4, mac5, mac6;
	const cJSON *mac = NULL;
    const cJSON *macs = NULL;
    cJSON *cmd_json = cJSON_Parse(my_json_string);
    macs = cJSON_GetObjectItem(cmd_json, "MACs");
    cJSON_ArrayForEach(mac, macs)
    {
        cJSON *mac_name = cJSON_GetObjectItem(mac, "MAC");
        sscanf(mac_name->valuestring, "%x:%x:%x:%x:%x:%x", &mac1,&mac2,&mac3,&mac4,&mac5,&mac6);
        smartbox->ble_data[smartbox->white_list_cnt].mac[0] = mac1;
		smartbox->ble_data[smartbox->white_list_cnt].mac[1] = mac2;
		smartbox->ble_data[smartbox->white_list_cnt].mac[2] = mac3;
		smartbox->ble_data[smartbox->white_list_cnt].mac[3] = mac4;
		smartbox->ble_data[smartbox->white_list_cnt].mac[4] = mac5;
		smartbox->ble_data[smartbox->white_list_cnt].mac[5] = mac6;
        cJSON *pos = cJSON_GetObjectItem(mac, "pos");
        smartbox->ble_data[smartbox->white_list_cnt].pos = pos->valueint;
        smartbox->white_list_cnt++;
    }
    cJSON_Delete(cmd_json);
}
