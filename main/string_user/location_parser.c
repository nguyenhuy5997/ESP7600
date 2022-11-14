/*
 * location_parser.c
 *
 *  Created on: 10 Jun 2022
 *      Author: nguyenphuonglinh
 */
#include "../main/string_user/location_parser.h"

int filter_comma(char *respond_data, int begin, int end, char *output)
{
	memset(output, 0, strlen(output));
	int count_filter = 0, lim = 0, start = 0, finish = 0,i;
	for (i = 0; i < strlen(respond_data); i++)
	{
		if ( respond_data[i] == ',')
		{
			count_filter ++;
			if (count_filter == begin)			start = i+1;
			if (count_filter == end)			finish = i;
		}

	}
	lim = finish - start;
	for (i = 0; i < lim; i++){
		output[i] = respond_data[start];
		start ++;
	}
	output[i] = 0;
	return 0;
}
bool CPSI_Decode(char* str, Network_Signal *Device_Singnal)
{
	if(strstr(str,"GSM"))
	{
		char temp_buf[50] = {0};
		char _LAC[10];
		char RX_buf[10];
		int i = 0, head = 0, tail = 0, k = 0, index = 0;
		for(i = 0; i < strlen(str); i++)
		{
			if(str[i] == ',') ++k;
			if(k == 1) head = i;
			if(k == 6) tail = i;
		}
		for(i = head+2; i < tail+1; i++)
		{
			if(str[i] == '-') str[i] = ',';
			temp_buf[index++] = str[i];
		}
		sscanf(temp_buf, "%d,%d,%[^,],%d,%[^,],,%d", &Device_Singnal->MCC, &Device_Singnal->MNC, _LAC, &Device_Singnal->cell_ID, RX_buf, &Device_Singnal->RSSI);
		Device_Singnal->RSSI = -1 * (Device_Singnal->RSSI);
		filter_comma(str,4,5,_LAC);
		Device_Singnal->LAC = (int)strtol(_LAC, NULL, 0);
		Device_Singnal->Network_type = GSM;
		if(Device_Singnal->MCC && Device_Singnal->MNC && Device_Singnal->cell_ID && Device_Singnal->RSSI && Device_Singnal->LAC)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(strstr(str,"LTE"))
	{
		char temp_buf[50] = {0};
		char _LAC[10];
		char RSRP_Buf[10];
		char RSRQ_Buf[10];
		int i = 0, head1 = 0, tail1 = 0, head2 = 0, tail2 = 0, k = 0, index = 0;
		for(i = 0; i < strlen(str); i++)
		{
			if(str[i] == ',') ++k;
			if(k == 1) head1 = i;
			if(k == 4) tail1 = i;
			if(k == 10) head2 = i;
			if(k == 11) tail2 = i;
		}
		for(i = head1 + 2; i < tail1 + 1; i++)
		{
			if(str[i] == '-') str[i] = ',';
			temp_buf[index++] = str[i];
		}
		for(i= head2 + 1; i < tail2 + 1; i++)
		{
			temp_buf[index++] = str[i];
		}
		sscanf(temp_buf, "%d,%d,%[^,],%d,%d", &Device_Singnal->MCC, &Device_Singnal->MNC, _LAC, &Device_Singnal->cell_ID, &Device_Singnal->RSSI);
		Device_Singnal->LAC = (int)strtol(_LAC, NULL, 0);
		filter_comma(str, 11, 12, RSRQ_Buf);
		Device_Singnal->RSRQ = atoi(RSRQ_Buf)/10;
		filter_comma(str, 12, 13, RSRP_Buf);
		Device_Singnal->RSRP = atoi(RSRP_Buf)/10;
		Device_Singnal->Network_type = LTE;
		if(Device_Singnal->MCC && Device_Singnal->MNC && Device_Singnal->cell_ID && Device_Singnal->RSSI && Device_Singnal->LAC && Device_Singnal->RSRQ && Device_Singnal->RSRP)
		{

			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

void getGPS(char *buffer, gps * _gps)
{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	char gpsbuffer[120];
	_gps->GPSfixmode = 0;
	strncpy(gpsbuffer, buffer, 120);
	// skip GPS run status
	char *tok = strtok(gpsbuffer, ",");
	if (! tok) return;
	// skip fix status
	tok = strtok(NULL, ",");
	if (! tok)
	{
		_gps->GPSfixmode = 0;
		return;
	}
	else
	{
		_gps->GPSfixmode = atoi(tok);
	}
	if(_gps->GPSfixmode == 1)
	{
		// only grab date and time if needed
		char *date = strtok(NULL, ",");
		if (! date) return;
		// Seconds
		char *ptr = date + 12;
		sec = atof(ptr);
		// Minutes
		ptr[0] = 0;
		ptr = date + 10;
		min = atoi(ptr);
		// Hours
		ptr[0] = 0;
		ptr = date + 8;
		hour = atoi(ptr);
		// Day
		ptr[0] = 0;
		ptr = date + 6;
		day = atoi(ptr);
		// Month
		ptr[0] = 0;
		ptr = date + 4;
		month = atoi(ptr);
		// Year
		ptr[0] = 0;
		ptr = date;
		year = atoi(ptr);
		// grab the latitude
		char *latp = strtok(NULL, ",");
		if (! latp) return;
		// grab longitude
		char *longp = strtok(NULL, ",");
		if (! longp) return;
		_gps->lat= atof(latp);
		_gps->lon = atof(longp);
		// grab altitude
		char *altp = strtok(NULL, ",");
		if (! altp) return;
		_gps->alt = atof(altp);
		// grab the speed in km/h
		char *speedp = strtok(NULL, ",");
		if (! speedp) return;
		_gps->speed = atof(speedp);
	}
}

void MQTT_Location_Payload_Convert(char* payload_str, gps hgps, Network_Signal Device_signal, Device_Infor Device_infor)
{
	memset(payload_str, 0, 500);
	char Network_Type_Str[10];
	if (Device_signal.Network_type == LTE)
	{
		sprintf(Network_Type_Str, "nb");
	}
	else if(Device_signal.Network_type == GSM)
	{
		sprintf(Network_Type_Str, "2g");
	}
	if(Device_signal.Network_type == GSM)
	{
		sprintf(payload_str, "{\"Type\":\"DPOS\",\"V\":\"%s\",\"ss\":%d,\"r\":%d,\"B\":%d,\"Cn\":\"%s\",\"Acc\":%f,\"lat\":%f,\"lon\":%f,\"T\":%ld}", \
				Device_infor.Version, Device_signal.RSSI, Device_signal.RSRQ, Device_infor.Bat_Level, Network_Type_Str, hgps.acc, hgps.lat, hgps.lon, hgps.epoch);
	}
	else
	{
		sprintf(payload_str, "{\"Type\":\"DPOS\",\"V\":\"%s\",\"ss\":%d,\"r\":%d,\"B\":%d,\"Cn\":\"%s\",\"Acc\":%f,\"lat\":%f,\"lon\":%f,\"T\":%ld}",\
				Device_infor.Version, Device_signal.RSRP, Device_signal.RSRQ, Device_infor.Bat_Level, Network_Type_Str, hgps.acc, hgps.lat, hgps.lon, hgps.epoch);
	}
}
void MQTT_WiFi_Payload_Convert(char* payload_str, char* wifi, Network_Signal Device_signal, Device_Infor Device_infor)
{
	memset(payload_str, 0, 500);
	char Network_Type_Str[10];
	if (Device_signal.Network_type == LTE)
	{
		sprintf(Network_Type_Str, "nb");
	}
	else if(Device_signal.Network_type == GSM)
	{
		sprintf(Network_Type_Str, "2g");
	}
	if(Device_signal.Network_type == GSM)
	{
		sprintf(payload_str,"{\"ss\":%d,\"Type\":\"DWFC\",\"B\":%d,\"r\":%d,\"C\":[{\"C\":%d,\"S\":%d,\"ID\":%d,\"L\":%d,\"N\":%d}],\"T\":%ld,\"V\":\"%s\",",\
				Device_signal.RSSI, Device_infor.Bat_Level, Device_signal.RSRQ, Device_signal.MCC, Device_signal.RSSI, Device_signal.cell_ID, Device_signal.LAC, \
				Device_signal.MNC, Device_infor.Timestamp , Device_infor.Version);
	}
	else
	{
		sprintf(payload_str,"{\"ss\":%d,\"Type\":\"DWFC\",\"B\":%d,\"r\":%d,\"C\":[{\"C\":%d,\"S\":%d,\"ID\":%d,\"L\":%d,\"N\":%d}],\"T\":%ld,\"V\":\"%s\",", \
				Device_signal.RSRP, Device_infor.Bat_Level, Device_signal.RSRQ,  Device_signal.MCC, Device_signal.RSSI,  Device_signal.cell_ID, Device_signal.LAC,\
				Device_signal.MNC, Device_infor.Timestamp, Device_infor.Version);
	}
	if(strlen(wifi) > 0)
	{
		sprintf(payload_str+strlen(payload_str),"%s",wifi);
	}
	if(Device_signal.Network_type == GSM)
	{
		if(strlen(wifi) > 0)
		{
			sprintf(payload_str+strlen(payload_str),"%s",",\"Cn\":\"2g\"}");
		}
		else
		{
			sprintf(payload_str+strlen(payload_str),"%s","\"W\":[],\"Cn\":\"2g\"}");
		}
	}
	else if(Device_signal.Network_type == LTE)
	{
		if(strlen(wifi) > 0)
		{
			sprintf(payload_str+strlen(payload_str),"%s",",\"Cn\":\"nb\"}");
		}
		else
		{
			sprintf(payload_str+strlen(payload_str),"%s","\"W\":[],\"Cn\":\"nb\"}");
		}
	}
}
