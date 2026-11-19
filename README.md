# PWM-Music (电子设计实验课)

## 项目简介
这是一个基于 Keil MDK 和 STM32 编写的电子设计实验小项目。
利用 STM32 的定时器产生 PWM 信号，驱动蜂鸣器模块，实现音乐《蕾西》的旋律播放。

## 开发环境
*   **开发工具**：Keil MDK-ARM
*   **配置工具**：STM32CubeMX (Lab9.ioc)
*   **芯片型号**：STM32 (根据你实际的芯片型号修改，比如 STM32F103C8T6)
*   **硬件模块**：有源/无源蜂鸣器模块

## 工程结构
*   `Core/`：STM32 核心代码，包含主函数和业务逻辑
*   `Drivers/`：STM32 HAL 库驱动文件
*   `MDK-ARM/`：Keil 工程文件（可用 Keil 直接打开 `.uvprojx` 文件）
*   `Lab9.ioc`：STM32CubeMX 配置文件

## 如何运行
1. 克隆本项目到本地。
2. 使用 STM32CubeMX 打开 `Lab9.ioc` 可查看引脚配置。
3. 使用 Keil MDK 打开 `MDK-ARM` 文件夹下的工程文件，编译并下载到单片机即可。
