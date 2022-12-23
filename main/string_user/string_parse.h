/*
 * string_parse.h
 *
 *  Created on: 27 Jun 2022
 *      Author: nguyenphuonglinh
 */

#ifndef MAIN_STRING_USER_STRING_PARSE_H_
#define MAIN_STRING_USER_STRING_PARSE_H_
#include <common.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>
#include "../json_user/json_user.h"
void getSubStrig(char *source, char *start, char *end, char *out);
void parse_ble_msg(uint8_t data_byte[31], sensor_data_t * sensor_data);
void conver_message_send(char* msg, smartbox_data_t smartbox_data);
void conver_message_send_tb(char* publishPayload, smartbox_data_t smartBox);
void JSON_analyze_MAC_update( char *  my_json_string, smartbox_data_t* smartbox);
#endif /* MAIN_STRING_USER_STRING_PARSE_H_ */
