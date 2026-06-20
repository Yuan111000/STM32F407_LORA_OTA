#include "int_at24c02.h"
uint8_t int_AT24C02_Read_Byte(uint8_t byte_addr)//读取一个字节
{
    uint8_t data;
    HAL_I2C_Mem_Read(&hi2c1, AT24C02_ADDR_R, byte_addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    return data;
}
void int_AT24C02_Read_Bytes(uint8_t byte_addr, uint8_t* data, uint16_t len)//读取多个字节
{
    HAL_I2C_Mem_Read(&hi2c1, AT24C02_ADDR_R, byte_addr, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}
void int_AT24C02_Write_Byte(uint8_t byte_addr, uint8_t data)//写入一个字节
{
    HAL_I2C_Mem_Write(&hi2c1, AT24C02_ADDR, byte_addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}
void int_AT24C02_Write_Bytes(uint8_t byte_addr, uint8_t* data, uint16_t len)//写入多个字节,一次只能写入一页，16个字节
{
    //地址值不能超过255
    if(byte_addr+len>255)
    {
        printf("byte_addr out!\r\n");
        return;
    }

    uint8_t page_remain_len = 16 - (byte_addr % 16);
    //已经写入的页数
    uint8_t page_count=0;
    if(len <= page_remain_len)
    {
        HAL_I2C_Mem_Write(&hi2c1, AT24C02_ADDR, byte_addr, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
    }
    else
    {
        uint8_t start_page_addr = byte_addr;
        while(len>page_remain_len)
        {
            HAL_I2C_Mem_Write(&hi2c1, AT24C02_ADDR, start_page_addr, I2C_MEMADD_SIZE_8BIT, data, page_remain_len, 1000);
            page_count++;
            //下一页的地址
            start_page_addr=byte_addr+page_remain_len;
            len-=page_remain_len;
            page_remain_len=16;
            HAL_Delay(10);
        }
        //写入最后一页
        if(len!=0)
        {
            HAL_I2C_Mem_Write(&hi2c1, AT24C02_ADDR, start_page_addr, I2C_MEMADD_SIZE_8BIT, data+page_count*16, len, 1000);
        }
    }
    
}
