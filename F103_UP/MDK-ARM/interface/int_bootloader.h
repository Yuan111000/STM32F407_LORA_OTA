#ifndef __INT_BOOTLOADER_H__
#define __INT_BOOTLOADER_H__
#include "usart.h"
#include "stdlib.h"
#include "string.h"
//串口接收缓冲区大小 512字节
#define BOOTLOADER_UART_REC_BUFF_LEN 512
//上位机的FLASH, 0x08080000---0x0800FFFF ,共64K,从0x08008000开始，还剩32K  
#define APP_START_ADDER 0x08008000 
#define APP_END_ADDER   0x08080000

#define STACK_ADDER_s   0x20000000
#define STACK_ADDER     0x20005000

void int_bootloader_receive_app(void);
uint8_t int_bootloader_jump_to_app(void);
void int_bootloader_erase_flash(uint32_t apge_addr, uint16_t pages);//外部可调用，提前擦除FLASH

#endif
