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
    config.xclk_freq_hz = 8000000;          // 降低到8MHz时钟频率以降低硬件刷新率
    config.frame_size = FRAMESIZE_QVGA;     // 320x240 for ST7735S
    config.pixel_format = PIXFORMAT_RGB565; // RGB565 format
    config.grab_mode = CAMERA_GRAB_LATEST;  // Changed to LATEST to avoid buffer buildup
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;                    // Reduced to 1 buffer to prevent overflow

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

    ESP_LOGI(TAG, "Camera sensor detected: PID=0x%02x, VER=0x%02x", s->id.PID, s->id.VER);

    // Enable and configure sensor settings for consistent RGB565 output
    ESP_LOGI(TAG, "Setting critical pixel format and frame size...");

    if (s->set_pixformat)
    {
        s->set_pixformat(s, PIXFORMAT_RGB565); // Ensure RGB565 format
        ESP_LOGI(TAG, "✓ Pixel format explicitly set to RGB565");
        vTaskDelay(pdMS_TO_TICKS(300));
    }

    if (s->set_framesize) {
        s->set_framesize(s, FRAMESIZE_QVGA); // Ensure 320x240
        ESP_LOGI(TAG, "✓ Frame size set to QQVGA (160x120)");
        vTaskDelay(pdMS_TO_TICKS(300));
    }

    // 禁用测试模式和颜色条 - 这是关键！
    if (s->set_colorbar)
    {
        s->set_colorbar(s, 0); // 确保禁用颜色条测试模式
        ESP_LOGI(TAG, "✓ Color bar test mode DISABLED");
        vTaskDelay(pdMS_TO_TICKS(300));
    }
    else
    {
        ESP_LOGW(TAG, "⚠ set_colorbar function not available");
    }

    // Configure sensor settings for stable operation
    if (s->set_brightness) {
        s->set_brightness(s, 0);     // -2 to 2
        ESP_LOGI(TAG, "✓ Brightness set");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    if (s->set_contrast) {
        s->set_contrast(s, 1); // 增加对比度
        ESP_LOGI(TAG, "✓ Contrast set to +1");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    if (s->set_saturation) {
        s->set_saturation(s, 0);     // -2 to 2
        ESP_LOGI(TAG, "✓ Saturation set");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    if (s->set_gainceiling) {
        s->set_gainceiling(s, GAINCEILING_16X); // Lower gain for stability
        ESP_LOGI(TAG, "✓ Gain ceiling set to 16X");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    if (s->set_whitebal) {
        s->set_whitebal(s, 1);       // 0 = disable, 1 = enable
        ESP_LOGI(TAG, "✓ White balance enabled");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    if (s->set_gain_ctrl) {
        s->set_gain_ctrl(s, 1);      // 0 = disable, 1 = enable
        ESP_LOGI(TAG, "✓ Gain control enabled");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    if (s->set_exposure_ctrl) {
        s->set_exposure_ctrl(s, 1);  // 0 = disable, 1 = enable
        ESP_LOGI(TAG, "✓ Exposure control enabled");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    if (s->set_hmirror) {
        s->set_hmirror(s, 0);        // 0 = disable, 1 = enable
        ESP_LOGI(TAG, "✓ Horizontal mirror disabled");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    if (s->set_vflip) {
        s->set_vflip(s, 0);          // 0 = disable, 1 = enable
        ESP_LOGI(TAG, "✓ Vertical flip disabled");
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // 关键的图像质量设置
    if (s->set_raw_gma)
    {
        s->set_raw_gma(s, 1); // 启用Gamma校正
        ESP_LOGI(TAG, "✓ Gamma correction enabled");
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    if (s->set_lenc)
    {
        s->set_lenc(s, 1); // 启用镜头校正
        ESP_LOGI(TAG, "✓ Lens correction enabled");
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    if (s->set_awb_gain)
    {
        s->set_awb_gain(s, 1); // 启用自动白平衡增益
        ESP_LOGI(TAG, "✓ Auto white balance gain enabled");
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    if (s->set_wb_mode)
    {
        s->set_wb_mode(s, 0); // 自动白平衡模式
        ESP_LOGI(TAG, "✓ White balance mode set to auto");
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // 最终确认像素格式
    ESP_LOGI(TAG, "Final confirmation of pixel format...");
    if (s->set_pixformat)
    {
        s->set_pixformat(s, PIXFORMAT_RGB565);
        ESP_LOGI(TAG, "✓ Pixel format RE-CONFIRMED as RGB565");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Wait for sensor to stabilize with new settings
    ESP_LOGI(TAG, "Waiting for sensor to stabilize...");
    vTaskDelay(pdMS_TO_TICKS(3000)); // 增加稳定时间到3秒

    ESP_LOGI(TAG, "Camera initialized successfully");
    return ESP_OK;
}
// ST7735S LCD initialization function
static esp_err_t init_st7735s_lcd(esp_lcd_panel_handle_t *panel_handle)
{

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
            // ESP_LOGI(TAG, "Camera frame: %dx%d, format: %d, size: %zu bytes",
            //          pic->width, pic->height, pic->format, pic->len);

            // Check for all possible valid configurations
            if ((pic->width == 160 && pic->height == 120 && pic->format == PIXFORMAT_RGB565) ||
                (pic->width == 128 && pic->height == 128 && pic->format == PIXFORMAT_RGB565) ||
                (pic->width == 320 && pic->height == 240 && pic->format == PIXFORMAT_RGB565))
            {
                uint16_t *src = (uint16_t *)pic->buf;
                uint16_t *dst = (uint16_t *)frame_buffer;

                if (pic->width == 160 && pic->height == 120) {
                    // Original code for 160x120 to 128x160 conversion
                    int src_start_x = (160 - 128) / 2; // Start from x=16

                    for (int dst_y = 0; dst_y < ST7735S_LCD_V_RES; dst_y++)
                    {
                        int src_y = (dst_y * 120) / ST7735S_LCD_V_RES; // Stretch mapping
                        for (int dst_x = 0; dst_x < ST7735S_LCD_H_RES; dst_x++)
                        {
                            int src_x = src_start_x + dst_x;
                            dst[dst_y * ST7735S_LCD_H_RES + dst_x] =
                                src[src_y * 160 + src_x];
                        }
                    }
                } else if (pic->width == 128 && pic->height == 128) {
                    // Handle 128x128 to 128x160 conversion - 修复拉伸算法
                    // 方案1: 居中显示，上下留黑边
                    int offset_y = (ST7735S_LCD_V_RES - 128) / 2; // 垂直偏移16像素

                    // 先清空整个目标缓冲区为黑色
                    memset(dst, 0, ST7735S_LCD_H_RES * ST7735S_LCD_V_RES * sizeof(uint16_t));

                    // 将128x128图像居中放置在128x160显示区域中
                    for (int src_y = 0; src_y < 128; src_y++)
                    {
                        int dst_y = src_y + offset_y;
                        if (dst_y >= 0 && dst_y < ST7735S_LCD_V_RES)
                        {
                            for (int src_x = 0; src_x < 128; src_x++)
                            {
                                dst[dst_y * ST7735S_LCD_H_RES + src_x] =
                                    src[src_y * 128 + src_x];
                            }
                        }
                    }

                    /* 备选方案2: 垂直拉伸填满屏幕（如果想要填满屏幕可以使用这个）
                    for (int dst_y = 0; dst_y < ST7735S_LCD_V_RES; dst_y++)
                    {
                        // 更精确的映射算法
                        int src_y = (dst_y * 128) / ST7735S_LCD_V_RES;
                        if (src_y >= 128) src_y = 127; // 边界检查

                        for (int dst_x = 0; dst_x < ST7735S_LCD_H_RES; dst_x++)
                        {
                            dst[dst_y * ST7735S_LCD_H_RES + dst_x] =
                                src[src_y * 128 + dst_x];
                        }
                    }
                    */
                }
                else if (pic->width == 320 && pic->height == 240)
                {
                    // Handle QVGA 320x240 to 128x160 conversion
                    // Method: Center crop and then scale down
                    // First crop 320x240 to get center 256x192 area (2:1.5 ratio similar to 128:96)
                    // Then scale down to 128x160 using sampling

                    int crop_width = 256;                       // Crop to center 256 pixels horizontally
                    int crop_height = 192;                      // Crop to center 192 pixels vertically
                    int crop_start_x = (320 - crop_width) / 2;  // Start at x=32
                    int crop_start_y = (240 - crop_height) / 2; // Start at y=24

                    for (int dst_y = 0; dst_y < ST7735S_LCD_V_RES; dst_y++)
                    {
                        // Map dst_y (0-159) to cropped source y (0-191)
                        int src_y = crop_start_y + (dst_y * crop_height) / ST7735S_LCD_V_RES;
                        if (src_y >= 240)
                            src_y = 239; // Boundary check

                        for (int dst_x = 0; dst_x < ST7735S_LCD_H_RES; dst_x++)
                        {
                            // Map dst_x (0-127) to cropped source x (0-255)
                            int src_x = crop_start_x + (dst_x * crop_width) / ST7735S_LCD_H_RES;
                            if (src_x >= 320)
                                src_x = 319; // Boundary check

                            dst[dst_y * ST7735S_LCD_H_RES + dst_x] =
                                src[src_y * 320 + src_x];
                        }
                    }
                }

                // Display to LCD
                esp_lcd_panel_draw_bitmap(panel_handle, 0, 0,
                                          ST7735S_LCD_H_RES, ST7735S_LCD_V_RES,
                                          frame_buffer);
            }
            else
            {
                ESP_LOGW(TAG, "Camera frame size/format mismatch: %dx%d, format: %d (expected 160x120, 128x128, or 320x240 with RGB565)",
                         pic->width, pic->height, pic->format);
            }

            esp_camera_fb_return(pic);
        } else {
            ESP_LOGE(TAG, "Camera capture failed");
        }

        // 控制帧率 - 由于降低了时钟频率，可以减少软件延迟
        vTaskDelay(pdMS_TO_TICKS(100)); // 恢复到10fps，因为硬件层面已经降速
    }
}

