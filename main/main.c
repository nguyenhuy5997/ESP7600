#include <common.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "cJSON.h"
#include "../main/simcom7600/simcom7600.h"
#include "../main/simcom7600/7600_config.h"
#include "../main/common.h"
#include "../main/string_user/location_parser.h"
#include "../main/json_user/json_user.h"
#include "../main/wifi_cell/wifi_cell.h"
#include "../main/OTA_LTE/FOTA_LTE.h"

#define TAG_MAIN "SIMCOM"

#define CLIENT_ID "MAN02ND00074"
#define MQTT_BROKER "tcp://vttmqtt.innoway.vn:1883"
#define CLIENT_PW "NULL"
#define VERSION "0.0.1"

char wifi_buffer[400];
client mqttClient7600 = {};
gps gps_7600;
Network_Signal network7600 = {};
Device_Infor deviceInfor = {};
void GetDeviceTimestamp(long *time_stamp)
{
	struct timeval time_now;
	gettimeofday(&time_now, 0);
	*time_stamp = time_now.tv_sec;
//	ESP_LOGW(TAG,"Device timestamp: %ld\r\n", time_stamp);
}
void init_gpio_output()
{
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
	gpio_set_level(POWER_KEY, 1);
}
void initMqttClient(client* client, char* id, int sv_type, char* user_password, char* broker_mqtt)
{
	client->index = 0;
	memcpy(client->id, id, strlen(id));
	client->sv_type = sv_type;
	memcpy(client->password, user_password, strlen(user_password));
	memcpy(client->broker, broker_mqtt, strlen(broker_mqtt));
}

void subcribe_callback(char * data)
{
	char* _buff;
	_buff = strstr(data, "{");
	sscanf(_buff, "%s", _buff);
	JSON_analyze_sub(_buff, &deviceInfor.Timestamp);
}
static void main_proc(void *arg)
{
	bool res;
	char pub_mqtt[500];
	while(1)
	{
		POWER_ON:
		printf("----------> START PROGRAM <----------\r\n");

		res = powerOn(POWER_KEY);
		if (res)
		{
			ESP_LOGW(TAG, "Module power on OK");
		}
		else
		{
			ESP_LOGE(TAG, "Module power on FALSE");
			powerOff_(POWER_KEY);
			goto POWER_ON;
		}

		res = isInit(5);
		if(res) ESP_LOGW(TAG, "Module Init OK");
		else ESP_LOGE(TAG, "Module Init FALSE");

		res = echoATSwtich(0, 3);
		if(res) ESP_LOGW(TAG, "Turn off echo OK");
		else ESP_LOGE(TAG, "Turn off echo FALSE");

		res = switchGPS(1, 5);
		if(res) ESP_LOGW(TAG, "Turn on GPS OK");
		else
		{
			ESP_LOGE(TAG, "Turn on GPS FALSE");
			goto POWER_ON;
		}
MQTT:
		res = networkType(GSM, 3);
		if(res) ESP_LOGW(TAG, "Select network OK");
		else
		{
			ESP_LOGE(TAG, "Select network FALSE");
			if(!isInit(3)) goto POWER_ON;
		}

		res = isRegistered(10);
		if(res) ESP_LOGW(TAG, "Module registed OK");
		else
		{
			if(!isInit(3)) goto POWER_ON;
			ESP_LOGE(TAG, "Module registed FALSE");
		}

		mqttDisconnect(mqttClient7600, 3);

		initMqttClient(&mqttClient7600, CLIENT_ID, 0, CLIENT_PW, MQTT_BROKER);

		res = mqttConnect(mqttClient7600, 5);
		if(res) ESP_LOGW(TAG, "MQTT Connected");
		else
		{
			if(!isInit(3)) goto MQTT;
			ESP_LOGE(TAG, "MQTT can not connect");
		}

		res = mqttSubcribe(mqttClient7600, "messages/MAN02ND00073/control", 1, 3, subcribe_callback);
		if(res) ESP_LOGW(TAG, "MQTT Sucribe OK");
		else
		{
			if(!isInit(3)) goto MQTT;
			ESP_LOGE(TAG, "MQTT Sucribe FALSE");
		}
//		update_handler();
		while(1)
		{
			memset(&gps_7600, 0, sizeof(gps_7600));
			readGPS(&gps_7600);
			if(gps_7600.GPSfixmode == 2 || gps_7600.GPSfixmode == 3)
			{
				networkInfor(5, &network7600);
				MQTT_Location_Payload_Convert(pub_mqtt, gps_7600, network7600,  deviceInfor);
				res = mqttPublish(mqttClient7600, pub_mqtt, "messages/MAN02ND00073/gps", 1, 1);
			}
			else
			{
				networkInfor(5, &network7600);
				wifi_scan(wifi_buffer);
				MQTT_WiFi_Payload_Convert(pub_mqtt, wifi_buffer, network7600, deviceInfor);
				res = mqttPublish(mqttClient7600, pub_mqtt, "messages/MAN02ND00073/wificell", 1, 1);
				memset(wifi_buffer, 0, sizeof(wifi_buffer));
			}
			if(res)
			{
				ESP_LOGW(TAG, "Publish OK");
			}
			else
			{
				ESP_LOGE(TAG, "Publish FALSE");
				goto MQTT;
			}
			vTaskDelay(1000/portTICK_PERIOD_MS);
		 }
	}
}

void app_main(void)
{
	esp_log_level_set("wifi", ESP_LOG_NONE);
	esp_log_level_set("wifi_init", ESP_LOG_NONE);
	nvs_flash_init();
	esp_netif_init();
	esp_event_loop_create_default();
	esp_netif_create_default_wifi_sta();
	sprintf(deviceInfor.Version, "%s", VERSION);
	deviceInfor.Bat_Level = 100;
	init_gpio_output();
	init_simcom(ECHO_UART_PORT_NUM_1, ECHO_TEST_TXD_1, ECHO_TEST_RXD_1, ECHO_UART_BAUD_RATE);
	xTaskCreate(main_proc, "main", 4096, NULL, 10, NULL);

}
