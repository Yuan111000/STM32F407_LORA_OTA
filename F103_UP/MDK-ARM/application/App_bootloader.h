#ifndef __APP_BOOTLOADER_H
#define __APP_BOOTLOADER_H
#include "int_bootloader.h"

typedef enum
{
    BOOTLOADER_STATUS_INIT,
    BOOTLOADER_STATUS_RUN,
    BOOTLOADER_STATUS_REC_DATA,
    BOOTLOADER_STATUS_CHECK_DATA,
    BOOTLOADER_STATUS_REC_DONE,

}Bootloader_status;
void App_bootloader_init(void);//打印日志
void App_bootloader_run(void);//等待用户确认
void App_bootloader_rec_data(void);//接收数据
uint8_t App_bootloader_check_data(void);//校验数据
uint8_t App_bootloader_jump_app(void);//跳转到应用程序
void App_bootloader_work(void);//再main中调用
#endif
