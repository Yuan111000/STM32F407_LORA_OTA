#ifndef __INT_CAN_H
#define __INT_CAN_H
#include "can.h"
#include "string.h"
typedef struct
{
    CAN_RxHeaderTypeDef txHeader;
    uint8_t data[8];
    uint8_t len;
}CAN_REC_MSG;   
void Int_CAN_init(void);//配置白名单过滤器
void Int_CAN_send(uint16_t id,uint8_t *data,uint8_t len);//发送消息
void Int_CAN_receive_msg(CAN_REC_MSG *rec_msg,uint8_t *msg_count);//接收消息
#endif
