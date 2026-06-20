基于 LoRa(LLCC68) 实现 STM32F407 的 OTA 远程程序升级
项目来源：学习B站尚硅谷STM32 OTA-BootLoader零基础教程
视频地址：(https://www.bilibili.com/video/BV11C65BDEJx/?spm_id_from=333.337.search-card.all.click)
原版硬件：STM32F103C8T6 + STM32F103ZET6
本项目修改适配：STM32F103C8T6 + STM32F407VET6
主要改动点：
1. STM32F407时钟树、Flash扇区分区重新配置
2. F4芯片底层Flash擦写驱动重写
3.双芯片通信参数适配F4外设时序
4. BootLoader地址映射、固件偏移量适配F4存储空间
仅用于个人学习、求职作品集展示，不进行任何商业售卖与盈利行为

