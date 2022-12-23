/*
 * spiffs_user.c
 *
 *  Created on: Dec 13, 2022
 *      Author: adminlocal
 */
#include "spiffs_user.h"
#include "../json_user/json_user.h"
/* Get total message count value in NVS
   Return an error if anything goes wrong
   during this process.
 */
#define SPIFFSTAG  "spiffs"
esp_err_t writetofile( char* FileName,char* textbuffer)
{
	ESP_LOGI(SPIFFSTAG, "Opening file");
	char *base_path = "/spiffs";
	char file[64];
	sprintf(file, "%s/%s", base_path, FileName);
	FILE* f = NULL;
	f = fopen(file, "w");
	if (f == NULL) {
		ESP_LOGE(SPIFFSTAG, "Failed to open file for writing");
		return ESP_FAIL;
	}

	char* tp = textbuffer;
	while(*tp != '\0') {
		fputc(*tp++,f);
	}
	fclose(f);
	ESP_LOGI(SPIFFSTAG, "File written");
	return ESP_OK;
}

esp_err_t readfromfile(char* FileName,char* textbuffer)
{
	char *base_path = "/spiffs";
	char file[64];
	sprintf(file, "%s/%s", base_path, FileName);
    FILE* f = NULL;
    ESP_LOGI(SPIFFSTAG, "Reading file");
    f = fopen(file, "r");
    if (f == NULL) {
        ESP_LOGE(SPIFFSTAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[1024];
	fgets(line, sizeof(line), f);
	fclose(f);
	// strip newline
	char *pos = strchr(line, '\n');
	if (pos) {
		*pos = '\0';
	}
	strcpy(textbuffer, line);
	ESP_LOGI(SPIFFSTAG, "Read from file: '%s'", textbuffer);
	return ESP_OK;
}

void mountSPIFFS()
{
	ESP_LOGI(SPIFFSTAG, "Initializing SPIFFS");

	esp_vfs_spiffs_conf_t conf = {
	  .base_path = "/spiffs",
	  .partition_label = NULL,
	  .max_files = 5,
	  .format_if_mount_failed = true
	};

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(SPIFFSTAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(SPIFFSTAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(SPIFFSTAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
	}
}

esp_err_t saveMAC(char* fileName, smartbox_data_t smartbox)
{
	esp_err_t res;
	cJSON *root = cJSON_CreateObject();
	if (root == NULL)
	{
		ESP_LOGE("JSON", "Create JSON object root fail!\r\n");
		return ESP_FAIL;
	}
	cJSON *fields = cJSON_CreateArray();
	if (fields == NULL)
	{
		ESP_LOGE("JSON", "Create JSON object root fail!\r\n");
		return ESP_FAIL;
	}
	for(int i = 0; i < smartbox.white_list_cnt; i++)
	{
		char MAC_buf[25];
		sprintf(MAC_buf, "%x:%x:%x:%x:%x:%x", smartbox.ble_data[i].mac[0], smartbox.ble_data[i].mac[1], smartbox.ble_data[i].mac[2], \
											  smartbox.ble_data[i].mac[3], smartbox.ble_data[i].mac[4], smartbox.ble_data[i].mac[5]);
		cJSON_AddItemToArray(fields, cJSON_CreateString(MAC_buf));
	}
    cJSON_AddItemToObject(root, "MAC", fields);
	char* strJson = cJSON_PrintUnformatted(root);
	if (strJson == NULL) return ESP_FAIL;
	ESP_LOGI("JSON", "Save %s", strJson);
	res = writetofile(fileName, strJson);
	free(strJson);
	cJSON_Delete(root);
	if (res != ESP_OK) 	return ESP_FAIL;
	return ESP_OK;
}


