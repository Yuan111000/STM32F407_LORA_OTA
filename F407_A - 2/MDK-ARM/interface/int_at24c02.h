#ifndef __INT_AT24C02_H__
#define __INT_AT24C02_H__
#include "i2c.h"
#include "usart.h"
//device address
#define AT24C02_ADDR 0xA0     //写
#define AT24C02_ADDR_R 0xA1   //读


#define AT24C02_ADDR_SIZE 8  //地址长度
#define AT24C02_PAGE_SIZE 16  //AT24C02的页大小为16字节,共16页,总共256字节
uint8_t int_AT24C02_Read_Byte(uint8_t byte_addr);//读取一个字节
void int_AT24C02_Read_Bytes(uint8_t byte_addr, uint8_t* data, uint16_t len);//读取多个字节
void int_AT24C02_Write_Byte(uint8_t byte_addr, uint8_t data);//写入一个字节
void int_AT24C02_Write_Bytes(uint8_t byte_addr, uint8_t* data, uint16_t len);//写入多个字节



#endif// __INT_AT24C02_H__

