#include "App_bootloader.h"
uint8_t app_boot_update_status=BOOT_NO_UPDATE;//默认不更新

void App_bootloader_check_update(void)//判断是否需要进行更新
{
    printf("bootloader start\r\n");
    printf("check update\r\n");
    uint8_t data[3];
    int_AT24C02_Read_Bytes(CHECK_UPATE_ADDR, data, 3);
    uint16_t check_key = (data[1] << 8) | data[2];
    if(check_key!=CHECK_KEY)
    {
        data[0] = BOOT_NO_UPDATE;
        data[1] = (uint8_t)(CHECK_KEY >> 8);
        data[2] = (uint8_t)(CHECK_KEY);
        int_AT24C02_Write_Bytes(CHECK_KEY_ADDR, data, 3);
        HAL_Delay(10);
    }
    else
    {
        app_boot_update_status = data[0];
    }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin==KEY4_Pin)
    {
        app_boot_update_status=BOOT_RESET;
    }
}
void App_bootloader_check_default(void)//判断是否需要进行恢复出厂设置
{
    HAL_Delay(3000);
}
static uint32_t Int_Get_Flash_Sector(uint32_t addr)
{
    if (addr < 0x08004000) return FLASH_SECTOR_0;
    else if (addr < 0x08008000) return FLASH_SECTOR_1;
    else if (addr < 0x0800C000) return FLASH_SECTOR_2;
    else if (addr < 0x08010000) return FLASH_SECTOR_3;
    else if (addr < 0x08020000) return FLASH_SECTOR_4;
    else if (addr < 0x08040000) return FLASH_SECTOR_5;
    else if (addr < 0x08060000) return FLASH_SECTOR_6;
    else return FLASH_SECTOR_7;
}
uint8_t meta_app_buff[9]={0};
uint32_t app_start_addr=0;//程序在W25Q64中 保存的位置
uint32_t app_size=0; //需要写入到flash的程序大小
static void App_bootloader_check_meta_data(void)
{
    //前4个字节程序的地址，后四个字节是程序的大小
    int_W25Q64_Read_Data(META_APP_ADDR_BLOCK, META_APP_ADDR_SECTOR, META_APP_ADDR_PAGE, META_APP_ADDR_ADDR, meta_app_buff, 8);
    app_start_addr=meta_app_buff[0] | meta_app_buff[1]<<8 | meta_app_buff[2]<<16 | meta_app_buff[3]<<24;
    app_size=meta_app_buff[4] | meta_app_buff[5]<<8 | meta_app_buff[6]<<16 | meta_app_buff[7]<<24;
    //假设程序存储的地址不能在第一扇中
    if(app_start_addr<0x001000)
    {
        printf("app_start_addr error\r\n");
        return;
    }
    if(app_size<APP_SIZE_MIN || app_size>APP_SIZE_MAX)
    {
        printf("app_size error\r\n");
        return;
    }
    //读取程序 判断头两个32位数据
    
    int_w25q64_Read_Data_With_32Addr(app_start_addr,meta_app_buff,8);
    uint32_t app_stack_ptr=meta_app_buff[0] | meta_app_buff[1]<<8 | meta_app_buff[2]<<16 | meta_app_buff[3]<<24;

    uint32_t app_reset_handler=meta_app_buff[4] | meta_app_buff[5]<<8 | meta_app_buff[6]<<16 | meta_app_buff[7]<<24;

    //校验栈顶地址
    if ((app_stack_ptr & 0xFF000000) != 0x20000000)
    {
        printf("stack_ptr error: 0x%X\r\n", app_stack_ptr);
        return;
    }
    // 校验复位中断
    if (app_reset_handler < APP_START_ADDER || app_reset_handler > APP_END_ADDER)
    {
        printf("reset_handler error: 0x%X\r\n", app_reset_handler);
        return;
    }
}
static void App_flash_erase(uint32_t addr,uint8_t erase_sector_num) 
{    
    HAL_FLASH_Unlock();
    uint32_t sector_num = 0; 
    sector_num = Int_Get_Flash_Sector(addr);  // 根据地址计算所在扇区号
        
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;  // 2.7V~3.6V
    EraseInitStruct.Sector = sector_num;
    EraseInitStruct.NbSectors = erase_sector_num;      //表示要连续擦除的扇区数量
    uint32_t SectorError = 0;
    HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
    HAL_FLASH_Lock();
}
static void App_bootloader_wirte_app_flash(void)
{
    //1.读取元数据的信息，描述后续程序
    //2.校验程序
    App_bootloader_check_meta_data();
    //3.写入程序
    //3.1先擦除足够的flash区域
    App_flash_erase(APP_START_ADDER,(app_size/FLASH_SECTOR_5_SIZE)+1);
    
    //3.2从W25Q64读出程序并写入Flash
    uint32_t flash_addr = APP_START_ADDER;      // Flash写入地址
    uint32_t w25q_addr = app_start_addr;        // W25Q64读取地址
    uint32_t remain_size = app_size;            // 剩余待写入字节数
    uint8_t  buffer[256];                       // 缓冲区，W25Q64一页=256字节
    

    //3.3写入flash
    HAL_FLASH_Unlock();
    while (remain_size > 0)
    {
        // 计算本次读取长度（最后一页可能不足256字节）
        uint32_t read_len = (remain_size > 256) ? 256 : remain_size;
        
        // 从W25Q64读取一页数据
        int_w25q64_Read_Data_With_32Addr(w25q_addr, buffer, read_len);
        
        // 写入Flash（Flash写入单位是32位，即4字节）
        for (uint32_t i = 0; i < read_len; i += 4)
        {
            // 将4字节组合成一个32位数据
            uint32_t data = buffer[i] | (buffer[i+1] << 8) | 
                           (buffer[i+2] << 16) | (buffer[i+3] << 24);
            
            // 写入Flash
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_addr + i, data);
        }
        
        // 更新指针和计数
        flash_addr += read_len;
        w25q_addr += read_len;
        remain_size -= read_len;
    }
    
    // 上锁Flash
    HAL_FLASH_Lock();
    //3.4写入完成后校验

}
void App_bootloader_update(void)    //更新应用程序
{
    if(app_boot_update_status==BOOT_UPDATE)
    {
		//擦除更新标志位
		int_AT24C02_Write_Byte(CHECK_UPATE_ADDR,BOOT_NO_UPDATE);
        //将W25Q64中的程序写入到flash中
        printf("update app\r\n");
        App_bootloader_wirte_app_flash();

        
    }
    else if(app_boot_update_status==BOOT_NO_UPDATE)
    {
        printf("no update\r\n");
    }
    else if(app_boot_update_status==BOOT_RESET)
    {
        printf("reset\r\n");
    }
}
void App_bootloader_jump_to_app(void)//跳转到应用程序
{
    if(app_boot_update_status==BOOT_RESET)
    {
        //跳转到出厂设置的默认程序 0x0800 8000
        int_bootloader_jump_to_app(RESET_START_ADDER);
    }
    else
    {
        //不需要恢复出厂设置，     0x0801 0000
        int_bootloader_jump_to_app(APP_START_ADDER);
    }
    
}
