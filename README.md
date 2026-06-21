# STM32F407 LoRa OTA 远程升级 (基于LLCC68)

## 📖 项目来源
本项目是基于 B站 [尚硅谷STM32 OTA-BootLoader零基础教程] 进行的二次开发与硬件适配。
*   **原版教程视频**：[点击跳转至B站视频](https://www.bilibili.com/video/BV11C65BDEJx/?spm_id_from=333.337.search-card.all.click)
*   **原版硬件**：STM32F103C8T6 + STM32F103ZET6
*   **本项目适配硬件**：STM32F103C8T6 + **STM32F407VET6**

## 🔧 主要改动与适配点
本项目针对 F4 系列芯片进行了深度移植和优化，主要改动包括：

1.  **硬件底层重构**：重新配置 STM32F407 的时钟树，并根据 F4 的 Flash 扇区划分机制重写了 Flash 擦写底层驱动。
2.  **通信时序适配**：重新调整双芯片通信参数，使其严格适配 F4 外设时序。
3.  **BootLoader 地址映射**：重新计算并适配了 BootLoader 地址映射与固件偏移量，以适应 F4 的存储空间。

---

## ⚠️ 声明
本项目**仅用于个人学习、求职作品集展示**，**不进行任何商业售卖与盈利行为**。 




用到的硬件:STM32F103C8T6,STM32F407VET6,两个大夏龙雀LoRa无线模块(DX-LR20-433M22SP),1个AT24C02,1个W25Q64,面包板,杜邦线,USB-TTL串口,任意烧录器。
F407的FLASH分区：0x0800 0000-0x0800 7FFF 对应的BOOTLOADER程序，即F407_B2文件
                0x0800 8000-0x0800 FFFF  对应的恢复出厂区域  ，F407_B2-reset文件
                0x0800 1000-0x0801 FFFF  A程序（用户程序）区域 ，F407_A文件

F407_B2中keil配置<img width="929" height="704" alt="image" src="https://github.com/user-attachments/assets/c3946f9b-9755-429e-b9b4-d9d9d96f9462" />
F407_B2-reset中keil配置<img width="939" height="699" alt="image" src="https://github.com/user-attachments/assets/67c3a81e-3ab9-4cf1-acec-43a039e66069" />
F407_A和F407_A-2中keil配置<img width="924" height="688" alt="image" src="https://github.com/user-attachments/assets/36deb705-95b8-473b-96fd-80be6e226840" />
F407_A相当于F407初始的代码，F407_A-2是F407升级后的代码。
测试方法：
打开F103_UP：
首先，把main函数中这段代码的注释去掉<img width="1276" height="503" alt="image" src="https://github.com/user-attachments/assets/259cf211-c806-4104-9408-4630bedf6020" />，
然后在App_update.c的App_Update_Init（）函数中，把if判断语句加上注释，<img width="802" height="448" alt="image" src="https://github.com/user-attachments/assets/9718c3e3-650c-4eaa-9b1e-6d39659493a5" />
之后就可以烧录到F103了，这时需要把串口与F103连接起来，串口工具中会打印相关信息，我们现在就可以通过串口把F407要升级的程序存储在F103中，在串口工具的发送框中，输入  start:(bin文件大小)，比如F407_A-2编译生成的bin文件大小为15804，那么就是输入 start:15804  ,如何可以产生bin文件呢，打开keil中魔术棒，在USER中配置<img width="930" height="701" alt="image" src="https://github.com/user-attachments/assets/98a639cd-7753-4448-8b3e-da117f3ead00" />
在User Commond中输入  E:\keil5\ARM\ARMCC\bin\fromelf --bin -o "$L@L.bin" "#L"  ，注意要输入自己路径。

把F407_A,F407_B2,F407_B2-reset全烧录到F407开发板中


学习中遇到的小问题：注意要让F103与F407的频率一致，比如都为72Mhz,不然会出现F407不响应的情况。


