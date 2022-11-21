/*
 * Common.h
 *
 *  Created on: 10 Jun 2022
 *      Author: nguyenphuonglinh
 */

#ifndef MAIN_COMMON_H_
#define MAIN_COMMON_H_

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
	BOTH,
}network;
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
}Network_Signal;
typedef struct
{
	long Timestamp;
	uint8_t Bat_Level;
	uint16_t Bat_Voltage;
	char Version[6];
}Device_Infor;
typedef struct _sensor_data
{
    uint8_t mac[6];
    uint8_t data[31];
    uint8_t data_len;
    uint32_t frame_cnt;
    uint8_t alarm;
    int8_t temp;
    uint16_t pres;
    uint16_t velo;
    uint8_t bat;
} sensor_data_t;
typedef struct _smartbox_data
{
    float lat;
	float lon;
	float speed;
	float acc;
	long epoch;
	uint8_t white_list_cnt;
	sensor_data_t ble_data[22];
} smartbox_data_t;
#endif /* MAIN_COMMON_H_ */
