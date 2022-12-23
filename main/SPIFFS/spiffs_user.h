/*
 * spiffs_user.h
 *
 *  Created on: Dec 13, 2022
 *      Author: adminlocal
 */

#ifndef MAIN_SPIFFS_SPIFFS_USER_H_
#define MAIN_SPIFFS_SPIFFS_USER_H_

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "../common.h"
#define STORAGE_NAMESPACE "storage"
#define MAX_CHAR 500

esp_err_t writetofile(char* FileName,char* textbuffer);
esp_err_t readfromfile(char* FileName,char* textbuffer);
esp_err_t saveMAC(char* fileName, smartbox_data_t smartbox);
void mountSPIFFS();




#endif /* MAIN_SPIFFS_SPIFFS_USER_H_ */
