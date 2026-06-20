#include "int_bootloader.h"
// 解决性能问题：加内存/换协议

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
static void Int_flash_erase(void)
{
    // 判断当前写入的地址是否是新的一页，如果是需要擦除
    uint8_t is_erase = 0;
    uint32_t page_adder = 0;
    for (uint16_t i = 0; i < uart_rec_len; i++)
    {
        uint8_t data = *(volatile uint8_t *)(APP_START_ADDER + i + flash_write_offset);
        if (data != 0xff)
        {

            is_erase = 1;
            // 计算当前页的地址
            page_adder = (APP_START_ADDER + i + flash_write_offset) - (APP_START_ADDER + i + flash_write_offset) % FLASH_PAGE_SIZE;
            break;
        }
    }
    // 擦除当前页
    if (is_erase)
    {
        FLASH_EraseInitTypeDef EraseInitStruct;
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.Banks = FLASH_BANK_1;
        EraseInitStruct.PageAddress = page_adder;
        EraseInitStruct.NbPages = 1;
        uint32_t PageError = 0;
        // flash擦除比较耗费性能
        HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    }
}
static void Int_flash_write_with_last(void) // 有上一次字节
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
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart->Instance == USART1)
    {
        last_rec_time=HAL_GetTick(); //ms单位
        uart_rec_len = size;
        uart_rec_len_full += uart_rec_len;
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


//指定擦除的区域
void int_bootloader_erase_flash(uint32_t apge_addr, uint16_t pages)
{
    //解锁
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks = FLASH_BANK_1;
    EraseInitStruct.PageAddress = apge_addr;
    EraseInitStruct.NbPages = pages;
    uint32_t PageError = 0;
    // flash擦除比较耗费性能
    HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    //加锁
    HAL_FLASH_Lock();
}
