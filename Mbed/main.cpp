/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include "SPI_TFT_ILI9341.h"
#include "string.h"
#include "Arial12x12.h"
#include "Arial24x23.h"
#include "Arial28x28.h"
#include "font_big.h"
#include "sd_backup.h"

// the TFT is connected to SPI pin 5-7
SDBackup backup;
SPI_TFT_ILI9341 TFT(PA_7,PA_6,PA_5,PA_4,PB_0,PB_1,"TFT"); // mosi, miso, sclk, cs, reset, dc
DigitalOut Blk(PA_1);
UnbufferedSerial U1(PA_9,PA_10);

Thread show_data(osPriorityNormal,1024);
Thread send_data(osPriorityNormal,1024);
Thread predict_data(osPriorityNormal,1024);
Thread periodic_th(osPriorityNormal,1024);
Thread release_th(osPriorityNormal,1024);
Thread parse_uart(osPriorityNormal,1024);

void hardware_init(void);
void lcd_show_data(void);
void show_predict_data(void);
void send_backup_data(void);
void U1_int_callback(void);
void parse_uart_data(void);
void save_periodic_data(void);
void clear_uart_buff(void);
void backup_release(void);

bool lcd_flag = false;
bool predict_flag = false;
bool data_flag = false;
bool periodic_flag = false;
bool uart_parse_flag = false;
bool release_done = false;
bool backup_release_flag = false;

// payload
uint8_t int_data;
uint8_t uart_buff[256];
char header[10];
uint8_t data_payload[128];
uint16_t uart_buff_cnt = 0;
// payload data
char PM1_status[32];// = "PM1.0 : 15";
char PM25_status[32];// = "PM2.5 : 24";
char PM10_status[32];// = "PM10 : 75";
char pred1_PM1[32];// = "PM1.0 : 25";
char pred1_PM25[32];// = "PM2.5 : 35";
char pred1_PM10[32];//= "PM10 : 21";
char pred2_PM1[32];// = "PM1.0 : 45";
char pred2_PM25[32];// = "PM2.5 : 35";
char pred2_PM10[32];// = "PM10 : 75";
char pred3_PM1[32];// = "PM1.0 : 35";
char pred3_PM25[32];// = "PM2.5 : 23";
char pred3_PM10[32];// = "PM10 : 45";
char periodic_data[64];

int main()
{
    hardware_init();
    show_data.start(lcd_show_data);
    send_data.start(send_backup_data);
    predict_data.start(show_predict_data);
    periodic_th.start(save_periodic_data);
    release_th.start(backup_release);
    parse_uart.start(parse_uart_data);

    while (true) {
        thread_sleep_for(1000);
    }
}

void hardware_init(void)
{
    U1.baud(9600);
    U1.attach(&U1_int_callback,SerialBase::RxIrq);
    Blk = 1;
    //backup.sdcard_test();
    backup.sdcard_init();
    TFT.claim(stdout);      // send stdout to the TFT display
    TFT.set_orientation(3);
    TFT.background(Black);    // set background to black
    TFT.foreground(White);    // set chars to white
    TFT.cls();                // clear the screen
    thread_sleep_for(1000); // wait for mcu to receive data
    // show data to lcd
}

void lcd_show_data(void)
{
    while (1){
        if(lcd_flag == true){
            lcd_flag = false;
            U1.write("show sensor data\r\n",18);
            TFT.cls();
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(100,0);
            TFT.printf("Pollution");
            TFT.locate(0,40);
            TFT.printf("Measurement Status");
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(0,80);
            TFT.printf("%s",PM1_status);
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(0,120);
            TFT.printf("%s",PM25_status);
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(0,160);
            TFT.printf("%s",PM10_status);
            thread_sleep_for(60000);
            //clean buffer
            for (int i=0;i<32;i++){
                PM1_status[i] = 0;
                PM25_status[i] = 0;
                PM10_status[i] = 0;
            }

        }
        thread_sleep_for(100);
    }
}

void show_predict_data(void)
{
    while (1){
        if (predict_flag == true){
            predict_flag = false;
            TFT.cls();
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(80,0);
            TFT.printf("Prediction");
            TFT.locate(80,40);
            TFT.printf("In 1 Hour");
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(0,80);
            TFT.printf("%s",pred1_PM1);
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(0,120);
            TFT.printf("%s",pred1_PM25);
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(0,160);
            TFT.printf("%s",pred1_PM10);
            ThisThread::sleep_for(10000);
            TFT.cls();
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(80,0);
            TFT.printf("Prediction");
            TFT.locate(80,40);
            TFT.printf("In 2 Hour");
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(0,80);
            TFT.printf("%s",pred2_PM1);
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(0,120);
            TFT.printf("%s",pred2_PM25);
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(0,160);
            TFT.printf("%s",pred2_PM10);
            ThisThread::sleep_for(10000);
            TFT.cls();
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(80,0);
            TFT.printf("Prediction");
            TFT.locate(80,40);
            TFT.printf("In 3 Hour");
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(0,80);
            TFT.printf("%s",pred3_PM1);
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(0,120);
            TFT.printf("%s",pred3_PM25);
            TFT.set_font((unsigned char*) Arial28x28);
            TFT.locate(0,160);
            TFT.printf("%s",pred3_PM10);
            thread_sleep_for(10000);
            thread_sleep_for(60000);

            for (int i=0;i<32;i++){
                pred1_PM1[i] = 0;
                pred1_PM25[i] = 0;
                pred1_PM10[i] = 0;
                pred2_PM1[i] = 0;
                pred2_PM25[i] = 0;
                pred2_PM10[i] = 0;
                pred3_PM1[i] = 0;
                pred3_PM25[i] = 0;
                pred3_PM10[i] = 0;
            }
        }
    }
}

void send_backup_data(void)
{
    while (1){
        if (data_flag == true){
            data_flag = false;
            TFT.cls();
            TFT.set_font((unsigned char*) Neu42x35);
            TFT.locate(70,50);
            TFT.printf("Sending");
            ThisThread::sleep_for(1000);
            TFT.locate(100,100);
            TFT.printf("Data");
            ThisThread::sleep_for(1000);
            TFT.locate(80,140);
            TFT.printf("....");
            ThisThread::sleep_for(1000);
            TFT.locate(120,140);
            TFT.printf("....");
            ThisThread::sleep_for(1000);
            TFT.locate(160,140);
            TFT.printf("....");
            ThisThread::sleep_for(1000);
            TFT.locate(200,140);
            TFT.printf("....");
            thread_sleep_for(60000);
        }
        thread_sleep_for(100);
    }
}

void backup_release(void)
{
    int data_cnt = 0;
    char filename[32];
    char buff_sd[64];
    char buff_dbg[64];

    while(1){
        if (backup_release_flag == true){
            backup_release_flag = false;
            for (int i=0;i<sizeof(filename);i++){
                filename[i] = 0;
            }
            for (int i=0;i<sizeof(buff_sd);i++){
                buff_sd[i] = 0;
            }
            data_cnt = 0;
            data_cnt = backup.sdcard_countfile();
            sprintf(buff_dbg,"amount of file: %i\r\n",data_cnt);
            U1.write(buff_dbg,strlen(buff_dbg));
            if (data_cnt>0){
                for (int i=0;i<data_cnt;i++){
                    backup.sdcard_detectfile(filename);
                    backup.sdcard_read(buff_sd, filename);
                    U1.write(buff_sd,strlen(buff_sd));
                    backup.sdcard_erase(filename);
                }
            }
            U1.write("sending data done\r\n",19);
            data_flag = true;
        }
    }
}


void save_periodic_data(void)
{
    int data_cnt;
    char filename[32];
    char buff_dbg[64];

    while(1){
        if (periodic_flag == true){
            periodic_flag = false;
            data_cnt = 0;
            for (int i=0;i<sizeof(filename);i++){
                filename[i] = 0;
            }
            data_cnt = backup.sdcard_countfile();
            sprintf(buff_dbg,"amount of file: %i\r\n",data_cnt);
            U1.write(buff_dbg,strlen(buff_dbg));
            data_cnt ++;
            sprintf(filename,"/fs/backup/file_%i.txt",data_cnt);
            backup.sdcard_write(periodic_data,filename);
            U1.write("saving periodic data OK\r\n",25);
        }

    }

}


void parse_uart_data(void)
{
    int cnt1;
    int cnt2;
    while(1){
        if (uart_parse_flag == true){
            //U1.write("parse uart data\r\n",17);
            uart_parse_flag = false;
            cnt1 = 0;
            cnt2 = 0;
            // parse header and data
            for (int i=0;i<sizeof(header);i++){
                header[i] = 0;
            }
            while(uart_buff[cnt1] != ';'){
                header[cnt1] = uart_buff[cnt1];
                //U1.write(&header[cnt1],1);
                cnt1++;
            }
            //U1.write(header,strlen(header));
            //U1.write("\r\n",2);
            // we got the header, then load the data
            if (strcmp((char*)header,"show") == 0){
                U1.write("P1 Pressed\r\n",12);
                cnt1++;
                while(uart_buff[cnt1] != ','){
                    PM1_status[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }
                cnt1++;
                cnt2=0;
                while(uart_buff[cnt1] != ','){
                    PM25_status[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }
                cnt1++;
                cnt2=0;
                while(uart_buff[cnt1] != ';'){
                    PM10_status[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }
                U1.write(PM1_status,strlen(PM1_status));
                U1.write("\r\n",2);
                U1.write(PM25_status,strlen(PM25_status));
                U1.write("\r\n",2);
                U1.write(PM10_status,strlen(PM10_status));
                U1.write("\r\n",2);

                lcd_flag = true;
                predict_flag = false;
                data_flag = false;
                periodic_flag = false;
            }
            else if(strcmp((char*)header,"predict1") == 0){
                U1.write("P2 Pressed\r\n",12);
                U1.write("prediction 1\r\n",14);
                cnt1++;
                while(uart_buff[cnt1] != ','){
                    pred1_PM1[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }
                cnt1++;
                cnt2=0;
                while(uart_buff[cnt1] != ','){
                    pred1_PM25[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }
                cnt1++;
                cnt2=0;
                while(uart_buff[cnt1] != ';'){
                    pred1_PM10[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }

                U1.write(pred1_PM1,strlen(pred1_PM1));
                U1.write("\r\n",2);
                U1.write(pred1_PM25,strlen(pred1_PM25));
                U1.write("\r\n",2);
                U1.write(pred1_PM10,strlen(pred1_PM10));
                U1.write("\r\n",2);
            }

            else if(strcmp((char*)header,"predict2") == 0){
                U1.write("prediction 2\r\n",14);
                cnt1++;
                while(uart_buff[cnt1] != ','){
                    pred2_PM1[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }
                cnt1++;
                cnt2=0;
                while(uart_buff[cnt1] != ','){
                    pred2_PM25[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }
                cnt1++;
                cnt2=0;
                while(uart_buff[cnt1] != ';'){
                    pred2_PM10[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }

                U1.write(pred2_PM1,strlen(pred2_PM1));
                U1.write("\r\n",2);
                U1.write(pred2_PM25,strlen(pred2_PM25));
                U1.write("\r\n",2);
                U1.write(pred2_PM10,strlen(pred2_PM10));
                U1.write("\r\n",2);
            }

            else if(strcmp((char*)header,"predict3") == 0){
                U1.write("prediction 3\r\n",14);
                cnt1++;
                while(uart_buff[cnt1] != ','){
                    pred3_PM1[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }
                cnt1++;
                cnt2=0;
                while(uart_buff[cnt1] != ','){
                    pred3_PM25[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }
                cnt1++;
                cnt2=0;
                while(uart_buff[cnt1] != ';'){
                    pred3_PM10[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }

                U1.write(pred3_PM1,strlen(pred3_PM1));
                U1.write("\r\n",2);
                U1.write(pred3_PM25,strlen(pred3_PM25));
                U1.write("\r\n",2);
                U1.write(pred3_PM10,strlen(pred3_PM10));
                U1.write("\r\n",2);

                predict_flag = true;
                lcd_flag = false;
                data_flag = false;
                periodic_flag = false;
            }

            else if(strcmp((char*)header,"send") == 0){
                U1.write("P3 Pressed\r\n",12);
                backup_release_flag = true;
                predict_flag = false;
                lcd_flag = false;
                periodic_flag = false;
            }

            else if(strcmp((char*)header,"periodic") == 0){
                U1.write("Periodic Data\r\n",15);
                cnt1++;
                while(uart_buff[cnt1] != ';'){
                    periodic_data[cnt2] = uart_buff[cnt1];
                    cnt1++;
                    cnt2++;
                }
                // save data periodic to sdcard
                U1.write(periodic_data,strlen(periodic_data));
                U1.write("\r\n",2);

                periodic_flag = true;
                data_flag = false;
                predict_flag = false;
                lcd_flag = false;
            }

            else{
                U1.write("unknown pressed button\r\n",24);
            }

            clear_uart_buff();
        }
        thread_sleep_for(100);
    }
}

void U1_int_callback(void)
{
    U1.read(&int_data,1);
    U1.write(&int_data,1);
    uart_buff[uart_buff_cnt] = int_data;
    uart_buff_cnt ++;
    if (int_data == '\n'){
        //U1.write("end of file\r\n",13);
        uart_parse_flag = true;
        uart_buff_cnt = 0;
    }
}

void clear_uart_buff(void)
{
    for (int i=0;i<sizeof(uart_buff);i++){
        uart_buff[i] = 0;
    }
}



