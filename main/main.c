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
#include "esp_sleep.h"
#include "cJSON.h"
#include "simcom7070/simcom7070.h"
#include "simcom7070/7070_config.h"
#include "common.h"
#include "string_user/location_parser.h"
#include "json_user/json_user.h"
#include "wifi_cell/wifi_cell.h"
#include "OTA_LTE/FOTA_LTE.h"
#include "Button/Button.h"
#include "gpioRoute.h"

#define TAG_MAIN "SIMCOM"


#define CLIENT_ID "MAN02ND09210"
#define CLIENT_PW "z4r8A7CU6YEPrVuU115Q"
#define MQTT_BROKER "tcp://vttmqtt.innoway.vn:1883"


#define VERSION "0.0.1"

char wifi_buffer[400];
client mqttClient7600 = {};
Network_Signal network7600 = {};
Device_Infor deviceInfor = {};

void getDevTs(long *time_stamp)
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
	io_conf.pin_bit_mask = ESP32_GPIO_OUTPUT_PIN_SEL;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
	gpio_set_level(PowerLatch, 1);
	gpio_set_level(PowerKey, 0);
	gpio_set_level(VCC_7070_EN, 1);
	gpio_set_level(VCC_GPS_EN, 0);
	gpio_set_level(UART_SW, 0);
}
void button (void * arg)
{
  button_event_t ev;
  QueueHandle_t button_events = button_init(PIN_BIT(BUTTON));
  int short_count = 0;
  uint32_t start = 0, press_duration = 0, check_process_time = 0;
  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) short_count = 1;
  while (true) {
      if (xQueueReceive(button_events, &ev, 1000/portTICK_PERIOD_MS)) {
        if ((ev.pin == BUTTON) && (ev.event == BUTTON_DOWN)) {
          ESP_LOGI(TAG, "BUTTON_DOWN");
          start = esp_timer_get_time() / 1000;
        } else if ((ev.pin == BUTTON) && (ev.event == BUTTON_UP)) {
        	ESP_LOGI(TAG, "BUTTON_UP");
        	press_duration = esp_timer_get_time() / 1000 - start;
        	if (press_duration < 1000) {
        		short_count++;
        		ESP_LOGI(TAG, "short press %d time", short_count);
        		check_process_time = esp_timer_get_time() / 1000;
        	}
        	else {
        		ESP_LOGI(TAG, "long press %d second", press_duration);
        	}
        }
     }
     if ((esp_timer_get_time() / 1000 > check_process_time + 1000) && short_count) {
    	 ESP_LOGI(TAG, "Total short press: %d", short_count);
    	 if (short_count == 5) {
    		 esp_sleep_enable_ext1_wakeup((1ULL << BUTTON), ESP_EXT1_WAKEUP_ANY_HIGH);
			 ESP_LOGW(TAG, "Enter to deep sleep mode, wake by BUTTON\r\n");
			 esp_deep_sleep_start();
    	 }
    	 short_count = 0;
     }
   }
}


void subcribe_callback(char * data)
{
	char* _buff;
	_buff = strstr(data, "{");
	sscanf(_buff, "%s", _buff);
	JSON_analyze_sub(_buff, &deviceInfor.Timestamp);
}
bool getGnssLoc(gps *gps, uint8_t scanNum)
{
	if (!setFunction(0)) return false;
	gpio_set_level(VCC_GPS_EN, 1);
	if(!powerOnGnss()) return false;
	while (--scanNum)
	{
		ESP_LOGW(TAG, "Scanned time: %d", scanNum);
		if(!readGPS(gps)) return false;
		if(gps->GPSfixmode) return true;
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
	return false;
}
static void main_proc(void *arg)
{
	Device_Infor vTagInfor;
	Bat devBat;
	bool res;
	gps gps_7600;
	msgInfor vTagMsgInfor;

	while(1)
	{
		while (!powerOn((gpio_num_t)PowerKey));
		res = getBatteryInfor(&devBat);
		if (!res) goto FAIL;
		vTagInfor.Bat_Level = devBat.level;
		getGnssLoc(&gps_7600, 60);
		if (gps_7600.GPSfixmode)
		{
			vTagMsgInfor.GPSfixmode = gps_7600.GPSfixmode;
			vTagMsgInfor.lat = gps_7600.lat;
			vTagMsgInfor.lon = gps_7600.lon;
		}
		else
		{
			wifi_scan(vTagMsgInfor.ap_infor, &vTagMsgInfor.ap_count);
		}
		FAIL:
		ESP_LOGE(TAG, "Error when excecuting AT Command");
	}
}

void app_main(void)
{
	printf("sieof: %d", sizeof(msgInfor));
	init_gpio_output();
	init_simcom(ECHO_UART_PORT_NUM_1, ECHO_TEST_TXD_1, ECHO_TEST_RXD_1, ECHO_UART_BAUD_RATE);
	xTaskCreate(button, "button", 4096, NULL, 3, NULL);
	xTaskCreate(main_proc, "main", 4096, NULL, 10, NULL);

}
