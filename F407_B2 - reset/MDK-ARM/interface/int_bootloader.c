#include "int_bootloader.h"
// 解决性能问题：加内存/换协议
static uint32_t Int_Get_Flash_Sector(uint32_t addr);
uint8_t uart_rec_buff[BOOTLOADER_UART_REC_BUFF_LEN] = {0};
uint16_t uart_rec_len = 0;
uint16_t uart_rec_len_full = 0;
// 写入程序的偏移量
uint32_t flash_write_offset = 0;
// 末尾可能出现的单个字节
uint8_t last_byte = 0;
uint8_t last_byte_flag = 0;
//记录当前一次接受数据的时间
uint32_t last_rec_time = 0;
static void Int_flash_erase(void) //用于回调函数zhong
{
    // 判断当前写入的地址是否是新的一页，如果是需要擦除
    uint8_t is_erase = 0;
    uint32_t sector_num = 0;
    for (uint16_t i = 0; i < uart_rec_len; i++)
    {
        uint8_t data = *(volatile uint8_t *)(APP_START_ADDER + i + flash_write_offset);
        if (data != 0xff)
        {
            is_erase = 1;
            // 计算当前页的地址
            //page_adder = (APP_START_ADDER + i + flash_write_offset) - (APP_START_ADDER + i + flash_write_offset) % FLASH_PAGE_SIZE;
            break;
        }
    }
    // 擦除当前页
    if (is_erase)
    {
        // 根据地址计算所在扇区号
        uint32_t addr = APP_START_ADDER + flash_write_offset;
        sector_num = Int_Get_Flash_Sector(addr);  // 需要新增此函数
        
        FLASH_EraseInitTypeDef EraseInitStruct;
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
        EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;  // 2.7V~3.6V
        EraseInitStruct.Sector = sector_num;
        EraseInitStruct.NbSectors = 1;
        uint32_t SectorError = 0;
        HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
    }
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

static void Int_flash_write_with_last(void) // 写入flash有上一次字节
{
    for (uint16_t i = 0; i < uart_rec_len; i += 2)
    {
        uint32_t flash_adder = APP_START_ADDER + i + flash_write_offset;
        uint16_t data16;
        if (i == 0)
        {
            data16 = last_byte | (uart_rec_buff[i] << 8);
        }
        else
        {
            data16 = uart_rec_buff[i - 1] | (uart_rec_buff[i] << 8);
        }
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash_adder, data16);
    }
}
static void Int_flash_write_no_last(void) // 没有上一次字节
{
    for (uint16_t i = 0; i < uart_rec_len; i += 2)
    {
        uint32_t flash_adder = APP_START_ADDER + i + flash_write_offset;
        uint16_t data16;
        if (i + 1 < uart_rec_len)
        {
            data16 = uart_rec_buff[i] | (uart_rec_buff[i + 1] << 8);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash_adder, data16);
        }
    }
}
static void Int_flash_write_halfworld(void)
{
    // 判断写入的的内容是否是偶数
    if ((uart_rec_len + last_byte_flag) % 2 == 0)
    {
        if (last_byte_flag == 1)
        {
            // 上次遗留有一个字节，这次作为第一个字节写入
            Int_flash_write_with_last();
            // 记录偏移量
            flash_write_offset += uart_rec_len + 1;
        }
        else // last_byte_flag==0
        {
            // 正好能写入，不再有上次遗留的字节
            // 写入数据,16位写入
            Int_flash_write_no_last();
            // 记录偏移量
            flash_write_offset += uart_rec_len;
        }
        last_byte_flag = 0;
    }
    else // 奇数字节
    {
        if (last_byte_flag == 1)
        {
            // 本身是奇数但有遗留的一个字节
            Int_flash_write_with_last();
            // 修改最后剩下的字节
            last_byte = uart_rec_buff[uart_rec_len - 1];
            // 记录偏移量
            flash_write_offset += uart_rec_len;
        }
        else
        {
            // 本身是奇数个也没有遗留的字节
            Int_flash_write_no_last();
            last_byte = uart_rec_buff[uart_rec_len - 1];

            // 记录偏移量
            flash_write_offset += uart_rec_len - 1;
        }
        last_byte_flag = 1;
    }
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)//回调函数
{
    if (huart->Instance == USART1)
    {
        last_rec_time=HAL_GetTick(); //ms单位
        uart_rec_len = size;
        uart_rec_len_full += uart_rec_len;
        // printf("uart_rec_len_full:%d\r\n", uart_rec_len_full);    影响性能

        // 将接受到的数据写入到flash中
        // 解锁FLASH
        HAL_FLASH_Unlock();

        // 擦除
        Int_flash_erase();
        
        // 写入
        Int_flash_write_halfworld();
       
        // 重新加锁
        HAL_FLASH_Lock();
        
        // 使用完数据后清空
        memset(uart_rec_buff, 0, BOOTLOADER_UART_REC_BUFF_LEN);
        __HAL_UART_CLEAR_OREFLAG(&huart1);
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);
        HAL_UARTEx_ReceiveToIdle_IT(&huart1, uart_rec_buff, BOOTLOADER_UART_REC_BUFF_LEN);
    }
}
void int_bootloader_receive_app(void)
{
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    // 带有中断的串口接收
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, uart_rec_buff, BOOTLOADER_UART_REC_BUFF_LEN);
}

// 跳转程序
uint8_t int_bootloader_jump_to_app(void)
{
    typedef void (*pFunc)(void);
    // 栈顶地址的值
    uint32_t app_stack_ptr = *(volatile uint32_t *)(APP_START_ADDER);
    uint32_t app_reset_handler = *(volatile uint32_t *)(APP_START_ADDER + 4);

    // 校验栈顶地址 (必须在SRAM范围内)
    if ((app_stack_ptr & 0xFF000000) != 0x20000000)
    {
        printf("stack_ptr error: 0x%X\r\n", app_stack_ptr);
        return 1;
    }


    // 校验复位中断
    if (app_reset_handler < APP_START_ADDER || app_reset_handler > APP_END_ADDER)
    {
        printf("reset_handler error: 0x%X\r\n", app_reset_handler);
        return 1;
    }
    

   // ===== 开始跳转 =====
    
    /* 1. 关闭所有中断 */
    __disable_irq();
    
    /* 2. 关闭SysTick */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    
    /* 3. 清除所有挂起的中断 */
    for (int i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }
    
    /* 4. 反初始化HAL */
    HAL_DeInit();
    
    /* 5. 设置主栈指针(MSP)为目标App的栈顶 */
    __set_MSP(app_stack_ptr);
    
    /* 6. 设置向量表偏移 */
    SCB->VTOR = APP_START_ADDER;
    
    /* 7. 数据同步屏障 */
    __DSB();
    __ISB();
    
    /* 8. 跳转到App (强制Thumb模式) */
    pFunc jump_to_app = (pFunc)(app_reset_handler & 0xFFFFFFFE | 0x01);
    jump_to_app();
    
    /* 如果执行到这里说明跳转失败 */
    while(1);  // 卡死在这里
    
    //return 0;
}

void int_bootloader_erase_flash(uint32_t apge_addr, uint16_t sectors)//起始的位置，擦除多少扇区
{
    //解锁
    HAL_FLASH_Unlock();
    uint32_t sector_num = Int_Get_Flash_Sector(apge_addr);

    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector = sector_num;
    EraseInitStruct.NbSectors = sectors;
    uint32_t SectorError = 0;
    // flash擦除比较耗费性能
    HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
    //加锁
    HAL_FLASH_Lock();
}
