#include "App_update.h"
extern llcc68_handle_t gs_handle;
APP_UPDATE_STATE_E App_Update_Statue = APP_UPDATE_WAIT_CMD;

extern uint16_t uart_rec_len_full;//程序总长度
uint16_t update_data_len=0;//已经发送的程序的长度
uint8_t update_buff[256]={0};//lora发送数据的缓存
void App_Update_Init(void)//初始化上位机更新程序
{
    printf("App_Update_Init\r\n");
    llcc68_lora_init();
    App_Update_Statue=APP_UPDATE_WAIT_CMD;
    printf("App_Update_Wait_Cmd\r\n");
    //如果已经烧录好更新的程序，需要手动填写一下程序的长度
   if(uart_rec_len_full == 0)
   {
       uart_rec_len_full=15804;
   }

}
void App_Update_Wait_Cmd(void)//等待下位机发送更新请求
{
    
    
    uint8_t res=llcc68_lora_receive();
    if(res==0)
    {
        if(strstr((char *)gs_handle.receive_buf,APP_UPDATE_CMD)!=NULL)
        {
            App_Update_Statue=APP_UPDATE_SEND_APP;
            printf("App_Update_Send_App\r\n");
            memset(gs_handle.receive_buf, 0, 256);
            gs_handle.receive_buf_len=0;
            //接受到更新请求后延时一会在发送程序
            HAL_Delay(100);
        }
    }
}

static uint32_t App_Crc_Cal(uint32_t flash_addr,uint16_t len)
{
    uint32_t *p_data=(uint32_t *)flash_addr;
    uint32_t word_count=(len+3)/4;
    //复位CRC
    __HAL_CRC_DR_RESET(&hcrc);
    uint32_t crc_val=HAL_CRC_Calculate(&hcrc, p_data, word_count);
    return crc_val;

}
void App_Update_Send_App(void)//发送更新程序给下位机
{
    if(update_data_len < uart_rec_len_full)
    {
        uint16_t send_len=0;
        if(uart_rec_len_full-update_data_len >= 100)
        {
            send_len=100;
        }
        else
        {
             send_len=uart_rec_len_full-update_data_len;
        }
        for(uint16_t i=0;i<send_len;i++)
        {
            update_buff[i]=*(volatile uint8_t *)(APP_START_ADDER + update_data_len + i);
        }
        update_data_len+=send_len;
       //发送lora，一次100字节
        llcc68_lora_send(update_buff,send_len);
        //添加延时，保证无线通讯的wending
        HAL_Delay(350);
    }
    else
    {
        //发送完毕
        printf("App_update_send_finish\r\n");
        update_data_len=0;
        App_Update_Statue=APP_UPDATE_WAIT_CMD;
        //CRC
        HAL_Delay(3100);
        uint32_t crc_val_F1=App_Crc_Cal(APP_START_ADDER,uart_rec_len_full);
        memset(update_buff,0,8);
        //将crc_val_F1 转换为 4字节的数组
        update_buff[0]=(crc_val_F1 & 0xFF);
        update_buff[1]=((crc_val_F1 >> 8) & 0xFF);
        update_buff[2]=((crc_val_F1 >> 16) & 0xFF);
        update_buff[3]=((crc_val_F1 >> 24) & 0xFF);
        llcc68_lora_send(update_buff,4);
    }
}
void App_Update_Work(void)//循环程序
{
    switch(App_Update_Statue)
    {
        case APP_UPDATE_WAIT_CMD:
            App_Update_Wait_Cmd();
            break;

        case APP_UPDATE_SEND_APP:
            App_Update_Send_App();
            break;
    }
}
