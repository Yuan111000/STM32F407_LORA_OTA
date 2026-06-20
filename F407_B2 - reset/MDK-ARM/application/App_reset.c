#include "App_reset.h"
//记录上次接受的时间
extern uint32_t last_rec_time;
// 记录当前的状态
Bootloader_status bootloader_status = BOOTLOADER_STATUS_INIT;
uint8_t app_rec_start_buff[64] = {0};
uint16_t app_rec_start_len = 0; // 打印日志中接受到串口数据的真实长度
// 记录用户发送的程序的总长度
uint32_t app_rec_total_len = 0;
//接受实际数据的长度
extern uint16_t uart_rec_len_full;

void App_bootloader_init(void) // 打印日志
{
    bootloader_status = BOOTLOADER_STATUS_INIT;
    printf("reset start\r\n");
    printf("wait user send data\r\n");
    printf("send 'start:len' to start\r\n");
} 
void App_bootloader_run(void) // 等待用户确认
{
    HAL_UARTEx_ReceiveToIdle(&huart1, app_rec_start_buff, 64, &app_rec_start_len, 0xffffff);

    if (app_rec_start_len > 0)
    {
        char *start_str = strstr((char *)app_rec_start_buff, "start:");
        if (start_str != NULL)
        {
            app_rec_total_len = atoi(start_str + 6);
            if (app_rec_total_len > 0)
            {
                printf("app_rec_total_len:%d\r\n", app_rec_total_len);
                //修该状态
               bootloader_status = BOOTLOADER_STATUS_RUN;
            }
            else
            {
                printf("len error!\r\n");
            }
        }
        else
        {
            printf("data error!\r\n");
        }
    }
}
void App_bootloader_rec_data(void)   // 等待接收数据
{
    //接收完成后修改状态为校验数据
    //软件：等待2s
    if(last_rec_time!=0 &&HAL_GetTick()-last_rec_time>2000)
    {
        //2s没接受数据
        bootloader_status=BOOTLOADER_STATUS_CHECK_DATA;
    }
    

}
uint8_t App_bootloader_check_data(void) // 校验数据 0表示校验成功，1表示校验失败
{
    if(app_rec_total_len==uart_rec_len_full)
    {
        printf("app rec ok \r\n ");
        bootloader_status=BOOTLOADER_STATUS_JUMP_APP;
        return 0;
    }
    else
    {
        printf("app rec error or timeout\r\n");
    }
    return 1;
}
uint8_t App_bootloader_jump_app(void)   // 跳转到应用程序
{
    printf("jump to app\r\n");
    uint8_t rec=int_bootloader_jump_to_app();
    return rec;



    // printf("jump to app\r\n");
    // printf("checking flash at 0x%X...\r\n", APP_START_ADDER);
    
    // // 打印前16字节的向量表内容用于调试
    // uint32_t *vector_table = (uint32_t *)APP_START_ADDER;
    // for (int i = 0; i < 4; i++)
    // {
    //     printf("vector[%d] = 0x%08X\r\n", i, vector_table[i]);
    // }
    
    // uint8_t rec = int_bootloader_jump_to_app();
    // return rec;
}
void App_bootloader_work(void)
{
    switch (bootloader_status)
    {
    case BOOTLOADER_STATUS_INIT:
        App_bootloader_run();
        break;

    case BOOTLOADER_STATUS_RUN:
        //接收数据准备
        int_bootloader_erase_flash(APP_START_ADDER, 6);  // 擦除6个扇区
        printf("flash erase ok!\r\n");
        printf("ready to receive app data!\r\n");
        bootloader_status = BOOTLOADER_STATUS_REC_DATA;
        int_bootloader_receive_app();
        break;

    case BOOTLOADER_STATUS_REC_DATA:
        //等待接收完成
        App_bootloader_rec_data();
        break;

    case BOOTLOADER_STATUS_CHECK_DATA:
        //检擦数据情况
        
        if(App_bootloader_check_data()==1)
        {
            printf("app rec error, reset system\r\n");

            NVIC_SystemReset();
        }
        break;

    case BOOTLOADER_STATUS_JUMP_APP:
  
        if(App_bootloader_jump_app()==1)
        {
            printf("jump app error, reset system\r\n");
            NVIC_SystemReset();
        }
        break;

    default:
        break;
    }
}
