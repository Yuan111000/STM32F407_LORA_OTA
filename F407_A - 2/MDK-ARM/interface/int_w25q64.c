#include "int_w25q64.h"

void int_W25Q64_Start(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
}
void int_W25Q64_Stop(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}
void int_W25Q64_Write_Byte(uint8_t byte)//写一个字节
{
    HAL_SPI_Transmit(&hspi1, &byte, 1, 100);
}
uint8_t int_W25Q64_Read_Byte(void)//读一个字节
{
    uint8_t receive_data;
    HAL_SPI_Receive(&hspi1, &receive_data, 1, 100);
    return receive_data;
}
void int_W25Q64_Read_ID(uint8_t* mf_id,uint16_t *device_id)
{
    int_W25Q64_Start();
   
    int_W25Q64_Write_Byte(W25Q64_READ_ID);//发送读取ID命令
    *mf_id = int_W25Q64_Read_Byte();
    uint8_t device_id_high = int_W25Q64_Read_Byte();
    uint8_t device_id_low = int_W25Q64_Read_Byte();
    *device_id = (device_id_high << 8) | device_id_low;
    int_W25Q64_Stop();
}

static void int_W25Q64_Wait_Busy(void)//等待芯片空闲
{
    int_W25Q64_Start();
    
    while (1)
    {
        int_W25Q64_Write_Byte(W25Q64_READ_STATUS_REG);
        uint8_t status = int_W25Q64_Read_Byte();
        if ((status & 0x01) == 0) // 检查忙标志,0为空闲
        {
            break; // 忙标志清除，退出循环
        }
    }
    int_W25Q64_Stop();
}

//一次擦除4KB，一次写入256字节
void int_W25Q64_Read_Data(uint8_t block,uint8_t sector,uint8_t page,uint8_t addr,uint8_t*data,uint16_t len)
{
    int_W25Q64_Wait_Busy();
    int_W25Q64_Start();
    int_W25Q64_Write_Byte(W25Q64_READ_DATA); // 发送读取数据命令
    uint32_t addr_24=(block << 16) | (sector << 12) | (page << 8) | addr; // 计算地址
    int_W25Q64_Write_Byte(addr_24 >>16);
    int_W25Q64_Write_Byte(addr_24 >> 8);
    int_W25Q64_Write_Byte(addr_24);
    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = int_W25Q64_Read_Byte(); // 读取数据
    }
    int_W25Q64_Stop();
}

void int_w25q64_Read_Data_With_32Addr(uint32_t addr,uint8_t*data,uint16_t len)
{
    int_W25Q64_Wait_Busy();
    int_W25Q64_Start();
    int_W25Q64_Write_Byte(W25Q64_READ_DATA); // 发送读取数据命令
    
    int_W25Q64_Write_Byte((addr >>16)&0xff);
    int_W25Q64_Write_Byte((addr >> 8)&0xff);
    int_W25Q64_Write_Byte(addr&0xff);
    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = int_W25Q64_Read_Byte(); // 读取数据
    }
    int_W25Q64_Stop();
}
static void int_W25Q64_Write_Enable(void)//写使能
{
    int_W25Q64_Wait_Busy();
    int_W25Q64_Start();
    int_W25Q64_Write_Byte(W25Q64_WRITE_ENABLE); // 写使能命令
    int_W25Q64_Stop();
}

void int_W25Q64_Write_Data(uint8_t block,uint8_t sector,uint8_t page,uint8_t addr,uint8_t*data,uint16_t len)
{
    int_W25Q64_Write_Enable();
   
    int_W25Q64_Start();
    uint32_t addr_24=(block << 16) | (sector << 12) | (page << 8) | addr; // 计算地址
    int_W25Q64_Write_Byte(W25Q64_PAGE_PROGRAM); 
    int_W25Q64_Write_Byte(addr_24 >>16);
    int_W25Q64_Write_Byte(addr_24 >> 8);
    int_W25Q64_Write_Byte(addr_24);
    for(uint16_t i = 0; i < len; i ++)
    {
        int_W25Q64_Write_Byte(data[i]);
    }
    int_W25Q64_Stop();
    int_W25Q64_Wait_Busy();
}

void int_W25Q64_Write_Data_With_32Addr(uint32_t addr,uint8_t*data,uint16_t len)
{
    int_W25Q64_Write_Enable();
   
    int_W25Q64_Start();
   
    int_W25Q64_Write_Byte(W25Q64_PAGE_PROGRAM); 
    int_W25Q64_Write_Byte((addr >>16) & 0xff);
    int_W25Q64_Write_Byte((addr >> 8) & 0xff);
    int_W25Q64_Write_Byte(addr& 0xff);
    for(uint16_t i = 0; i < len; i ++)
    {
        int_W25Q64_Write_Byte(data[i]);
    }
    int_W25Q64_Stop();
    int_W25Q64_Wait_Busy();
}
void int_W25Q64_Erase_Sector(uint8_t block,uint8_t sector)//擦除一扇
{
    int_W25Q64_Write_Enable();
    int_W25Q64_Start();

    uint32_t addr=(uint32_t)(block *65535) + (uint32_t)(sector *4096); // 计算地址
    int_W25Q64_Write_Byte(W25Q64_SECTOR_ERASE);
    int_W25Q64_Write_Byte((addr >>16)&0xFF);
    int_W25Q64_Write_Byte((addr >> 8)&0xFF);
    int_W25Q64_Write_Byte(addr & 0xFF);

    int_W25Q64_Stop();

}
