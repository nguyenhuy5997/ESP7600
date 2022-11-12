/*
 * 7070_config.h
 *
 *  Created on: 8 Nov 2022
 *      Author: nguyenphuonglinh
 */

#ifndef MAIN_SIMCOM7070_7070_CONFIG_H_
#define MAIN_SIMCOM7070_7070_CONFIG_H_

#define ECHO_TEST_TXD_1 (17)
#define ECHO_TEST_RXD_1 (16)
#define ECHO_UART_PORT_NUM_1    (2)
#define ECHO_UART_BAUD_RATE     (115200)
#define POWER_KEY (2)
#define GPIO_OUTPUT_PIN_SEL (1ULL << POWER_KEY)

#define TAG "AT_CMD"

#endif /* MAIN_SIMCOM7070_7070_CONFIG_H_ */
