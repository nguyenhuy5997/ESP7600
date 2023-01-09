/*
 * OBD.c
 *
 *  Created on: Jan 9, 2023
 *      Author: nguyenphuonglinh
 */

#include "OBD.h"
#include "../simcom7600/simcom7600.h"
void uart_OBD(void *arg);
void _sendAT_OBD(char *AT_command);
AT_res ___readSerial_OBD(uint32_t timeout, char* expect);
bool _readSerial_OBD(uint32_t timeout);
obd_serial obd = {};
void init_OBD(uart_port_t uart_num, int tx_io_num, int rx_io_num, int baud_rate)
{
	obd.uart_num = uart_num;
	obd.tx_io_num = tx_io_num;
	obd.rx_io_num = rx_io_num;
	obd.baud_rate = baud_rate;
	uart_config_t uart_config =
	{
		.baud_rate = baud_rate,
		.data_bits = UART_DATA_8_BITS,
		.parity    = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	};
	int intr_alloc_flags = 0;
	ESP_ERROR_CHECK(uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
	ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(uart_num, tx_io_num, rx_io_num, ECHO_TEST_RTS, ECHO_TEST_CTS));
	xTaskCreate(uart_OBD, "uart_echo_task1", 4096, NULL, 10, NULL);
}
void uart_OBD(void *arg)
{
	uint8_t data[BUF_SIZE];
	while (1) {
		int len = uart_read_bytes(obd.uart_num, data, (BUF_SIZE - 1), 1000 / portTICK_PERIOD_MS);
		// Write data back to the UART
		if (len) {
			data[len] = '\0';
			ESP_LOGI(TAG_OBD, "Rec_OBD: %s", (char*) data);
			memcpy(obd.AT_buff, data, len);
			obd.AT_buff_avai = true;
		}
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}
void _sendAT_OBD(char *AT_command)
{
	ESP_LOGI(TAG_OBD, "Send: %s", AT_command);
	obd.AT_buff_avai = false;
	memset(obd.AT_buff, 0, BUF_SIZE);
	uart_write_bytes(obd.uart_num, (const char *) AT_command, strlen((char *)AT_command));
	uart_write_bytes(obd.uart_num, (const char *)"\r\n", strlen("\r\n"));
	vTaskDelay(100/portTICK_PERIOD_MS);
}
AT_res ___readSerial_OBD(uint32_t timeout, char* expect)
{
	uint64_t timeOld = esp_timer_get_time() / 1000;
	while (!(esp_timer_get_time() / 1000 > timeOld + timeout))
	{
		vTaskDelay(10/portTICK_PERIOD_MS);
		if(obd.AT_buff_avai)
		{
			if(strstr((char*)obd.AT_buff, "ERROR")) return AT_ERROR;
			else if(strstr((char*)obd.AT_buff, expect)) return AT_OK;
		}
	}
	return AT_TIMEOUT;
}
bool _readSerial_OBD(uint32_t timeout)
{
	uint64_t timeOld = esp_timer_get_time() / 1000;
	while (!obd.AT_buff_avai && !(esp_timer_get_time() / 1000 > timeOld + timeout))
	{
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
	if(obd.AT_buff_avai == false) return false;
	else return true;
}

bool OBD_getSpeed(float * speed)
{
	AT_res res;
	int retry = 3;
	while (retry--)
	{
		_sendAT_OBD("ATRON");
		res = ___readSerial_OBD(1500, "$OBD-RT");
		if (res == AT_OK)
		{
			char sub_str[300];
			int idx_start = 0, idx_end = 0;
			for(int i = 0; i < strlen((char*)obd.AT_buff); i++)
			{
				if(obd.AT_buff[i] == 'R' && obd.AT_buff[i+1] == 'T')
				{
					idx_start = i + 3;
				}
				if(obd.AT_buff[i] == 0x0D && obd.AT_buff[i+1] == 0x0A && idx_start != 0)
				{
					idx_end = i;
					break;
				}
			}
			strncpy(sub_str, (char*)obd.AT_buff + idx_start, idx_end - idx_start);
			sub_str[idx_end - idx_start] = 0;
			float a, b, c;
			sscanf(sub_str, "%f,%f,%f,", &a, &b,&c);
			*speed = c;
			return true;
		}
		else if (res == AT_ERROR) return false;
	}
	return false;
}
