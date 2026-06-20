#include "int_bootloader.h"

// 跳转到A程序
uint8_t int_bootloader_jump_to_app(uint32_t app_start_addr)
{
    typedef void (*pFunc)(void);
    // 栈顶地址的值
    uint32_t app_stack_ptr = *(volatile uint32_t *)(app_start_addr);
    uint32_t app_reset_handler = *(volatile uint32_t *)(app_start_addr + 4);

    // 校验栈顶地址 (必须在SRAM范围内)
    if ((app_stack_ptr & 0xFF000000) != 0x20000000)
    {
        printf("stack_ptr error: 0x%X\r\n", app_stack_ptr);
        return 1;
    }


    // 校验复位中断
    if (app_reset_handler < app_start_addr || app_reset_handler > APP_END_ADDER)
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
    SCB->VTOR = app_start_addr;
    
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

