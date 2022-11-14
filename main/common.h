/*
 * Common.h
 *
 *  Created on: 10 Jun 2022
 *      Author: nguyenphuonglinh
 */

#ifndef MAIN_COMMON_H_
#define MAIN_COMMON_H_
#include "esp_wifi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <math.h>

typedef struct gps_
{
	float lat;
	float lon;
	int GPSfixmode;
	float speed;
	float alt;
	int gps_num;
	int bei_num;
	int glo_num;
	float acc;
	long epoch;
	int vsat;
}gps;
typedef enum
{
	GSM,
	LTE,
	NB,
	BOTH,
} network;
typedef struct
{
	network Network_type;
	int MCC;
	int MNC;
	int LAC;
	int cell_ID;
	int16_t RSRP;
	int16_t RSRQ;
	int RSSI;
} Network_Signal;
typedef struct
{
	long Timestamp;
	uint8_t Bat_Level;
	uint16_t Bat_Voltage;
	char Version[6];
} Device_Infor;
typedef struct
{
	uint8_t bssid[6];
	int8_t  rssi;
} apInfor;
typedef struct
{
	int GPSfixmode;
	float lat;
	float lon;
	network net;
	Network_Signal netSignal;
	uint8_t Bat_Level;
	apInfor ap_infor[10];
	uint8_t ap_count;
} msgInfor;

#endif /* MAIN_COMMON_H_ */
