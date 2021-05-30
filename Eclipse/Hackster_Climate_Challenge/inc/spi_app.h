/*
 * spi_app.h file
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

#ifndef __SPI_APP_H_
#define __SPI_APP_H_

#include "Fw_global_config.h"
#include "dbg_uart.h"
#include "eoss3_hal_fpga_gpio.h"
#include "eoss3_hal_spi.h"
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_timer.h"
#include "s3x_clock_hal.h"


void spi_init(void);
void spi_write_data(uint8_t* data);
void spi_write_info(void);
void spi_read_data(uint8_t* data);
void spi_err_handler(void);

// tester
void test_spi_write(uint8_t* data, int len);


#endif
//EOF


