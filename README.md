# TEM Watch

基于 STM32F103C8T6 的嵌入式智能手表项目。工程使用 STM32 HAL、RTC、I2C OLED、MPU6050 和按键扫描实现时钟、设置菜单及多种小功能。

## 功能

- RTC 数字时钟与日期显示
- 日期时间设置与掉电备份
- 秒表
- 手电筒控制
- MPU6050 姿态数据显示
- 动态表情动画
- 水平仪
- 恐龙小游戏
- 三按键菜单导航

## 硬件平台

| 模块 | 型号或接口 | 说明 |
| --- | --- | --- |
| MCU | STM32F103C8T6 | Cortex-M3，LQFP48 |
| OLED | SSD1306，128 x 64 | I2C1，地址 `0x3C` |
| 姿态传感器 | MPU6050 | I2C2，地址 `0x68`（AD0 接地） |
| RTC | STM32 内置 RTC | 使用 LSE 32.768 kHz 晶振 |
| KEY1 | PA6 | 低电平有效 |
| KEY2 | PA4 | 低电平有效 |
| KEY3 | PB1 | 低电平有效 |
| LED | PC13 | 低电平点亮 |
| 手电筒输出 | PB12 | GPIO 输出 |
| 调试串口 | USART1 | TX：PA9，RX：PA10 |
| ADC 输入 | PA0 / ADC1_IN0 | 模拟量采集 |

TIM1 每 5 ms 产生一次中断，用于标记 MPU6050 数据采样。I2C1 连接 OLED，I2C2 连接 MPU6050。

## 目录结构

```text
TEM_watch/
├── APP/
│   ├── Key/            # 按键扫描
│   ├── MPU6050/        # MPU6050 驱动与姿态计算
│   ├── features/       # 秒表、手电筒、小游戏等功能
│   ├── fonts/          # OLED 字库和图标
│   ├── menu/           # 页面状态、菜单和日期时间设置
│   ├── oled/           # OLED 驱动及绘图接口
│   └── screen/         # 屏幕页面模块
├── Core/               # STM32CubeMX 生成的外设初始化与主程序
├── Drivers/            # STM32 HAL 与 CMSIS
├── cmake/              # CMake 和交叉编译工具链配置
├── CMakeLists.txt      # 项目源文件与包含目录配置
├── CMakePresets.json   # Debug / Release 构建预设
├── STM32F103XX_FLASH.ld
├── TEM_watch.ioc       # STM32CubeMX 工程配置
└── date_time_setting_design.md
```

## 构建

### 环境要求

- CMake 3.22 或更高版本
- Ninja
- `arm-none-eabi-gcc`，并确保已加入系统 `PATH`

### Debug 构建

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

生成文件位于：

```text
build/Debug/TEM_watch.elf
```

### Release 构建

```powershell
cmake --preset Release
cmake --build --preset Release
```

生成文件位于：

```text
build/Release/TEM_watch.elf
```

构建完成后，可使用 STM32CubeProgrammer、OpenOCD 或 ST-Link 工具将 ELF 文件下载到单片机。

## 按键操作

主菜单使用三枚低电平有效按键：

| 按键 | 菜单中的功能 |
| --- | --- |
| KEY1 | 向左切换图标 |
| KEY2 | 向右切换图标 |
| KEY3 | 进入当前功能 |

功能页面中的按键含义由各页面自行处理，通常 KEY3 用于返回菜单。日期时间设置页面中，KEY1 / KEY2 调整当前数值，KEY3 切换字段并在最后一个字段确认保存。

## 开发说明

- `Core/` 和 `Drivers/` 中的基础外设代码由 STM32CubeMX 生成，业务代码尽量放在 `APP/` 中。
- 修改引脚、时钟或外设配置时，应通过 `TEM_watch.ioc` 完成并重新生成代码。
- 新增 `.c` 文件后，需要在根目录 `CMakeLists.txt` 的 `target_sources()` 中添加源文件。
- 新增头文件目录后，需要在 `target_include_directories()` 中添加包含路径。
- `build/` 是编译输出目录，已通过 `.gitignore` 排除，不应提交到 Git。

## 第三方代码

工程包含 STMicroelectronics 的 STM32 HAL、CMSIS 及相关启动文件，其授权信息位于 `Drivers/` 下各组件附带的 `LICENSE.txt` 中。
