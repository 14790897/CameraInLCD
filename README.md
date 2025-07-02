| Supported Targets | ESP32-S3 |
| ----------------- | -------- |

# ESP32-S3 DVP Camera + LCD Display Project

## 📋 项目概述

基于ESP32-S3的DVP摄像头和LCD显示屏项目

本项目提供了完整的ESP32-S3 DVP摄像头和LCD显示解决方案，包含：
- OV7670 DVP摄像头支持
- LCD驱动芯片支持（ST7735S）
- 完整的硬件测试和诊断工具
- 模块化的代码结构，便于选择不同功能

🎥 **项目演示视频**: [B站视频 - ESP32-S3 DVP摄像头+LCD显示项目演示](https://www.bilibili.com/video/BV1zwK2zFEBt/)

⚠️ **重要提示**: 使用前请先阅读 [核心代码修改说明(必看)](#-核心代码修改说明必看)，了解关键的SCCB驱动修改内容！

## 📋 快速导航

🎥 **项目演示**: [B站视频演示](https://www.bilibili.com/video/BV1zwK2zFEBt/) - 完整功能展示

🔥 **必读内容**:

- [🔧 核心代码修改说明(必看)](#-核心代码修改说明必看) - SCCB驱动关键修改
- [🔧 硬件配置](#-硬件配置) - 引脚连接和上拉电阻配置
- [💡 SCCB连接技巧](#如何判断是否需要添加上拉电阻) - 初始化后可断开SCCB的有趣现象
- [🎨 QQVGA彩色条纹](#qqvga分辨率彩色条纹现象--有趣现象) - QQVGA分辨率的特殊视觉效果

📖 **功能说明**:

- [🧪 测试模式说明](#-测试模式说明) - 不同功能模块的使用方法
- [🔍 故障排除](#-故障排除) - 常见问题解决方案

🛠️ **开发相关**:

- [🔧 自定义配置](#-自定义配置) - 修改引脚和参数
- [📝 开发说明](#-开发说明) - 扩展和定制指南

## 🔧 硬件配置

### ESP32-S3开发板引脚定义

#### LCD显示屏连接 (SPI接口)

```
ESP32-S3 引脚    →    LCD模块引脚
GPIO13 (SCLK)    →    SCL/SCLK/CLK
GPIO14 (MOSI)    →    SDA/MOSI
GPIO17 (MISO)    →    SDO/MISO (可选)
GPIO16 (CS)      →    CS
GPIO48 (DC)      →    DC/RS
GPIO42 (RST)     →    RESET/RST
GPIO-1 (背光)    →    LED/BL (未连接)
3.3V             →    VCC
GND              →    GND
```

#### OV7670摄像头连接 (DVP接口)

```
ESP32-S3 引脚    →    OV7670引脚
GPIO12           →    PWDN
GPIO11           →    RESET
GPIO2            →    SCCB_SCL (I2C时钟) + 上拉电阻
GPIO1            →    SCCB_SDA (I2C数据) + 上拉电阻
GPIO38           →    XCLK (主时钟)
GPIO21           →    PCLK (像素时钟)
GPIO39           →    VSYNC (垂直同步)
GPIO40           →    HSYNC (水平同步)
GPIO8            →    D0 (数据线0)
GPIO9            →    D1 (数据线1)
GPIO10           →    D2 (数据线2)
GPIO4            →    D3 (数据线3)
GPIO3            →    D4 (数据线4)
GPIO45           →    D5 (数据线5)
GPIO47           →    D6 (数据线6)
GPIO46           →    D7 (数据线7)
```

⚠️ **重要**: OV7670摄像头的SCCB_SCL和SCCB_SDA引脚需要手动添加上拉电阻！

**上拉电阻连接方法**:

- SCCB_SCL (GPIO2) → 通过4.7kΩ电阻连接到3.3V
- SCCB_SDA (GPIO1) → 通过4.7kΩ电阻连接到3.3V

**为什么需要上拉电阻**:

- OV7670的SCCB接口本质上是I2C协议
- I2C总线需要上拉电阻才能正常工作
- 某些OV7670模块没有内置上拉电阻，需要外部添加

**如何判断是否需要添加上拉电阻**:

📋 **检测方法**:

1. **硬件检测法** (推荐，最准确)

   ```text
   步骤1: 断开摄像头与ESP32-S3的所有连接
   步骤2: 使用万用表测量摄像头模块上SCCB_SCL和SCCB_SDA引脚的电阻
   步骤3: 将万用表的一端接触SCCB_SCL引脚，另一端接触VCC (3.3V)引脚
   步骤4: 读取电阻值并判断
   ```

   **判断标准**:
   - ✅ **需要外部上拉**: 电阻值显示为无穷大 (∞Ω 或 OL)
   - ⚠️ **模块异常**: 电阻值为 20kΩ - 60kΩ (内部上拉异常，建议更换模块)
   - ❓ **可能有内置上拉**: 电阻值为 4.7kΩ - 10kΩ (但仍建议添加外部上拉以确保稳定)

💡 **有趣现象**: 摄像头初始化完成后SCCB连接可选

一个有趣的技术现象是，OV7670摄像头在完成初始化配置后，实际上可以断开SCCB_SCL和SCCB_SDA连接，摄像头仍能正常工作！

**原理解释**:

- SCCB接口主要用于摄像头的寄存器配置（分辨率、格式、曝光等）
- 一旦配置完成，摄像头会按照设定参数自主工作
- 图像数据通过DVP数据线(D0-D7)和控制信号(PCLK/VSYNC/HSYNC)传输
- SCCB连接只在需要重新配置摄像头参数时才必需

💡 **另一个有趣现象**: QQVGA分辨率的彩色条纹效果

在测试不同分辨率时发现，当摄像头设置为QQVGA (160x120)分辨率时，会产生一个意外但有趣的现象：

**现象描述**:

- 输出画面呈现为彩色条纹图案
- 条纹颜色会随着镜头前物体的变化而动态改变
- 虽然不是正常的图像输出，但可以感知到场景的变化

**技术分析**:

- QQVGA分辨率在某些OV7670配置下可能存在时序不匹配
- 数据传输时序与LCD显示期望不完全对齐
- 造成了像素数据的重新排列，形成条纹效果

**实际用途**:

- 🎨 **创意显示**: 可以作为独特的视觉效果使用
- 🔍 **运动检测**: 虽然不显示图像，但能感知场景变化
- 🧪 **教学演示**: 很好地展示了分辨率和时序的重要性

**解决方案**:

- 使用QVGA (320x240)分辨率可获得正常图像输出

### 支持的LCD规格

- **ST7735S**: 128x160像素，RGB565 (当前配置)

## 🚀 快速开始

### 环境要求

- ESP-IDF v5.0+
- ESP32-S3开发板
- 支持的LCD显示屏
- OV7670摄像头模块 (无FIFO)

### 编译和烧录

⚠️ **重要**: ST7735S驱动需要手动添加依赖！

```bash
# 克隆项目
git clone https://github.com/14790897/CameraInLCD
cd CameraInLCD

# 配置ESP-IDF环境
. $IDF_PATH/export.sh

# 添加ST7735S驱动依赖 (必须执行)
idf.py add-dependency "teriyakigod/esp_lcd_st7735^0.0.1"

# 编译项目
idf.py build

# 烧录到设备
idf.py flash

# 查看串口输出
idf.py monitor
```

## 📁 项目结构

### 主要源文件

| 文件名 | 功能描述 | 推荐用途 |
|--------|----------|----------|
| `dvp_lcd_main.c` | 完整的摄像头+LCD组合功能 | 最终产品功能 |
| `st7735s_official_test.c` | ST7735S驱动测试 | ST7735S显示屏测试 |
| `camera_test.c` | 摄像头独立测试 | 摄像头功能验证 |

### 配置文件选择

在 `main/CMakeLists.txt` 中选择要编译的模块：

```cmake
# 取消注释对应的行来选择功能模块

# ST7735S驱动测试（当前默认）
idf_component_register(SRCS "st7735s_official_test.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_mm esp_driver_spi esp_lcd esp32-camera driver log teriyakigod__esp_lcd_st7735
                       )

# 其他模块（注释状态）
# idf_component_register(SRCS "dvp_lcd_main.c" ...)
# idf_component_register(SRCS "lcd_simple.c" ...)
```

## 🧪 测试模式说明

### 1. ST7735S第三方驱动测试 (`st7735s_official_test.c`)

- **用途**: 使用ESP-IDF第三方ST7735S驱动库
- **功能**:
  - 标准化的驱动初始化
  - 颜色填充测试（红、绿、蓝）
  - 彩色矩形图案测试
- **适用**: ST7735S显示屏的标准测试

### 2. 摄像头测试 (`camera_test.c`)

- **用途**: 摄像头独立测试
- **功能**:
  - 摄像头初始化
  - 帧捕获测试
  - 图像格式验证
- **适用**: 摄像头功能验证

### 3. 完整功能 (`dvp_lcd_main.c`)

- **用途**: 摄像头+LCD完整功能
- **功能**:
  - OV7670摄像头初始化
  - 实时图像采集
  - LCD显示输出
- **适用**: 最终产品功能

## 🔍 故障排除

### 编译错误

1. **ST7735S头文件找不到**
   
   **错误信息**: `fatal error: esp_lcd_st7735.h: No such file or directory`
   
   **解决方案**:
   ```bash
   # 添加ST7735S驱动依赖
   idf.py add-dependency "teriyakigod/esp_lcd_st7735^0.0.1"
   
   # 清理后重新编译
   idf.py fullclean
   idf.py build
   ```

### LCD无显示问题

1. **检查硬件连接**
   - 确认所有引脚连接正确 (重要)
   - 检查电源是否为3.3V（不是5V）
   - 确认GND连接

### 摄像头问题

1. **QQVGA分辨率彩色条纹现象** 💡 **有趣现象**

   **问题现象**: 设置QQVGA (160x120)分辨率时输出彩色条纹而非正常图像
   
   **技术原因**:
   - QQVGA分辨率在某些OV7670配置下存在时序不匹配
   - 数据传输时序与LCD显示期望不完全对齐
   - 像素数据重新排列形成动态变化的条纹效果
   
   **解决方案**:

   ```c
   // 推荐使用QVGA分辨率获得正常图像
   #define EXAMPLE_CAM_FORMAT "DVP_8bit_20Minput_RGB565_320x240_30fps"
   
   // 避免使用QQVGA（除非需要特殊效果）
   // #define EXAMPLE_CAM_FORMAT "DVP_8bit_20Minput_RGB565_160x120_30fps"
   ```

2. **OV7670硬件问题排查** ⚠️ **重要经验**

   **问题现象**: 摄像头无法正常工作

   **解决方案**:

   **步骤1: 检查上拉电阻**
   - **必须添加外部上拉电阻**: SCCB_SCL和SCCB_SDA都需要通过4.7kΩ电阻连接到3.3V
   - **检查内部上拉电阻**: 断开摄像头与ESP32-S3的连接，用万用表测量SCCB_SCL和SCCB_SDA引脚的电阻
     - ✅ **正常**: 电阻值为无穷大（说明没有内部上拉电阻，需要外部添加）
     - ❌ **异常**: 电阻值为20-60千欧（说明内部上拉电阻异常，模块可能损坏）

   **步骤2: 硬件连接**
   ```
   3.3V ──┬── 4.7kΩ ── SCCB_SCL (GPIO2)
          └── 4.7kΩ ── SCCB_SDA (GPIO1)
   ```

   **步骤3: 软件配置**
   - **更换摄像头模块**: 如果发现内部上拉电阻异常，建议更换新的OV7670模块
   - **调整输出格式**: 如果出现彩色条纹，尝试将输出格式改为QVGA (320x240)

   **实际案例**:
   ```
   问题摄像头: 内部上拉电阻 20-60kΩ → 无法正常工作
   正常摄像头: 内部上拉电阻 ∞Ω + 外部4.7kΩ上拉 → 工作正常，输出QVGA格式
   ```

## 📦 依赖组件

项目使用以下ESP-IDF组件：

- `espressif/esp32-camera^2.0.0` - 摄像头驱动 (自动安装)
- `espressif/esp_lcd_ili9341^1.0.0` - ILI9341 LCD驱动 (自动安装)
- `teriyakigod/esp_lcd_st7735^0.0.1` - ST7735S LCD驱动 ⚠️ **需要手动添加**

### ST7735S依赖安装说明

⚠️ **重要**: ST7735S驱动不会自动安装，必须手动添加依赖！

**安装命令**:

```bash
# 在项目根目录执行
idf.py add-dependency "teriyakigod/esp_lcd_st7735^0.0.1"
```

**常见问题**:

- 如果忘记添加依赖，编译时会出现 `esp_lcd_st7735.h: No such file or directory` 错误
- 依赖添加后会自动更新 `main/idf_component.yml` 文件
- 首次编译会自动下载ST7735S驱动组件

**验证安装**:

```bash
# 检查依赖是否正确添加
cat main/idf_component.yml
```

应该能看到类似以下内容：

```yaml
dependencies:
  teriyakigod/esp_lcd_st7735: "^0.0.1"
```

## 🔧 自定义配置


### 修改引脚定义

在 `main/example_config.h` 中修改对应的GPIO定义。

### 修改SPI时钟频率

```c
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)  // 20MHz
```

**注意**: 如果遇到彩色条纹问题，建议使用QVGA (320x240) 格式。

## 🔧 核心代码修改说明(必看)

### SCCB_Read 函数优化

**文件位置**: `managed_components/espressif__esp32-camera/driver/sccb-ng.c`

**修改原因**: 为了提高SCCB/I2C通信的兼容性和稳定性，我们对原始的 `SCCB_Read` 函数进行了关键修改。

**原始代码问题**:

- 使用单次 `i2c_master_transmit_receive` 调用
- 在某些摄像头模块或特定硬件配置下可能出现通信不稳定

**修改后的实现**:

```c
esp_err_t SCCB_Read(uint8_t slv_addr, uint8_t reg, uint8_t *data)
{
    esp_err_t ret = ESP_FAIL;
    
    // 分步骤进行I2C通信，提高兼容性
    
    // 步骤1: 发送寄存器地址
    ret = i2c_master_transmit(s_i2c_handle, slv_addr, &reg, 1, SCCB_FREQ);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 步骤2: 读取数据
    ret = i2c_master_receive(s_i2c_handle, slv_addr, data, 1, SCCB_FREQ);
    
    return ret;
}
```

## 📝 开发说明

### 添加新的LCD驱动

1. 在 `main/` 目录创建新的文件
2. 在 `main/CMakeLists.txt` 中添加编译选项
3. 根据需要在 `main/idf_component.yml` 中添加依赖

### 代码风格

- 使用ESP-IDF标准的错误处理 (`ESP_RETURN_ON_ERROR`)
- 详细的日志输出用于调试
- 模块化设计，便于功能选择

## 📄 许可证

本项目基于 Apache 2.0 许可证开源。

## 🤝 贡献

欢迎提交Issue和Pull Request来改进项目。

## 📞 支持

如有问题，请查看故障排除部分或提交Issue。
