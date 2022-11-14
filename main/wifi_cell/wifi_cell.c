/*
 * wifi_cell.c
 *
 *  Created on: 10 Jun 2022
 *      Author: nguyenphuonglinh
 */

#include "wifi_cell.h"
#define TAG "ESP_WIFI"
#define DEFAULT_SCAN_LIST_SIZE 10

void wifi_scan(apInfor msg_ap_infor[], uint8_t* msg_ap_count)
{
	uint16_t ap_count;
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	uint16_t number = DEFAULT_SCAN_LIST_SIZE;
	wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_start();
	esp_wifi_scan_start(NULL, true);
	esp_wifi_scan_get_ap_records(&number, ap_info);
	esp_wifi_scan_get_ap_num(&ap_count);
	ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);

	if(ap_count > 0)
	{
		if(ap_count > 10) *msg_ap_count = DEFAULT_SCAN_LIST_SIZE;
		for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++)
		{
			msg_ap_infor[i].bssid[0] = ap_info[i].bssid[0];
			msg_ap_infor[i].bssid[1] = ap_info[i].bssid[1];
			msg_ap_infor[i].bssid[2] = ap_info[i].bssid[2];
			msg_ap_infor[i].bssid[3] = ap_info[i].bssid[3];
			msg_ap_infor[i].bssid[4] = ap_info[i].bssid[4];
			msg_ap_infor[i].bssid[5] = ap_info[i].bssid[5];
			msg_ap_infor[i].bssid[6] = ap_info[i].bssid[6];
			msg_ap_infor[i].rssi = ap_info[i].rssi;
		}
	}
	else
	{
		msg_ap_count = 0;
	}
	esp_wifi_scan_stop();
	esp_wifi_stop();
	esp_wifi_deinit();
}

