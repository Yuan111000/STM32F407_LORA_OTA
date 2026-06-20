#ifndef __INT_BOOTLOADER_H__
#define __INT_BOOTLOADER_H__
#include "usart.h"
#include "stdlib.h"
#include "string.h"

//程序写入的起始位置:A区的起始位置      B区32K：0x08000000——0x08007FFF  恢复出场区32K：0x08008000——0x0800FFFF  A区剩余所有：0x08010000——0x0801FFFF
#define RESET_START_ADDER 0x08008000
#define APP_START_ADDER   0x08010000 
#define APP_END_ADDER     0x0807FFFF



#define STACK_ADDER_s   0x20000000
#define STACK_ADDER     0x2002FFFF        //192K的SRAM


uint8_t int_bootloader_jump_to_app(uint32_t app_start_addr);


#endif
