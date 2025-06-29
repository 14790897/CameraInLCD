/*
 * ST7735S LCD Test using ESP-IDF Official Driver
 * 使用ESP-IDF官方ST7735S驱动库的测试程序
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_check.h"  // 添加这个头文件以支持ESP_RETURN_ON_ERROR
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "esp_lcd_st7735.h"
#include "example_config.h"

static const char *TAG = "ST7735S_OFFICIAL";

// 全局句柄
static esp_lcd_panel_io_handle_t io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

// ST7735S 实际分辨率定义（覆盖config中的ILI9341设置）
#define ST7735S_LCD_H_RES 128
#define ST7735S_LCD_V_RES 160

// GPIO调试函数 - 检查引脚状态
static esp_err_t debug_gpio_status(void)
{
    ESP_LOGI(TAG, "=== GPIO引脚状态检查 ===");
    
    // 检查各个LCD控制引脚的当前状态
    int pins[] = {EXAMPLE_PIN_NUM_MOSI, EXAMPLE_PIN_NUM_SCLK, EXAMPLE_PIN_NUM_LCD_CS, 
                  EXAMPLE_PIN_NUM_LCD_DC, EXAMPLE_PIN_NUM_LCD_RST, EXAMPLE_PIN_NUM_BK_LIGHT};
    const char* pin_names[] = {"MOSI", "SCLK", "CS", "DC", "RST", "BL"};
    
    for (int i = 0; i < 6; i++) {
        int level = gpio_get_level((gpio_num_t)pins[i]);
        ESP_LOGI(TAG, "%s (GPIO%d): %s", pin_names[i], pins[i], level ? "HIGH" : "LOW");
    }
    
    ESP_LOGI(TAG, "=== GPIO状态检查完成 ===");
    return ESP_OK;
}

// 初始化SPI总线和LCD面板
static esp_err_t init_lcd_panel(void)
{
    ESP_LOGI(TAG, "=== 初始化ST7735S LCD面板 ===");
    ESP_LOGI(TAG, "使用ST7735S分辨率: %dx%d", ST7735S_LCD_H_RES, ST7735S_LCD_V_RES);
    

    // 1. 初始化SPI总线
    ESP_LOGI(TAG, "1. 初始化SPI总线");
    spi_bus_config_t bus_config = {
        .mosi_io_num = EXAMPLE_PIN_NUM_MOSI,
        .miso_io_num = -1, 
        .sclk_io_num = EXAMPLE_PIN_NUM_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = ST7735S_LCD_H_RES * ST7735S_LCD_V_RES * sizeof(uint16_t),
        .flags = SPICOMMON_BUSFLAG_MASTER,  // 明确指定为主机模式
    };
    
    // 先检查SPI总线是否已经初始化
    esp_err_t ret = spi_bus_initialize(SPI3_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI总线初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "✓ SPI总线初始化成功");
    
    ESP_LOGI(TAG, "2. 创建LCD面板IO");
    ESP_LOGI(TAG, "使用较低的SPI时钟频率进行调试: 1MHz");
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_PIN_NUM_LCD_DC,
        .cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS,
        .pclk_hz = 1 * 1000 * 1000,  // 降低到1MHz进行调试
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,  // 暂时不使用回调
        .user_ctx = NULL,
    };
    
    ESP_LOGI(TAG, "SPI配置 - DC:GPIO%d, CS:GPIO%d, 时钟:%dMHz", 
             io_config.dc_gpio_num, io_config.cs_gpio_num, io_config.pclk_hz/1000000);
    
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI3_HOST, 
                                  &io_config, &io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LCD面板IO创建失败: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "✓ LCD面板IO创建成功");
    
    ESP_LOGI(TAG, "3. 创建ST7735S面板");
    ESP_LOGI(TAG, "面板配置 - RST:GPIO%d, 色彩格式:RGB565, 位深:16", EXAMPLE_PIN_NUM_LCD_RST);
    
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,  // ST7735S通常使用BGR
        .bits_per_pixel = 16,  // RGB565
    };
    
    // 使用官方ST7735S驱动创建面板
    ret = esp_lcd_new_panel_st7735(io_handle, &panel_config, &panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ST7735S面板创建失败: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "✓ ST7735S面板创建成功");
    
    // 4. 重置和初始化面板
    ESP_LOGI(TAG, "4. 重置和初始化面板");
    ESP_LOGI(TAG, "执行硬件重置...");
    ret = esp_lcd_panel_reset(panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "面板重置失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 重置后等待一段时间
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI(TAG, "执行面板初始化...");
    
    ret = esp_lcd_panel_init(panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "面板初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "✓ 面板重置和初始化成功");
    
    // 5. 设置显示方向和镜像（重要！）
    ESP_LOGI(TAG, "5. 设置显示方向");
    ret = esp_lcd_panel_mirror(panel_handle, true, false);  // 水平镜像
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置镜像失败: %s", esp_err_to_name(ret));
    }
    
    ret = esp_lcd_panel_swap_xy(panel_handle, false);  // 不交换XY轴
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置XY轴失败: %s", esp_err_to_name(ret));
    }
    
    ret = esp_lcd_panel_invert_color(panel_handle, true);  // 尝试颜色反转
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置颜色反转失败: %s", esp_err_to_name(ret));
    }
    ESP_LOGI(TAG, "✓ 显示方向设置完成");
    
    // 6. 开启显示
    ESP_LOGI(TAG, "6. 开启显示");
    ret = esp_lcd_panel_disp_on_off(panel_handle, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "开启显示失败: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "✓ 显示已开启");
    
    return ESP_OK;
}

// 初始化背光GPIO
static esp_err_t init_backlight(void)
{
    ESP_LOGI(TAG, "初始化背光控制");
    gpio_config_t bl_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&bl_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "背光GPIO配置失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 开启背光
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
    ESP_LOGI(TAG, "✓ 背光已开启");
    
    return ESP_OK;
}

// 填充纯色
static esp_err_t fill_color(uint16_t color)
{
    ESP_LOGI(TAG, "填充颜色: 0x%04X (分辨率:%dx%d)", color, ST7735S_LCD_H_RES, ST7735S_LCD_V_RES);
    
    // 分配像素缓冲区 - 使用ST7735S的实际分辨率
    size_t pixel_count = ST7735S_LCD_H_RES * ST7735S_LCD_V_RES;
    ESP_LOGI(TAG, "分配像素缓冲区: %d 像素 (%d 字节)", pixel_count, pixel_count * sizeof(uint16_t));
    
    uint16_t *pixel_buffer = heap_caps_malloc(pixel_count * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (!pixel_buffer) {
        ESP_LOGE(TAG, "像素缓冲区分配失败");
        return ESP_ERR_NO_MEM;
    }
    
    // 填充缓冲区
    for (size_t i = 0; i < pixel_count; i++) {
        pixel_buffer[i] = color;
    }
    ESP_LOGI(TAG, "缓冲区填充完成，开始绘制...");
    
    // ST7735S可能需要显示偏移，尝试不同的起始位置
    int x_offset = 2;  // ST7735S通常有2像素X偏移
    int y_offset = 1;  // ST7735S通常有1像素Y偏移
    
    ESP_LOGI(TAG, "尝试带偏移的绘制: X偏移=%d, Y偏移=%d", x_offset, y_offset);
    
    // 绘制到屏幕 - 使用ST7735S分辨率和偏移
    esp_err_t ret = esp_lcd_panel_draw_bitmap(panel_handle, 
                                             x_offset, y_offset, 
                                             x_offset + ST7735S_LCD_H_RES, 
                                             y_offset + ST7735S_LCD_V_RES, 
                                             pixel_buffer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "带偏移绘制位图失败: %s，尝试无偏移绘制", esp_err_to_name(ret));
        
        // 如果带偏移失败，尝试无偏移绘制
        ret = esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 
                                       ST7735S_LCD_H_RES, ST7735S_LCD_V_RES, 
                                       pixel_buffer);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "无偏移绘制位图也失败: %s", esp_err_to_name(ret));
            free(pixel_buffer);
            return ret;
        }
        ESP_LOGI(TAG, "无偏移绘制成功");
    } else {
        ESP_LOGI(TAG, "带偏移绘制成功");
    }
    
    free(pixel_buffer);
    ESP_LOGI(TAG, "✓ 颜色填充完成");
    return ESP_OK;
}

// 显示多色测试（用于调试）
static esp_err_t run_color_display_test(void)
{
    ESP_LOGI(TAG, "=== 开始多色显示测试 ===");
    
    // 测试多种颜色以确认显示是否工作
    uint16_t test_colors[] = {
        0xF800,  // 红色 RGB565
        0x07E0,  // 绿色 RGB565  
        0x001F,  // 蓝色 RGB565
        0xFFFF,  // 白色 RGB565
        0x0000,  // 黑色 RGB565
        0xFFE0,  // 黄色 RGB565
        0xF81F,  // 品红色 RGB565
        0x07FF,  // 青色 RGB565
    };
    
    const char* color_names[] = {
        "红色", "绿色", "蓝色", "白色", 
        "黑色", "黄色", "品红色", "青色"
    };
    
    for (int i = 0; i < 8; i++) {
        ESP_LOGI(TAG, "测试颜色 %d/8: %s (0x%04X)", i+1, color_names[i], test_colors[i]);
        
        esp_err_t ret = fill_color(test_colors[i]);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "%s填充失败", color_names[i]);
            return ret;
        }
        
        // 每种颜色显示2秒
        ESP_LOGI(TAG, "✓ %s显示完成，等待2秒...", color_names[i]);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
    ESP_LOGI(TAG, "✓ 所有颜色测试完成");
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "🚀 ST7735S LCD多色显示测试程序");
    ESP_LOGI(TAG, "ST7735S分辨率: %dx%d", ST7735S_LCD_H_RES, ST7735S_LCD_V_RES);
    ESP_LOGI(TAG, "调试模式SPI时钟: 1 MHz (降低频率用于调试)");
    ESP_LOGI(TAG, "测试内容: 循环显示多种颜色");

    // 检查GPIO状态
    debug_gpio_status();
    
    // 初始化背光
    // if (init_backlight() != ESP_OK) {
    //     ESP_LOGE(TAG, "背光初始化失败");
    //     return;
    // }
    
    // 初始化LCD面板
    if (init_lcd_panel() != ESP_OK) {
        ESP_LOGE(TAG, "LCD面板初始化失败");
        return;
    }
    
    // 运行多色显示测试
    if (run_color_display_test() != ESP_OK) {
        ESP_LOGE(TAG, "多色显示测试失败");
        return;
    }
    
    // 循环显示不同颜色
    ESP_LOGI(TAG, "开始循环显示...");
    uint16_t colors[] = {0xF800, 0x07E0, 0x001F, 0xFFFF, 0x0000};  // 红绿蓝白黑
    const char* names[] = {"红色", "绿色", "蓝色", "白色", "黑色"};
    int color_index = 0;
    
    while (1) {
        ESP_LOGI(TAG, "循环显示: %s", names[color_index]);
        fill_color(colors[color_index]);
        
        color_index = (color_index + 1) % 5;
        vTaskDelay(pdMS_TO_TICKS(3000));  // 每3秒切换一次颜色
    }
}
