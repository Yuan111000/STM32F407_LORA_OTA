#ifndef __APP_BOOTLOADER_H
#define __APP_BOOTLOADER_H
#include "int_at24c02.h"
#include "usart.h"
#include "int_bootloader.h"
#include "int_w25q64.h"
//添加校验的密钥
#define CHECK_KEY_ADDR   0x11  //密钥的地址
#define CHECK_KEY        0x5A6B  //密钥值

#define CHECK_UPATE_ADDR 0x10  //更新状态的位置
#define BOOT_UPDATE      0x01  //更新标志
#define BOOT_NO_UPDATE   0x02  //不更新标志

#define BOOT_RESET 0x03         //恢复出厂设置

#define META_APP_ADDR_BLOCK     0x00  //元数据的地址
#define META_APP_ADDR_SECTOR    0x00  //元数据的地址
#define META_APP_ADDR_PAGE      0x00
#define META_APP_ADDR_ADDR      0x00
//程序存储的判断
#define APP_START_ADDR_MIN      0x001000  //程序存储的最小地址
#define APP_SIZE_MIN            500  
#define APP_SIZE_MAX            0x70000  
//STM32407VET6的FLASH划分   可以用于App_flash_erase（）的参数
// #define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) 	//扇区0起始地址, 16 Kbytes  B区起始
// #define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) 	//扇区1起始地址, 16 Kbytes  
// #define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) 	//扇区2起始地址, 16 Kbytes  恢复出厂设置区起始
// #define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) 	//扇区3起始地址, 16 Kbytes  
// #define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) 	//扇区4起始地址, 64 Kbytes  A区起始
// #define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) 	//扇区5起始地址, 128 Kbytes  
// #define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) 	//扇区6起始地址, 128 Kbytes  
// #define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) 	//扇区7起始地址, 128 Kbytes

#define FLASH_SECTOR_0_SIZE 16384  //B
#define FLASH_SECTOR_1_SIZE 16384
#define FLASH_SECTOR_2_SIZE 16384  //RESET
#define FLASH_SECTOR_3_SIZE 16384
#define FLASH_SECTOR_4_SIZE 65536  //A
#define FLASH_SECTOR_5_SIZE 131072
#define FLASH_SECTOR_6_SIZE 131072
#define FLASH_SECTOR_7_SIZE 131072
void App_bootloader_check_update(void);//判断是否需要进行更新
void App_bootloader_check_default(void);//判断是否需要进行恢复出厂设置
void App_bootloader_update(void);    //更新应用程序
void App_bootloader_jump_to_app(void);//跳转到应用程序






#endif
