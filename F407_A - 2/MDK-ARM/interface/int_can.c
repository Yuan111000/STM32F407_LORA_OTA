#include "int_can.h"
void Int_CAN_init(void)//配置白名单过滤器
{
    //配置过滤器
    CAN_FilterTypeDef CAN_FilterConfig={0};
    CAN_FilterConfig.FilterBank = 0;//过滤器编号
    CAN_FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;//掩码模式
    CAN_FilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;//32位
    CAN_FilterConfig.FilterIdHigh = 0x0020;
    CAN_FilterConfig.FilterIdLow = 0x0000;
    CAN_FilterConfig.FilterMaskIdHigh = 0xffe0;
    CAN_FilterConfig.FilterMaskIdLow = 0x0000;
    CAN_FilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    CAN_FilterConfig.FilterActivation = ENABLE;
    HAL_CAN_ConfigFilter(&hcan1,&CAN_FilterConfig);
    
    HAL_CAN_Start(&hcan1);
}
void Int_CAN_send(uint16_t id,uint8_t *data,uint8_t len)//发送消息
{
    //等待发送邮箱的空闲
    while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1)==0);
    //将发送的消息添加到发送邮箱中
    CAN_TxHeaderTypeDef CAN_TxHeader={0};
    CAN_TxHeader.StdId = id;//标准帧ID
    CAN_TxHeader.RTR = CAN_RTR_DATA; 
    CAN_TxHeader.IDE = CAN_ID_STD;
    CAN_TxHeader.DLC = len;
    uint32_t mailbox=0;
    //句柄，头信息，数据，邮箱编号
    HAL_CAN_AddTxMessage(&hcan1, &CAN_TxHeader, data, &mailbox);
}
void Int_CAN_receive_msg(CAN_REC_MSG *rec_msg, uint8_t *msg_count)//接收消息
{
    *msg_count=HAL_CAN_GetRxFifoFillLevel(&hcan1,CAN_RX_FIFO0);
    for(uint8_t i=0; i<*msg_count; i++)
    {
        //指向当前使用的缓存
        CAN_REC_MSG *rec_msg_temp=&rec_msg[i];
        //读取接收到的消息
       memset(rec_msg_temp,0,sizeof(CAN_REC_MSG));
        
        HAL_CAN_GetRxMessage(&hcan1,CAN_RX_FIFO0,&(rec_msg_temp->txHeader),rec_msg_temp->data);
       
    }
}
