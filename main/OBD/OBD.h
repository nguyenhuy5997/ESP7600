/*
 * OBD.h
 *
 *  Created on: Jan 9, 2023
 *      Author: nguyenphuonglinh
 */

#ifndef MAIN_OBD_OBD_H_
#define MAIN_OBD_OBD_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_event.h"
#include "OBD.h"

#define ECHO_TEST_TXD_2 (10)
#define ECHO_TEST_RXD_2 (9)
#define ECHO_UART_PORT_NUM_2    (2)
#define ECHO_UART_BAUD_RATE_OBD     (9600)

#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)
#define ECHO_TASK_STACK_SIZE    (2048)
#define BUF_SIZE (2048)

#define TAG_OBD "AT_OBD"
typedef struct obd_t
{
	uart_port_t uart_num;
	int tx_io_num;
	int rx_io_num;
	int baud_rate;
	bool AT_buff_avai;
	uint8_t AT_buff[BUF_SIZE];
}obd_serial;

void init_OBD(uart_port_t uart_num, int tx_io_num, int rx_io_num, int baud_rate);
bool OBD_getSpeed(float * speed);
#endif /* MAIN_OBD_OBD_H_ */
