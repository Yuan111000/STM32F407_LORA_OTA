#ifndef __APP_UPDATE_H__
#define __APP_UPDATE_H__
#include "int_can.h"
#include "usart.h"
#include "int_bootloader.h"
#include "crc.h"
#include "int_llcc68.h"
#define APP_UPDATE_CMD "ycf"
#define APP_UPDATE_CMD_1 'y'  //更新请求
#define APP_UPDATE_CMD_2 'c'
#define APP_UPDATE_CMD_3 'f'

#define APP_UPDATE_CMD_ID 1
typedef enum
{
    APP_UPDATE_WAIT_CMD=0,
    APP_UPDATE_SEND_APP,
}APP_UPDATE_STATE_E;
void App_Update_Init(void);//初始化上位机更新程序
void App_Update_Wait_Cmd(void);//等待下位机发送更新请求
void App_Update_Send_App(void);//发送更新程序给下位机
void App_Update_Work(void);//循环程序
#endif
