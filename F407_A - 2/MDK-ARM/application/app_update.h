#ifndef __APP_UPDATE_H__
#define __APP_UPDATE_H__
#include "usart.h"
#include "int_can.h"
#include "crc.h"
#include "int_w25q64.h"
#include "int_at24c02.h"
#define UART_SEC_BUFF_LEN 32

#define CAN_UPDATE_CMD_ID 0
#define APP_UPDATE_CMD  "ycf"//与上位机相同
#define APP_UPDATE_CMD_LEN 3

#define APP_UPDATE_MAX_LEN 16384 //16K缓冲区，接受整个程序

//W25Q64存放元数据的地址
#define W25Q64_FLASH_META_ADDR 0x000000
//W25Q64存放程序数据的地址     块（2位），扇（1），页（1），字节（2位）
#define W25Q64_FLASH_APP_DATA_ADDR 0x001000

//AT24C02
#define CHECK_UPATE_ADDR 0x10  //更新状态的位置
#define BOOT_UPDATE      0x01  //更新标志
#define BOOT_NO_UPDATE   0x02  //不更新标志
#define CHECK_KEY_ADDR   0x11  //密钥的地址
#define CHECK_KEY        0x5A6B  //密钥值
typedef enum
{
    UPDATE_IDLE=0,
    UPDATE_RECV_SEND_CMD,
    UPDATE_RECV_DATA,
    UPDATE_RECV_CHECK_DATA,
    UPDATE_RECV_BOOT_UPDATE,
    UPDATE_END
}Update_State_t;
void App_Update_Init(void);//接收串口数据==》收到更新标记==》发送CAN 的更新指令
void App_Update_Send_Update_Cmd(void);//使用CAN发送更新指令
void App_Update_Receive_App_Data(void);//CAN接受程序数据，保存到W25Q64中
void App_Update_Change_Boot_Mode(void);//修改在AT24C02中的更新标志位，将程序保存到W25Q64中
void App_Update_Work(void);


#endif
