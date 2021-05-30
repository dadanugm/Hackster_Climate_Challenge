/*
File    : sd_backup.c
Author  : Dadan (dadanugm07@gmail.com)
Date    : 10/11/2020
Version : 0.1
Revision: 

*/

#include "sd_backup.h"
#include <cstdio>

SDBlockDevice sd(PB_15, PB_14, PB_13, PB_12);
FATFileSystem fs("fs");
const char FOLDER_NAME[] = "/fs/backup";
int filecnt;
int err;

/*
    Init to mount SD card for STM32
    @sd : SDblockdevice object
    
*/
void SDBackup::sdcard_init(void)
{
    // Try to mount the filesystem
    printf("Mounting the filesystem... ");
    fflush(stdout);
    err = fs.mount(&sd);
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        // Reformat if we can't mount the filesystem
        // this should only happen on the first boot
        printf("No filesystem found, formatting... ");
        fflush(stdout);
        err = fs.reformat(&sd);
        printf("%s\n", (err ? "Fail :(" : "OK"));
        if (err) {
            error("error: %s (%d)\n", strerror(-err), err);
        }
    }

    // check if directory backup is exist
    DIR* d = opendir(FOLDER_NAME);
    if (!d)
    {
        fflush(stdout);
        printf("directory not found\r\n");
        // create directory
        mkdir(FOLDER_NAME, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    } 
}

/*
    write file in SD card
    @filename : constant string filename
    @data : constant string that written to file
    
*/
void SDBackup::sdcard_write(char* data, const char* filename)
{
    // get filename
    fflush(stdout);
    FILE*   f = fopen(filename, "w+");
    fflush(stdout);
    err = fprintf(f,"%s",data);
    if (err < 0) {
        printf("Fail :(\n");
        error("error: %s (%d)\n", strerror(errno), -errno);
    }

    fflush(stdout);
    err = fclose(f);
    printf("%s\n", (err < 0 ? "Fail :(" : "Backup Data OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(errno), -errno);
    }
}

/*
    write file in SD card
    @filename : constant string filename
    @data : store to buffer
    
*/

void SDBackup::sdcard_read(char* data, const char* filename)
{
    int len;

    fflush(stdout);
    FILE*   f = fopen(filename, "r+");
    printf("%s\n", (!f ? "Fail :(" : "OK"));
    if (!f) {
        // Create the numbers file if it doesn't exist
        printf("No file found\r\n");
        fflush(stdout);
        f = fopen(filename, "w+");
        printf("%s\n", (!f ? "Fail :(" : "OK"));
        fflush(stdout);
        fprintf(f, "%d\n",0);
    } 
    else {
    fflush(stdout);
    printf("read data from file %s\r\n", filename);
    err = fscanf(f,"%s",data);
        if (err < 0) {
            printf("Fail :(\n");
            error("error: %s (%d)\n", strerror(errno), -errno);
            }
        }
    fflush(stdout);
    err = fclose(f);
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(errno), -errno);
    }
}

/*
    erase file in SD card
    @filename : constant string filename

*/
void SDBackup::sdcard_erase(const char* filename)
{
    fflush(stdout);
    err = remove(filename);
    printf("%s\n", (err ? "Fail :(" : "Erase succeed"));
    if (err) {
        printf("No file found\r\n");
    }
}

int SDBackup::sdcard_countfile(void)
{
    int amount = 0;
    fflush(stdout);
    DIR* d = opendir(FOLDER_NAME);
    while (true) {
        struct dirent*  e = readdir(d);
        if (!e) {
            break;
        }
        amount++;
        //printf("    %s\n", e->d_name);
    }

    printf("amount of file: %i \r\n",amount);
    return amount;
}

void SDBackup::sdcard_detectfile(char* filename)
{
    fflush(stdout);
    DIR* d = opendir(FOLDER_NAME);
    struct dirent* e = readdir(d);
    sprintf(filename,"%s/%s",FOLDER_NAME,e->d_name);
}

void SDBackup::sdcard_test(void)
{
    char sensors_data[1000];
    int file_cnt;
    char filename[32];
    char buff_sd[500];

    printf("--- Mbed OS filesystem example ---\n");
    sdcard_init();
// save file
    for (int i=0;i<10;i++)
    {
        printf("saving file %i to backup folder\r\n",i);
        sprintf(filename,"/fs/backup/file_%i.txt",i);
        sdcard_write(sensors_data,filename);
        thread_sleep_for(1000);
    }

    file_cnt = sdcard_countfile();
// backup  release example
    if (file_cnt>0)
    {
        for (int i=0;i<file_cnt;i++){
            sdcard_detectfile(filename);
            printf("file:%s\r\n",filename);
            sdcard_read(buff_sd, filename);
            printf("backup data:%s\r\n",buff_sd);
            sdcard_erase(filename);
        }

    }
}




