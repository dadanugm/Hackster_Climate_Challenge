/*
 * hackster_app.c file
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

#include "Fw_global_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include <eoss3_hal_uart.h>
#include "RtosTask.h"
#include <stdio.h>
#include <string.h>
#include "hackster_app.h"

uint16_t PM1 = 0;
uint16_t PM25 = 0;
uint16_t PM10 = 0;
uint16_t PM1_pred1 = 0;
uint16_t PM25_pred1 = 0;
uint16_t PM10_pred1 = 0;
uint16_t PM1_pred2 = 0;
uint16_t PM25_pred2 = 0;
uint16_t PM10_pred2 = 0;
uint16_t PM1_pred3 = 0;
uint16_t PM25_pred3 = 0;
uint16_t PM10_pred3 = 0;
uint32_t button_sw = 0;
uint8_t uart_rx_buff[128];


//task handle
xTaskHandle xPeriodicHandle;
xTaskHandle xUartRxHandle;
xTaskHandle xBt1Handle;
xTaskHandle xBt2Handle;
xTaskHandle xBt3Handle;
xTaskHandle xSensorTestHandle;
//

void turn_on_lcd(void)
{
	HAL_GPIO_Write(0,1);
}

void turn_off_lcd(void)
{
	HAL_GPIO_Write(0,0);
}

void turn_on_sensor(void)
{
	HAL_GPIO_Write(2,1);
}

void turn_off_sensor(void)
{
	HAL_GPIO_Write(2,0);
}

void lcd_write_status(void)
{
	char buff[128];
	int decode_stat = 0;
	turn_on_lcd();
	turn_on_sensor();
	for (int i=0;i<4;i++){
		vTaskDelay(2000/portTICK_PERIOD_MS);
		decode_stat = decode_sensor_data();
		if (decode_stat == 1){
			break;
		}
	}
	sprintf(buff,"show;PM1.0 : %i,PM2.5 : %i,PM10 : %i;\r\n",PM1,PM25,PM10); // send data to lcd controller
	uart_send_data(buff,strlen(buff));
	vTaskDelay(40000/portTICK_PERIOD_MS); // show for 40 second
	turn_off_sensor();
	turn_off_lcd();
}

void air_quality_prediction(void)
{
	char buff[128];
	int decode_stat = 0;

	turn_on_lcd();
	turn_on_sensor();
	for (int i=0;i<4;i++){
		vTaskDelay(2000/portTICK_PERIOD_MS);
		decode_stat = decode_sensor_data();
		if (decode_stat == 1){
			break;
		}
	}
		// calculate prediction and send data
	do_prediction();
		//
	sprintf(buff,"predict1;PM1.0 : %i,PM2.5 : %i,PM10 : %i;\r\n",PM1_pred1,PM25_pred1,PM10_pred1); // send data to lcd controller
	uart_send_data(buff,strlen(buff));
	vTaskDelay(2000/portTICK_PERIOD_MS); //
	sprintf(buff,"predict2;PM1.0 : %i,PM2.5 : %i,PM10 : %i;\r\n",PM1_pred2,PM25_pred2,PM10_pred2); // send data to lcd controller
	uart_send_data(buff,strlen(buff));
	vTaskDelay(2000/portTICK_PERIOD_MS); //
	sprintf(buff,"predict3;PM1.0 : %i,PM2.5 : %i,PM10 : %i;\r\n",PM1_pred3,PM25_pred3,PM10_pred3); // send data to lcd controller
	uart_send_data(buff,strlen(buff));
	vTaskDelay(50000/portTICK_PERIOD_MS); //

	turn_off_sensor();
	turn_off_lcd();
}

void send_sdcard_data(void)
{
	char buffer[32];
	turn_on_lcd();
	vTaskDelay(5000/portTICK_PERIOD_MS);
	sprintf(buffer,"send;\r\n");
	uart_send_data(buffer,strlen(buffer));
	vTaskDelay(80000/portTICK_PERIOD_MS); // 1 minute delay to send data in sd card
	turn_off_lcd();
}

void periodic_data(void)
{
	char buff[128];
	int decode_stat = 0;

	turn_on_sensor();
	vTaskDelay(10000/portTICK_PERIOD_MS); // wait 10 sec to sensor stabilize
	for (int i=0;i<4;i++){
		vTaskDelay(2000/portTICK_PERIOD_MS);
		decode_stat = decode_sensor_data();
		if (decode_stat == 1){
			break;
		}
	}
	sprintf(buff,"periodic;{PM1.0:%i,PM2.5:%i,PM10:%i};\r\n",PM1,PM25,PM10); // send data to lcd controller
	turn_on_lcd();
	vTaskDelay(2000/portTICK_PERIOD_MS); //
	uart_send_data(buff,strlen(buff));
	vTaskDelay(10000/portTICK_PERIOD_MS); //
	turn_off_lcd();
	turn_off_sensor(); // turnoff sensor
}

// button 1 for showing current data
void button_1_task (void *pParameter)
{
	uint8_t bt1_stat;
	while(1){
		// scan push button stat
		HAL_GPIO_Read(4, &bt1_stat);
		if (bt1_stat == 0){ // pressed
			vTaskDelay(1000/portTICK_PERIOD_MS); //delay for debounce
			lcd_write_status();
		}
	}
}

// button 2 for prediction
void button_2_task (void *pParameter)
{
	uint8_t bt2_stat;
	while(1){
		// scan push button stat
		HAL_GPIO_Read(3, &bt2_stat);
		if (bt2_stat == 0){ // pressed
			vTaskDelay(1000/portTICK_PERIOD_MS); //delay for debounce
			air_quality_prediction();
		}
	}
}

// button 3 to send data
void button_3_task (void *pParameter)
{
	uint8_t bt3_stat;
	while(1){
		// scan push button stat
		HAL_GPIO_Read(2, &bt3_stat);
		if (bt3_stat == 0){ // pressed
			vTaskDelay(1000/portTICK_PERIOD_MS); //delay for debounce
			send_sdcard_data();
		}
	}
}

void periodic_task (void *pParameter)
{
	while(1){
		vTaskDelay(5000/portTICK_PERIOD_MS);
		periodic_data();
		for(int i=0;i<15;i++){
			vTaskDelay(55000/portTICK_PERIOD_MS);
		}
	}
}

int decode_sensor_data(void)
{
	uint8_t buf_dbg[128];
	int stat;
	// clear value
	PM1 = 0;
	PM25 = 0;
	PM10 = 0;
	// we get the required data from buffer
	if (uart_rx_buff[0] == 0x42){ // check if the header is OK
		if (uart_rx_buff[1] == 0x4d){
			// we only need data on specific buffer slot
			// 2,3 -> frame; 4,5 -> PM1.0; 6,7 -> PM2.5; 8,9 -> PM10
			PM1 = uart_rx_buff[4];
			PM1 = (PM1<<8)|uart_rx_buff[5];
			PM25 = uart_rx_buff[6];
			PM25 = (PM25<<8)|uart_rx_buff[7];
			PM10 = uart_rx_buff[8];
			PM10 = (PM10<<8)|uart_rx_buff[9];
			stat = 1;
		}
		else{
			stat = 0;
		}
	}
	else {
		stat = 0;
	}

	//sprintf((char*)buf_dbg,"PM1:%.4x, PM2.5:%.4x, PM10:%.4x, Stat:%i\r\n",PM1,PM25,PM10,stat);
	//uart_send_data(buf_dbg,strlen(buf_dbg));

	return stat;
}


void uart_rx_task(void *pParameter)
{
	uint8_t urx;
	uint8_t rx_cnt = 0;
	while(1){
		urx = 0;
		urx = uart_rx(UART_ID_HW);
		// for test only
		//if (urx > 0){
		//	uart_tx(UART_ID_HW,urx);
		//}
		// for PMS5000 PM2.5 Sensor receive data
		uart_rx_buff[rx_cnt] = urx;
		rx_cnt ++;
		if (urx > 0){
			//rx_cnt ++;
			//uart_rx_buff[rx_cnt] = urx;
			if (urx == 0x42){
				// clean buffer
				for(int i=0;i<sizeof(uart_rx_buff);i++){
					uart_rx_buff[i] = 0;
				}
				rx_cnt = 0; // reset counter
				uart_rx_buff[rx_cnt] = urx; // put data to buffer
				rx_cnt ++;
			}
		}
	}
}


void uart_send_data(char* data, int len)
{
	for (int i=0;i<len;i++){
		uart_tx(UART_ID_HW,*data);
		data++;
	}
}

void test_uart_tx (void)
{
	char test_data1[] = "test uart\r\n";
	char test_data2[] = "sending data from uart HW";
	uart_send_data(test_data1,sizeof(test_data1));
	uart_send_data(test_data2,sizeof(test_data2));
}

void run_hackster_task(void)
{
	xTaskCreate (button_1_task, "button 1 task",512, NULL, (UBaseType_t)(PRIORITY_NORMAL), &xBt1Handle);
	configASSERT(xBt1Handle);
	xTaskCreate (button_2_task, "button 2 task",512, NULL, (UBaseType_t)(PRIORITY_NORMAL), &xBt2Handle);
	configASSERT(xBt2Handle);
	xTaskCreate (button_3_task, "button 3 task",512, NULL, (UBaseType_t)(PRIORITY_NORMAL), &xBt3Handle);
	configASSERT(xBt3Handle);
	xTaskCreate (periodic_task, "periodic Task",1024, NULL, (UBaseType_t)(PRIORITY_NORMAL), &xPeriodicHandle);
	configASSERT(xPeriodicHandle);
	xTaskCreate (uart_rx_task, "uart rx Task",1024, NULL, (UBaseType_t)(PRIORITY_NORMAL), &xUartRxHandle);
	configASSERT(xUartRxHandle);
	//xTaskCreate (sensor_test, "sensor test",1024, NULL, (UBaseType_t)(PRIORITY_NORMAL), &xSensorTestHandle);
	//configASSERT(xSensorTestHandle);
}

void sensor_test(void *pParameter)
{
	char buff[128];
	int decode_stat = 0;
	while(1){

		for (int i=0;i<4;i++){
			vTaskDelay(2000/portTICK_PERIOD_MS);
			decode_stat = decode_sensor_data();
			if (decode_stat == 1){
				break;
			}
		}
		sprintf(buff,"show;{PM1.0:%i,PM2.5:%i,PM10:%i}\r\n",PM1,PM25,PM10); // send data to lcd controller
		uart_send_data(buff,strlen(buff));
		vTaskDelay(1000/portTICK_PERIOD_MS); //
	}
}

void do_prediction(void)
{
	/*
	 *  Prediction/trend calculation is using linear regression
	 *  with formula Y = W0+W1.X
	 *  where:
	 *  Y -> Prediction result
	 *  X -> Time, where sampling 15 minute, 1 hour = 4x15
	 *  W0 -> Weight 0, starting value
	 *  W1 -> Weight 1, Gradient line
	 *  This value of W0 and W1 need to be calculated using Sensiml
	 *  in this case, I use instant value
	 */
	uint16_t Y = 0;
	uint16_t X = 0;
	uint16_t W0 = 0;
	uint16_t W1 = 1;

	// prediction for 1 hour
	// PM 1.0
	X = 1;
	W0 = PM1;
	Y = W0 + W1*X;
	PM1_pred1 = Y;
	//PM25
	W0 = PM25;
	Y = W0 + W1*X;
	PM25_pred1 = Y;
	//PM10
	W0 = PM10;
	Y = W0 + W1*X;
	PM10_pred1 = Y;

	// prediction for 2 hour
	// PM 1.0
	X = 2;
	W0 = PM1;
	Y = W0 + W1*X;
	PM1_pred2 = Y;
	//PM25
	W0 = PM25;
	Y = W0 + W1*X;
	PM25_pred2 = Y;
	//PM10
	W0 = PM10;
	Y = W0 + W1*X;
	PM10_pred2 = Y;

	// prediction for 3 hour
	// PM 1.0
	X = 3;
	W0 = PM1;
	Y = W0 + W1*X;
	PM1_pred3 = Y;
	//PM25
	W0 = PM25;
	Y = W0 + W1*X;
	PM25_pred3 = Y;
	//PM10
	W0 = PM10;
	Y = W0 + W1*X;
	PM10_pred3 = Y;
}
