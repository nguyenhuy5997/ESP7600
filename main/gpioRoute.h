/*
 * gpioRoute.h
 *
 *  Created on: Nov 13, 2022
 *      Author: admin
 */

#ifndef MAIN_GPIOROUTE_H_
#define MAIN_GPIOROUTE_H_

#define PowerLatch     	26   // Cap nguon toan mach
#define PowerKey     	32	 // PWR 7070
#define VCC_7070_EN		19	 // Cap nguon 7070
#define VCC_GPS_EN		18   // Cap nguon anten GPS
#define UART_SW			23
#define LED_1			5  	 // led xanh
#define LED_2			15	 // led do
#define DTR_Sim7070_3V3	27

#define BUTTON  		25
#define ACC_INT			2
#define RI_Sim7070_3V3	34
#define CHARGE			35

#define ESP32_GPIO_OUTPUT_PIN_SEL 	((1ULL<<PowerLatch) | (1ULL<<PowerKey) | (1ULL<<VCC_7070_EN) | (1ULL<<VCC_GPS_EN) | (1ULL<<UART_SW) | (1ULL<<LED_1) | (1ULL<<LED_2) | (1ULL<<DTR_Sim7070_3V3))
#define ESP32_GPIO_INPUT_PIN_SEL 	((1ULL << BUTTON) | (1ULL << RI_Sim7070_3V3) | (1ULL << CHARGE))

#endif /* MAIN_GPIOROUTE_H_ */
