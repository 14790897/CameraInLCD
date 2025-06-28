/*
 * DVP Camera + ST7735S LCD Integration
 * 摄像头与ST7735S LCD显示集成
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "esp_cache.h"
#include "driver/ledc.h"
#include "esp_camera.h"
#include "esp_lcd_st7735.h"
#include "example_config.h"

// ST7735S 实际分辨率定义
#define ST7735S_LCD_H_RES 128
#define ST7735S_LCD_V_RES 160

static const char *TAG = "dvp_camera_st7735";

// Camera initialization function for ESP32-S3
static esp_err_t example_camera_init(void)
{
    ESP_LOGI(TAG, "=== Camera Initialization ===");

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = EXAMPLE_ISP_DVP_CAM_D0_IO;
    config.pin_d1 = EXAMPLE_ISP_DVP_CAM_D1_IO;
    config.pin_d2 = EXAMPLE_ISP_DVP_CAM_D2_IO;
    config.pin_d3 = EXAMPLE_ISP_DVP_CAM_D3_IO;
    config.pin_d4 = EXAMPLE_ISP_DVP_CAM_D4_IO;
    config.pin_d5 = EXAMPLE_ISP_DVP_CAM_D5_IO;
    config.pin_d6 = EXAMPLE_ISP_DVP_CAM_D6_IO;
    config.pin_d7 = EXAMPLE_ISP_DVP_CAM_D7_IO;
    config.pin_xclk = EXAMPLE_ISP_DVP_CAM_XCLK_IO;
    config.pin_pclk = EXAMPLE_ISP_DVP_CAM_PCLK_IO;
    config.pin_vsync = EXAMPLE_ISP_DVP_CAM_VSYNC_IO;
    config.pin_href = EXAMPLE_ISP_DVP_CAM_HSYNC_IO;
    config.pin_sccb_sda = EXAMPLE_ISP_DVP_CAM_SCCB_SDA_IO;
    config.pin_sccb_scl = EXAMPLE_ISP_DVP_CAM_SCCB_SCL_IO;
    config.pin_pwdn = EXAMPLE_ISP_DVP_CAM_PWDN_IO;
    config.pin_reset = EXAMPLE_ISP_DVP_CAM_RESET_IO;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_QQVGA;    // 160x120 for ST7735S
    config.pixel_format = PIXFORMAT_RGB565; // RGB565 format
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 2;

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return err;
    }

    // Get camera sensor
    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return ESP_FAIL;
    }

    // OV7670 specific settings
    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);
    s->set_gainceiling(s, GAINCEILING_128X);
    s->set_colorbar(s, 0);
    s->set_whitebal(s, 1);
    s->set_gain_ctrl(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_hmirror(s, 0);
    s->set_vflip(s, 0);

    ESP_LOGI(TAG, "Camera initialized successfully");
    return ESP_OK;
}
// ST7735S LCD initialization function
static esp_err_t init_st7735s_lcd(esp_lcd_panel_handle_t *panel_handle)
{
    ESP_LOGI(TAG, "=== 初始化ST7735S LCD面板 ===");

    // 1. 初始化SPI总线
    ESP_LOGI(TAG, "1. 初始化SPI总线");
    spi_bus_config_t bus_config = {
        .mosi_io_num = EXAMPLE_PIN_NUM_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = EXAMPLE_PIN_NUM_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = ST7735S_LCD_H_RES * ST7735S_LCD_V_RES * sizeof(uint16_t),
        .flags = SPICOMMON_BUSFLAG_MASTER,
    };

    esp_err_t ret = spi_bus_initialize(SPI3_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI总线初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "✓ SPI总线初始化成功");

    // 2. 创建LCD面板IO
    ESP_LOGI(TAG, "2. 创建LCD面板IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_PIN_NUM_LCD_DC,
        .cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS,
        .pclk_hz = 10 * 1000 * 1000, // 10MHz
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
    };

    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI3_HOST, &io_config, &io_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "LCD面板IO创建失败: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "✓ LCD面板IO创建成功");

    // 3. 创建ST7735S面板
    ESP_LOGI(TAG, "3. 创建ST7735S面板");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };

    ret = esp_lcd_new_panel_st7735(io_handle, &panel_config, panel_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "ST7735S面板创建失败: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "✓ ST7735S面板创建成功");

    // 4. 重置和初始化面板
    ESP_LOGI(TAG, "4. 重置和初始化面板");
    ret = esp_lcd_panel_reset(*panel_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "面板重置失败: %s", esp_err_to_name(ret));
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    ret = esp_lcd_panel_init(*panel_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "面板初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "✓ 面板重置和初始化成功");

    // 5. 开启显示
    ESP_LOGI(TAG, "5. 开启显示");
    ret = esp_lcd_panel_disp_on_off(*panel_handle, true);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "开启显示失败: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "✓ 显示已开启");

    return ESP_OK;
}

void app_main(void)
{
    esp_lcd_panel_handle_t panel_handle = NULL;
    void *frame_buffer = NULL;

    ESP_LOGI(TAG, "=== DVP Camera + ST7735S LCD Integration ===");

    // 初始化ST7735S LCD
    ESP_ERROR_CHECK(init_st7735s_lcd(&panel_handle));

    // 分配帧缓冲区 - 使用ST7735S分辨率
    size_t frame_buffer_size = ST7735S_LCD_H_RES * ST7735S_LCD_V_RES * sizeof(uint16_t);
    frame_buffer = heap_caps_malloc(frame_buffer_size, MALLOC_CAP_DMA);
    if (frame_buffer == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate frame buffer");
        return;
    }
    ESP_LOGI(TAG, "Frame buffer allocated: %zu bytes for %dx%d display",
             frame_buffer_size, ST7735S_LCD_H_RES, ST7735S_LCD_V_RES);

    // 初始化摄像头
    ESP_ERROR_CHECK(example_camera_init());

    ESP_LOGI(TAG, "=== Starting Camera Preview ===");

    // 主循环 - 获取摄像头图像并显示到LCD
    while (1)
    {
        camera_fb_t *pic = esp_camera_fb_get();
        if (pic) {
            // 检查图像尺寸是否匹配
            if (pic->width == 160 && pic->height == 120 && pic->format == PIXFORMAT_RGB565)
            {
                // 图像尺寸需要调整到ST7735S分辨率 (128x160)
                // 从160x120裁剪/缩放到128x160
                uint16_t *src = (uint16_t *)pic->buf;
                uint16_t *dst = (uint16_t *)frame_buffer;

                // 简单的裁剪策略：从中心裁剪128x120，然后拉伸到128x160
                int src_start_x = (160 - 128) / 2; // 从x=16开始

                for (int dst_y = 0; dst_y < ST7735S_LCD_V_RES; dst_y++)
                {
                    int src_y = (dst_y * 120) / ST7735S_LCD_V_RES; // 拉伸映射
                    for (int dst_x = 0; dst_x < ST7735S_LCD_H_RES; dst_x++)
                    {
                        int src_x = src_start_x + dst_x;
                        dst[dst_y * ST7735S_LCD_H_RES + dst_x] =
                            src[src_y * 160 + src_x];
                    }
                }

                // 显示到LCD
                esp_lcd_panel_draw_bitmap(panel_handle, 0, 0,
                                          ST7735S_LCD_H_RES, ST7735S_LCD_V_RES,
                                          frame_buffer);
            }
            else
            {
                ESP_LOGW(TAG, "Camera frame size mismatch: %dx%d, format: %d",
                         pic->width, pic->height, pic->format);
            }

            esp_camera_fb_return(pic);
        } else {
            ESP_LOGE(TAG, "Camera capture failed");
        }

        // 控制帧率
        vTaskDelay(pdMS_TO_TICKS(50)); // 约20fps
    }
}

