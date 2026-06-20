#ifndef __INT_BOOTLOADER_H__
#define __INT_BOOTLOADER_H__
#include "usart.h"
#include "stdlib.h"
#include "string.h"
//串口接收缓冲区大小 512字节
#define BOOTLOADER_UART_REC_BUFF_LEN 512
//程序写入的起始位置:A区的起始位置      B区32K：0x08000000——0x08007FFF  出场设置区32K：0x08008000——0x0800FFFF  A区剩余所有：0x08010000——0x0807FFFF
#define APP_START_ADDER 0x08010000 
#define APP_END_ADDER   0x08080000

#define STACK_ADDER_s   0x20000000
#define STACK_ADDER     0x2002FFFF        //192K的SRAM

void int_bootloader_receive_app(void);
uint8_t int_bootloader_jump_to_app(void);
void int_bootloader_erase_flash(uint32_t sector_addr, uint16_t sectors);//外部可调用，提前擦除FLASH

#endif
