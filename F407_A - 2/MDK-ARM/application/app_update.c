#include "app_update.h"
uint8_t uart_rec_buff[32]={0};
Update_State_t update_state=UPDATE_IDLE;
//CAN接受缓冲区
CAN_REC_MSG can_rec_msg[3]={0};
uint8_t can_rec_msg_count=0;

//声明一个容纳整个程序的静态缓存
uint8_t app_update_buff[APP_UPDATE_MAX_LEN]={0};
//app_update_buff中接受数据的长度
uint16_t can_rec_msg_len=0;
//记录当前一次接受的时间
uint32_t can_rec_time=0;
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)//串口接收完成中断处理函数
{ 
    //串口收到数据-》cmd
    if((huart->Instance==USART1) && (update_state==UPDATE_IDLE))
    {
        //校验数据 cmd 使用CAN   
        if(strstr((char *)uart_rec_buff,"cmd"))
        {
            update_state=UPDATE_RECV_SEND_CMD;
            //接受失败还能重新接受
            __HAL_UART_CLEAR_OREFLAG(&huart1);
            __HAL_UART_CLEAR_IDLEFLAG(&huart1);
            // 带有中断的串口接收
            HAL_UARTEx_ReceiveToIdle_IT(&huart1, uart_rec_buff, UART_SEC_BUFF_LEN);
        }
    }
}
static void App_run()
{
    //升级后的闪灯 2.0
    HAL_GPIO_WritePin(GPIOE, LED0_Pin, GPIO_PIN_RESET);
    //HAL_GPIO_WritePin(GPIOE, LED1_Pin, GPIO_PIN_RESET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(GPIOE, LED0_Pin, GPIO_PIN_SET);
    //HAL_GPIO_WritePin(GPIOE, LED1_Pin, GPIO_PIN_SET);
    HAL_Delay(200);
}
void App_Update_Init(void)//接收串口数据==》收到更新标记==》发送CAN 的更新指令
{
     __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    // 带有中断的串口接收
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, uart_rec_buff, UART_SEC_BUFF_LEN);
    Int_CAN_init();
}
void App_Update_Send_Update_Cmd(void)//使用CAN发送更新指令
{
    Int_CAN_send(CAN_UPDATE_CMD_ID, APP_UPDATE_CMD, APP_UPDATE_CMD_LEN);
    update_state=UPDATE_RECV_DATA;
}
void App_Update_Receive_App_Data(void)//CAN接受程序数据
{
    
    Int_CAN_receive_msg(can_rec_msg , &can_rec_msg_count);
    for(uint8_t i=0;i<can_rec_msg_count;i++)
    {
        can_rec_time=HAL_GetTick();
        //保存数据到app_update_buff中
        memcpy(app_update_buff+can_rec_msg_len, can_rec_msg[i].data, can_rec_msg[i].txHeader.DLC);
        //记录接收数据的长度
       can_rec_msg_len+=can_rec_msg[i].txHeader.DLC;
    }
    can_rec_msg_count=0;
    if(can_rec_time!=0 && can_rec_time+2000<HAL_GetTick())
    {
        //已经断开发送数据2s，则认为数据接收完成
        printf("recv data len:%d\r\n",can_rec_msg_len);
        update_state=UPDATE_RECV_CHECK_DATA;
    }
}
static uint32_t App_Crc_Cal(uint8_t *data,uint16_t len)
{
    uint32_t *p_data=(uint32_t *)data;
    uint32_t word_count=(len+3)/4;
    //复位CRC
    __HAL_CRC_DR_RESET(&hcrc);
    uint32_t crc_val=HAL_CRC_Calculate(&hcrc, p_data, word_count);
    return crc_val;

}
void App_Update_Check_Data(void)//检查app_update_buff中的数据
{
    Int_CAN_receive_msg(can_rec_msg , &can_rec_msg_count);
     
    for(uint8_t i=0;i<can_rec_msg_count;i++)
    {
        //读取发送来的CRC值，四字节，低位在前
        uint32_t crc_val_F1=can_rec_msg[i].data[0] | (can_rec_msg[i].data[1]<<8) | (can_rec_msg[i].data[2]<<16) | (can_rec_msg[i].data[3]<<24);
        uint32_t crc_val_F4=App_Crc_Cal(app_update_buff, can_rec_msg_len);
        if(crc_val_F1==crc_val_F4)
        {
            printf("crc check ok\r\n");
            update_state=UPDATE_RECV_BOOT_UPDATE;
        }
        else
        {
            printf("crc check failed\r\n");
            //清空状态
            memset(app_update_buff,0,APP_UPDATE_MAX_LEN);
            can_rec_msg_len=0;
            can_rec_time=0;
            update_state=UPDATE_IDLE;
        }
    }
    
}
uint8_t w25q64_write_buff[8]={0};
void App_Update_Change_Boot_Mode(void)//修改在AT24C02中的更新标志位，将程序保存到W25Q64中
{
    //将程序写入W25Q64
      //擦除
    uint16_t sector_erase_count=(can_rec_msg_len/4096)+2;
    for(uint8_t i=0;i<sector_erase_count;i++)
    {
        int_W25Q64_Erase_Sector(0,0+i);
    }
      //写入元数据
    w25q64_write_buff[0]=W25Q64_FLASH_APP_DATA_ADDR&0xff;
    w25q64_write_buff[1]=(W25Q64_FLASH_APP_DATA_ADDR>>8)&0xff;
    w25q64_write_buff[2]=(W25Q64_FLASH_APP_DATA_ADDR>>16)&0xff;
    w25q64_write_buff[3]=(W25Q64_FLASH_APP_DATA_ADDR>>24)&0xff;
    w25q64_write_buff[4]=can_rec_msg_len & 0xff;
    w25q64_write_buff[5]=(can_rec_msg_len>>8)&0xff;
    w25q64_write_buff[6]=(can_rec_msg_len>>16)&0xff;
    w25q64_write_buff[7]=(can_rec_msg_len>>24)&0xff;
    int_w25q64_Read_Data_With_32Addr(W25Q64_FLASH_APP_DATA_ADDR,w25q64_write_buff,8);
    uint16_t write_len = 0;
    uint16_t write_tmp_len= 0;
    //按照页写入到W25Q64中
    while(write_len<can_rec_msg_len)
    {
        if(can_rec_msg_len-write_len>256)
        {
            write_tmp_len=256;
        }
        else
        {
           write_tmp_len=can_rec_msg_len-write_len;
        }
        int_W25Q64_Write_Data_With_32Addr(W25Q64_FLASH_APP_DATA_ADDR+write_len,app_update_buff+write_len,write_tmp_len);
        write_len+=write_tmp_len;
    }
    //更新AT24C02更新标志位,密钥高8位在前
    uint8_t eeprom_buff[3]={0};
    eeprom_buff[0]=BOOT_UPDATE;
    eeprom_buff[1]=(CHECK_KEY>>8) & 0xff;
    eeprom_buff[2]= CHECK_KEY & 0xff;
    int_AT24C02_Write_Bytes(CHECK_UPATE_ADDR,eeprom_buff,3);

    //修改状态
    update_state=UPDATE_END;
}
void App_Update_Work(void)
{
    switch(update_state)
    {
        case UPDATE_IDLE: 
        //只有不需要更新程序的时候才闪灯
        App_run();
            break;

        case UPDATE_RECV_SEND_CMD: 
            printf("recv cmd\r\n");
            printf("send ycf to F103UP\r\n");
            App_Update_Send_Update_Cmd();
            //接收数据前清空缓冲区
            memset(app_update_buff, 0, APP_UPDATE_MAX_LEN);
            can_rec_msg_len=0;
            break;

        case UPDATE_RECV_DATA: 
            App_Update_Receive_App_Data();
            break;

        case UPDATE_RECV_CHECK_DATA:
            App_Update_Check_Data();
            break;

        case UPDATE_RECV_BOOT_UPDATE:
            App_Update_Change_Boot_Mode();
            break;
        case UPDATE_END:
            //延时之后重启
            HAL_Delay(1000);
            HAL_NVIC_SystemReset();
            break;
        default:
            break;    
    }
}
