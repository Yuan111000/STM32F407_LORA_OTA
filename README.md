# STM32F407 LoRa OTA 远程升级 (基于LLCC68)

## 🔧 主要改动与适配点
1. **硬件底层重构**：重新配置 STM32F407 的时钟树，并根据 F4 的 Flash 扇区划分机制重写了 Flash 擦写底层驱动。
2. **通信时序适配**：重新调整双芯片通信参数，使其严格适配 F4 外设时序。
3. **BootLoader 地址映射**：重新计算并适配了 BootLoader 地址映射与固件偏移量，以适应 F4 的存储空间。

> ⚠️ **声明**：本项目**仅用于个人学习、求职作品集展示**，**不进行任何商业售卖与盈利行为**。

---

## 🛠️ 硬件与环境
用到的硬件：STM32F103C8T6、STM32F407VET6、两个大夏龙雀LoRa无线模块(DX-LR20-433M22SP)、1个AT24C02、1个W25Q64、面包板、杜邦线、USB-TTL串口、任意烧录器。

**F407的FLASH分区：**
*   `0x0800 0000 ~ 0x0800 7FFF`：对应的BOOTLOADER程序（F407_B2文件）
*   `0x0800 8000 ~ 0x0800 FFFF`：对应的恢复出厂区域（F407_B2-reset文件）
*   `0x0801 0000 ~ 0x0801 FFFF`：App程序（用户程序）区域（F407_A文件）

---

## 🚀 测试方法说明

### 1. Keil 工程配置说明
*   `F407_B2` 中配置：<img width="929" height="704" alt="image" src="https://github.com/user-attachments/assets/c3946f9b-9755-429e-b9b4-d9d9d96f9462" />
*   `F407_B2-reset` 中配置：<img width="939" height="699" alt="image" src="https://github.com/user-attachments/assets/67c3a81e-3ab9-4cf1-acec-43a039e66069" />
*   `F407_A` 中配置：<img width="924" height="688" alt="image" src="https://github.com/user-attachments/assets/36deb705-95b8-473b-96fd-80be6e226840" />
*   *(注：F407_A 相当于 F407 初始的代码，F407_A-2 为 F407 升级后的代码。)*

### 2. 测试代码修改步骤
打开 `f103_up` 文件：
1.  首先，把 `main` 函数中这段代码的注释去掉。<img width="1276" height="503" alt="image" src="https://github.com/user-attachments/assets/259cf211-c806-4104-9408-4630bedf6020" />，
2.  然后在 `App_update_Init()` 函数中，把 `if` 判断语句加上注释。<img width="802" height="448" alt="image" src="https://github.com/user-attachments/assets/9718c3e3-650c-4eaa-9b1e-6d39659493a5" />

### 3. 烧录F103
之后就可以烧录到F103了，这时需要把串口与F103连接起来，串口工具中会打印相关信息，我们现在就可以通过串口把F407要升级的程序存储在F103中，在串口工具的发送框中，输入  start:(bin文件大小)，比如F407_A-2编译生成的bin文件大小为15804，那么就是输入 start:15804 ,把文件传进去后就可以把代码恢复原样，即 将main函数中刚刚去掉注释的代码再次注释，（这段代码主要就是让你进入把程序存入F103中），以及App_update.c的App_Update_Init（）函数中把if语句的注释解开（if语句中的数字要与bin文件的大小对应上），再次烧录，这样就不用重复的给F103下载程序了。

### 4. 生成 .bin 文件说明
如何可以产生bin文件呢，打开keil中魔术棒，在USER中配置<img width="930" height="701" alt="image" src="https://github.com/user-attachments/assets/98a639cd-7753-4448-8b3e-da117f3ead00" />
在User Commond中输入  E:\keil5\ARM\ARMCC\bin\fromelf --bin -o "$L@L.bin" "#L"  ，注意要输入自己路径。

### 5. 烧录F407
把F407_A,F407_B2,F407_B2-reset全烧录到F407开发板中，注意FLASH的分配，上文也提到过，把这些都做好后，把串口与F407连接起来，在串口的输入框中输入 cmd 即可实现程序升级。
学习中遇到的小问题：注意要让F103与F407的频率一致，比如都为72Mhz,不然会出现F407不响应的情况。




