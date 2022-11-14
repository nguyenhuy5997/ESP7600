/*
 * wifi_cell.h
 *
 *  Created on: 10 Jun 2022
 *      Author: nguyenphuonglinh
 */

#ifndef MAIN_WIFI_CELL_WIFI_CELL_H_
#define MAIN_WIFI_CELL_WIFI_CELL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include "esp_wifi.h"
#include "cJSON.h"
#include "esp_log.h"
#include "driver/timer.h"
#include "../json_user/json_user.h"
#include "../common.h"
void wifi_scan(apInfor msg_ap_infor[], uint8_t* msg_ap_count);

#endif /* MAIN_WIFI_CELL_WIFI_CELL_H_ */
