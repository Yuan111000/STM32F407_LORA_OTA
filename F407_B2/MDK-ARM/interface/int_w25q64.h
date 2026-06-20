#ifndef __INT_W25Q64_H__
#define __INT_W25Q64_H__
#include "spi.h"
//W25Q64的指令
#define W25Q64_READ_ID          0X9F
#define W25Q64_READ_STATUS_REG  0X05
#define W25Q64_READ_DATA        0X03
#define W25Q64_PAGE_PROGRAM     0X02
#define W25Q64_SECTOR_ERASE     0x20
#define W25Q64_WRITE_ENABLE     0x06
void int_W25Q64_Start(void);
void int_W25Q64_Stop(void);
void int_W25Q64_Write_Byte(uint8_t byte);
uint8_t int_W25Q64_Read_Byte(void);
void int_W25Q64_Read_ID(uint8_t* mf_id,uint16_t *device_id);

//void int_W25Q64_Read_Data(uint32_t addr,uint8_t*data,uint16_t len);
void int_W25Q64_Read_Data(uint8_t block,uint8_t sector,uint8_t page,uint8_t addr,uint8_t*data,uint16_t len);
void int_w25q64_Read_Data_With_32Addr(uint32_t addr,uint8_t*data,uint16_t len);
void int_W25Q64_Write_Data(uint8_t block,uint8_t sector,uint8_t page,uint8_t addr,uint8_t*data,uint16_t len);
void int_W25Q64_Erase_Sector(uint8_t block,uint8_t sector);//擦除一扇
#endif // __INT_W25Q64_H__

