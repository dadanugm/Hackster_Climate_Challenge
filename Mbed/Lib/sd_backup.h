/*
File    : sd_backup.h
Author  : Dadan (dadanugm07@gmail.com)
Date    : 10/11/2020
Version : 0.1
Revision: 

*/


#ifndef SD_BACKUP_H
#define SD_BACKUP_H

#include "mbed.h"
#include <cstdio>
#include <stdio.h>
#include <errno.h>
#include <functional>
#include <string.h>
#include "BlockDevice.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"

typedef enum 
{
    OK = 1,
    FAIL
} status;

class SDBackup {
public:
    void sdcard_init(void);
    void sdcard_write(char* data, const char* filename);
    void sdcard_read(char* data, const char* filename);
    void sdcard_erase(const char* file);
    void sdcard_format(void);
    int sdcard_countfile(void);
    void sdcard_detectfile(char* filename);
    void sdcard_test(void);
};

#endif