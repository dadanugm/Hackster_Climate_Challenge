/*
 * hackster_app.h file
 *
 * Author		: dadan
 * Hackster ID	: dadanugm
 * email		: dadanugm07@gmail.com
 * Linkedin 	: https://www.linkedin.com/in/surya-ramadhan-92467736/
 * Scholar		: https://scholar.google.com/citations?user=KaE3Qg8AAAAJ&hl=en&authuser=3
 *
 * Seeking for oportunity as embedded system engineer in Germany or New Zealand
 * or as PhD Student
 *
 */

#ifndef __HACKSTER_APP_H_
#define __HACKSTER_APP_H_

#include "Fw_global_config.h"
#include "dbg_uart.h"
#include "eoss3_hal_fpga_gpio.h"
#include "eoss3_hal_spi.h"
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_timer.h"
#include "s3x_clock_hal.h"

void turn_on_lcd(void);
void turn_off_lcd(void);
void turn_on_sensor(void);
void turn_off_sensor(void);
int decode_sensor_data(void);
void sensor_get_data(void);
void lcd_write_status(void);
void air_quality_prediction(void);
void periodic_data(void);
void send_sdcard_data(void);
void uart_send_data(char* data, int len);
void test_uart_tx (void);
void do_prediction(void);

/// routine task
void button_1_task (void *pParameter);
void button_2_task (void *pParameter);
void button_3_task (void *pParameter);
void periodic_task (void *pParameter);
void uart_rx_task(void *pParameter);
void sensor_test(void *pParameter);
void run_hackster_task(void);

#endif
//EOF
