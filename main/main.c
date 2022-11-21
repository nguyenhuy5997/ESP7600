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
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_gatt_common_api.h"
#include "cJSON.h"
#include "../main/simcom7600/simcom7600.h"
#include "../main/simcom7600/7600_config.h"
#include "../main/common.h"
#include "../main/string_user/location_parser.h"
#include "../main/json_user/json_user.h"
#include "../main/wifi_cell/wifi_cell.h"
#include "../main/OTA_LTE/FOTA_LTE.h"

#define GATTC_TAG "BT"
#define PROFILE_NUM 1
#define PROFILE_A_APP_ID 0

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

#define EMQX
#define TAG_MAIN "SIMCOM"

#ifdef	INNOWAY
	#define CLIENT_ID 			"MAN02ND09210"
	#define CLIENT_PW 			"z4r8A7CU6YEPrVuU115Q"
	#define MQTT_BROKER 		"tcp://vttmqtt.innoway.vn:1883"
#endif
#ifdef THINGSBOARD
	#define CLIENT_ID 			"eyJhbGciOiJIUzI1NiIsInR5cC"
	#define CLIENT_PW 			"eyJhbGciOiJIUzI1NiIsInR5cC"
	#define MQTT_BROKER 		"tcp://thingsboard.cloud:1883"
	#define PUB_TOPIC 			"v1/devices/me/telemetry"
#endif
#ifdef EMQX
	#define MQTT_BROKER 		"tcp://broker.emqx.io:1883"
	#define CLIENT_ID 			"47d1b050-6942-11ed-9022-0242ac120002"
	#define CLIENT_PW 			"47d1b050-6942-11ed-9022-0242ac120002"
	#define PUB_TOPIC 			"tpmsdata/47d1b050-6942-11ed-9022-0242ac120002"
	#define SUB_TOPIC 			"tpmsdata/47d1b050-6942-11ed-9022-0242ac120002"
#endif

#define VERSION "0.0.1"

char wifi_buffer[400];
client mqttClient7600 = {};
gps gps_7600;
Network_Signal network7600 = {};
Device_Infor deviceInfor = {};
LBS LBS_location;
uint8_t whitelist_addr[][6] = {{0xf9, 0xe0, 0x0c, 0xf5, 0x4a, 0xca},
								{0xea, 0x6e, 0x66, 0xe3, 0xa9, 0x5f},
								{0xd0, 0x59, 0x55, 0xe2, 0x01, 0xd0},
								{0xcb, 0xc6, 0x2f, 0xd5, 0xaf, 0x07}};

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
	//gpio_set_level(POWER_KEY, 1);
	gpio_set_level(nRST, 0);
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
	ESP_LOGI(TAG, "Subcribe mess: %s", _buff);
//	JSON_analyze_sub(_buff, &deviceInfor.Timestamp);
}
// Scan parameters
static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ONLY_WLST,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE};

// GATT data structure
struct gattc_profile_inst
{
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t char_handle;
    esp_bd_addr_t remote_bda;
};

// GATT data connected to GATT event handler
static struct gattc_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gattc_cb = gattc_profile_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,
    },
};

// GATT event handler
static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTC_REG_EVT:
        esp_ble_gap_set_scan_params(&ble_scan_params);
        for ( int i = 0; i < sizeof(whitelist_addr)/ sizeof(whitelist_addr[0]); i ++)
        {
        	ESP_LOGI(GATTC_TAG, "Add %d to white list", i);
        	esp_ble_gap_update_whitelist(true, whitelist_addr[i], BLE_WL_ADDR_TYPE_RANDOM);
        }
        uint16_t wl_length;
        esp_ble_gap_get_whitelist_size(&wl_length);
        ESP_LOGI(GATTC_TAG, "white list length: %d", wl_length);
        printf("1 - Register gatt event\n");
        break;
    default:
        break;
    }
}

// GAP callback function - search BT connections
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	uint8_t *adv_name = NULL;
	uint8_t adv_name_len = 0;
	switch (event)
	{
		case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
		{
			uint32_t duration = 0xFFFFFFFF;     // the unit of the duration is second
			esp_ble_gap_start_scanning(duration);
			printf("2 - Scan parameters set\n");
			break;
		}
		case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
			printf("3 - Start scan\n");
			break;
		case ESP_GAP_BLE_SCAN_RESULT_EVT:
		{
			esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
			switch (scan_result->scan_rst.search_evt)
			{
				case ESP_GAP_SEARCH_INQ_RES_EVT:
					adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
					ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
					esp_log_buffer_char(GATTC_TAG, adv_name, adv_name_len);
					if (scan_result->scan_rst.adv_data_len > 0) {
						ESP_LOGI(GATTC_TAG, "adv data:");
						esp_log_buffer_hex(GATTC_TAG, &scan_result->scan_rst.ble_adv[0], scan_result->scan_rst.adv_data_len);
					}
					break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
}

// GATTC callback function for gl_profile_tab structure initialization
// esp_gattc_cb_event_t - GATT Client callback function events
// esp_gatt_if_t - GATT interface type
// esp_ble_gattc_cb_param_t - GATT client callback parameters union
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
    do
    {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++)
        {
            if (gattc_if == ESP_GATT_IF_NONE || gattc_if == gl_profile_tab[idx].gattc_if)
            {
                if (gl_profile_tab[idx].gattc_cb)
                {
                    gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
}
void main_proc(void *arg)
{
	bool res;
	char pub_mqtt[500];
	while(1)
	{
		POWER_ON:
		ESP_LOGI(TAG, "----------> START PROGRAM <----------\r\n");
		gpio_set_level(nRST, 0);
		vTaskDelay(5000/portTICK_PERIOD_MS);
		if(isInit(20)) ESP_LOGW(TAG, "Module Init OK");
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
		res = networkType(BOTH, 3);
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

		res = mqttSubcribe(mqttClient7600, SUB_TOPIC, 1, 3, subcribe_callback);
		if(res) ESP_LOGW(TAG, "MQTT Sucribe OK");
		else
		{
			if(!isInit(3))
			{
				ESP_LOGE(TAG, "MQTT Sucribe FALSE");
				goto MQTT;
			}
		}
		res = openNetwork();
		if (res) ESP_LOGW(TAG, "Network Opened");
		else
		{
			if(!isInit(3))
			{
				ESP_LOGE(TAG, "Network Open FALSE");
				goto POWER_ON;
			}
		}
		while(1)
		{
			memset(&LBS_location, 0, sizeof(LBS_location));
			memset(&gps_7600, 0, sizeof(gps_7600));
			readGPS(&gps_7600);
			if(gps_7600.GPSfixmode == 2 || gps_7600.GPSfixmode == 3)
			{
				networkInfor(5, &network7600);
				cJSON *root = cJSON_CreateObject();
				cJSON_AddNumberToObject(root, "latitude", gps_7600.lat);
				cJSON_AddNumberToObject(root, "longitude", gps_7600.lon);
				cJSON_AddNumberToObject(root, "speed", gps_7600.speed);
				cJSON_AddStringToObject(root, "status", "On route");
//				MQTT_Location_Payload_Convert(pub_mqtt, gps_7600, network7600,  deviceInfor);
				res = mqttPublish(mqttClient7600, cJSON_Print(root), PUB_TOPIC, 1, 1);
				if(res)
				{
					ESP_LOGW(TAG, "Publish OK");
				}
				else
				{
					ESP_LOGE(TAG, "Publish FALSE");
					goto MQTT;
				}
			}
			else
			{
				networkInfor(5, &network7600);
				wifi_scan(wifi_buffer);
				MQTT_WiFi_Payload_Convert(pub_mqtt, wifi_buffer, network7600, deviceInfor);
				res = mqttPublish(mqttClient7600, pub_mqtt, PUB_TOPIC, 1, 1);
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


	esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT); // Release the controller memory
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	esp_bt_controller_init(&bt_cfg);
	esp_bt_controller_enable(ESP_BT_MODE_BLE);
	esp_bluedroid_init();                          // Initialize BT controller to allocate task
	esp_bluedroid_enable();                        // Enable bluetooth
	esp_ble_gap_register_callback(esp_gap_cb);     // Register the callback function to the GAP module
	esp_ble_gattc_register_callback(esp_gattc_cb); // Register the callback function to the GATTC module
	esp_ble_gattc_app_register(PROFILE_A_APP_ID);  // Register application callbacks with GATTC module

	sprintf(deviceInfor.Version, "%s", VERSION);

	deviceInfor.Bat_Level = 100;
//	init_gpio_output();
//	init_simcom(ECHO_UART_PORT_NUM_1, ECHO_TEST_TXD_1, ECHO_TEST_RXD_1, ECHO_UART_BAUD_RATE);
//	xTaskCreate(main_proc, "main", 4096, NULL, 10, NULL);

}
