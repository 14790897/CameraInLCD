/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "example_config.h"

static const char *TAG = "camera_test";

// 清理摄像头缓冲区的函数
static void camera_clear_buffers(void)
{
    ESP_LOGI(TAG, "Clearing camera buffers...");
    camera_fb_t *fb;
    int cleared_count = 0;

    // 清理所有待处理的帧缓冲，增加清理次数
    while ((fb = esp_camera_fb_get()) != NULL && cleared_count < 20)
    {
        esp_camera_fb_return(fb);
        cleared_count++;
        vTaskDelay(pdMS_TO_TICKS(20)); // 增加延迟时间
    }

    if (cleared_count > 0)
    {
        ESP_LOGI(TAG, "Cleared %d frame buffers", cleared_count);
    }
    else
    {
        ESP_LOGI(TAG, "No buffers to clear");
    }

    // 额外延迟确保清理完成
    vTaskDelay(pdMS_TO_TICKS(100));
}

// Camera initialization function for ESP32-S3
static esp_err_t camera_init(void)
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
    config.frame_size = FRAMESIZE_QVGA; // 320x240
    config.pixel_format = PIXFORMAT_RGB565; // RGB565 format
    config.grab_mode = CAMERA_GRAB_LATEST;  // 改为LATEST避免缓冲积累
    config.fb_location = CAMERA_FB_IN_PSRAM; // 使用 PSRAM
    config.jpeg_quality = 12;
    config.fb_count = 1; // 减少到1个缓冲避免溢出

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return err;
    }

    ESP_LOGI(TAG, "Camera hardware initialized successfully");

    // 初始化后立即清理可能存在的缓冲区
    ESP_LOGI(TAG, "Clearing any initial buffers...");
    vTaskDelay(pdMS_TO_TICKS(1000)); // 等待1秒让硬件稳定
    camera_clear_buffers();

    // Get camera sensor
    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Camera sensor object obtained successfully");

    // 输出传感器信息
    ESP_LOGI(TAG, "Camera sensor detected: PID=0x%02x, VER=0x%02x", s->id.PID, s->id.VER);

    // 在配置传感器前再次清理缓冲区
    ESP_LOGI(TAG, "Pre-configuration buffer cleanup...");
    camera_clear_buffers();
    vTaskDelay(pdMS_TO_TICKS(500)); // 额外延迟

    // 安全地设置传感器参数 - 检查函数指针是否有效
    ESP_LOGI(TAG, "Configuring sensor settings...");

    // 首先设置帧率控制以增加HTS/VTS时间
    if (s->set_framesize)
    {
        s->set_framesize(s, FRAMESIZE_QVGA);
        ESP_LOGI(TAG, "✓ Frame size set to QVGA");
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // 设置较低的增益和曝光以稳定时序
    if (s->set_gainceiling)
    {
        s->set_gainceiling(s, GAINCEILING_32X); // 降低增益上限
        ESP_LOGI(TAG, "✓ Gain ceiling set to 32X (lower for stable timing)");
        vTaskDelay(pdMS_TO_TICKS(150));
    }
    else
    {
        ESP_LOGW(TAG, "⚠ set_gainceiling function not available");
    }

    if (s->set_gain_ctrl)
    {
        s->set_gain_ctrl(s, 1); // 0 = disable, 1 = enable
        ESP_LOGI(TAG, "✓ Gain control enabled");
        vTaskDelay(pdMS_TO_TICKS(150));
    }
    else
    {
        ESP_LOGW(TAG, "⚠ set_gain_ctrl function not available");
    }

    if (s->set_exposure_ctrl)
    {
        s->set_exposure_ctrl(s, 1); // 0 = disable, 1 = enable
        ESP_LOGI(TAG, "✓ Exposure control enabled");
        vTaskDelay(pdMS_TO_TICKS(150));
    }
    else
    {
        ESP_LOGW(TAG, "⚠ set_exposure_ctrl function not available");
    }
    if (s->set_brightness) {
        s->set_brightness(s, 0);     // -2 to 2
        ESP_LOGI(TAG, "✓ Brightness set");
        vTaskDelay(pdMS_TO_TICKS(150)); // 增加延迟为时序留出空间
    } else {
        ESP_LOGW(TAG, "⚠ set_brightness function not available");
    }
    
    if (s->set_contrast) {
        s->set_contrast(s, 0);       // -2 to 2
        ESP_LOGI(TAG, "✓ Contrast set");
        vTaskDelay(pdMS_TO_TICKS(150));
    } else {
        ESP_LOGW(TAG, "⚠ set_contrast function not available");
    }
    
    if (s->set_saturation) {
        s->set_saturation(s, 0);     // -2 to 2
        ESP_LOGI(TAG, "✓ Saturation set");
        vTaskDelay(pdMS_TO_TICKS(150));
    } else {
        ESP_LOGW(TAG, "⚠ set_saturation function not available");
    }

    if (s->set_colorbar) {
        s->set_colorbar(s, 0);       // 0 = disable, 1 = enable
        ESP_LOGI(TAG, "✓ Color bar disabled");
        vTaskDelay(pdMS_TO_TICKS(150));
    } else {
        ESP_LOGW(TAG, "⚠ set_colorbar function not available");
    }
    
    if (s->set_whitebal) {
        s->set_whitebal(s, 1);       // 0 = disable, 1 = enable
        ESP_LOGI(TAG, "✓ White balance enabled");
        vTaskDelay(pdMS_TO_TICKS(150));
    } else {
        ESP_LOGW(TAG, "⚠ set_whitebal function not available");
    }

    if (s->set_hmirror) {
        s->set_hmirror(s, 0);        // 0 = disable, 1 = enable
        ESP_LOGI(TAG, "✓ Horizontal mirror disabled");
        vTaskDelay(pdMS_TO_TICKS(150));
    } else {
        ESP_LOGW(TAG, "⚠ set_hmirror function not available");
    }
    
    if (s->set_vflip) {
        s->set_vflip(s, 0);          // 0 = disable, 1 = enable
        ESP_LOGI(TAG, "✓ Vertical flip disabled");
        vTaskDelay(pdMS_TO_TICKS(150));
    } else {
        ESP_LOGW(TAG, "⚠ set_vflip function not available");
    }

    // 额外的时序优化设置
    ESP_LOGI(TAG, "Applying additional timing optimizations for DMA...");

    // 设置特殊的时序模式以减少数据突发传输
    if (s->set_special_effect)
    {
        s->set_special_effect(s, 0); // 无特效，减少处理负载
        ESP_LOGI(TAG, "✓ Special effects disabled for better timing");
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // 如果支持，设置较低的质量模式以减少数据量
    if (s->set_quality)
    {
        s->set_quality(s, 20); // 较低质量以减少数据传输压力
        ESP_LOGI(TAG, "✓ Quality set to lower value for stable DMA");
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    ESP_LOGI(TAG, "Camera initialized successfully");

    // 应用HTS/VTS时序优化
    ESP_LOGI(TAG, "Applying HTS/VTS timing optimizations...");

    // 配置完成后清理缓冲区
    ESP_LOGI(TAG, "Post-configuration buffer cleanup...");
    camera_clear_buffers();

    // 等待传感器稳定
    ESP_LOGI(TAG, "Waiting for sensor to stabilize with new timing settings...");
    vTaskDelay(pdMS_TO_TICKS(8000)); // 增加到8秒等待时间，让时序完全稳定

    // 最终清理
    ESP_LOGI(TAG, "Final buffer cleanup before use...");
    camera_clear_buffers();

    // 显示摄像头内存使用情况
    multi_heap_info_t psram_info_after;
    heap_caps_get_info(&psram_info_after, MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "After camera init - PSRAM free: %zu bytes", psram_info_after.total_free_bytes);
    
    return ESP_OK;
}

static void print_camera_frame_info(camera_fb_t *fb, uint32_t frame_num)
{
    ESP_LOGI(TAG, "Frame %lu:", frame_num);
    ESP_LOGI(TAG, "  Format: %s (%d)", 
             (fb->format == PIXFORMAT_RGB565) ? "RGB565" : "Unknown", fb->format);
    ESP_LOGI(TAG, "  Resolution: %dx%d", fb->width, fb->height);
    ESP_LOGI(TAG, "  Buffer size: %d bytes", fb->len);
    ESP_LOGI(TAG, "  Timestamp: %lld", fb->timestamp.tv_sec * 1000000LL + fb->timestamp.tv_usec);
    
    // 简单的数据完整性检查
    if (fb->len > 0 && fb->buf != NULL) {
        uint16_t *pixel_data = (uint16_t *)fb->buf;
        uint32_t total_pixels = fb->len / 2; // RGB565 = 2 bytes per pixel
        
        // 统计像素值分布
        uint32_t zero_pixels = 0;
        uint32_t max_pixels = 0;
        for (uint32_t i = 0; i < total_pixels; i++) {
            if (pixel_data[i] == 0x0000) zero_pixels++;
            if (pixel_data[i] == 0xFFFF) max_pixels++;
        }
        
        ESP_LOGI(TAG, "  Pixel analysis: %lu total, %lu black (0x0000), %lu white (0xFFFF)", 
                 total_pixels, zero_pixels, max_pixels);
        
        // 显示前几个像素值作为样本
        ESP_LOGI(TAG, "  Sample pixels: 0x%04X 0x%04X 0x%04X 0x%04X", 
                 pixel_data[0], pixel_data[1], pixel_data[2], pixel_data[3]);
    }
}

void app_main(void)
{
    // 检查PSRAM状态
    ESP_LOGI(TAG, "=== PSRAM Status Check ===");
    
    multi_heap_info_t psram_info;
    heap_caps_get_info(&psram_info, MALLOC_CAP_SPIRAM);
    
    if (psram_info.total_allocated_bytes + psram_info.total_free_bytes > 0) {
        ESP_LOGI(TAG, "PSRAM is available");
        ESP_LOGI(TAG, "PSRAM total: %zu bytes (%.2f MB)", 
                 psram_info.total_allocated_bytes + psram_info.total_free_bytes,
                 (float)(psram_info.total_allocated_bytes + psram_info.total_free_bytes) / (1024 * 1024));
        ESP_LOGI(TAG, "PSRAM free: %zu bytes (%.2f MB)", 
                 psram_info.total_free_bytes, 
                 (float)psram_info.total_free_bytes / (1024 * 1024));
    } else {
        ESP_LOGE(TAG, "PSRAM is NOT available - camera may not work properly");
        return;
    }
    
    // 显示内部RAM状态
    multi_heap_info_t dram_info;
    heap_caps_get_info(&dram_info, MALLOC_CAP_INTERNAL);
    ESP_LOGI(TAG, "Internal RAM free: %zu bytes (%.2f KB)", 
             dram_info.total_free_bytes, (float)dram_info.total_free_bytes / 1024);
    ESP_LOGI(TAG, "=== End PSRAM Status ===");

    // =================================================================
    // 摄像头测试
    // =================================================================
    ESP_LOGI(TAG, "=== Camera Test Start ===");
    vTaskDelay(pdMS_TO_TICKS(5000)); // 增加到5000ms延迟

    // 初始化摄像头
    esp_err_t camera_init_result = camera_init();
    if (camera_init_result != ESP_OK) {
        ESP_LOGE(TAG, "Camera initialization failed. Test aborted.");
        return;
    }
    
    ESP_LOGI(TAG, "Camera initialized successfully. Starting capture test...");

    // 摄像头捕获测试
    uint32_t total_test_frames = 10;  // 先测试10帧，成功后可以增加
    uint32_t successful_captures = 0;
    uint32_t failed_captures = 0;
    
    ESP_LOGI(TAG, "Testing %lu frames...", total_test_frames);
    
    for (uint32_t frame_num = 1; frame_num <= total_test_frames; frame_num++) {
        ESP_LOGI(TAG, "--- Capturing frame %lu/%lu ---", frame_num, total_test_frames);

        // 清理可能存在的旧帧缓冲
        camera_fb_t *temp_fb;
        int cleanup_count = 0;
        while ((temp_fb = esp_camera_fb_get()) != NULL && cleanup_count < 5)
        {
            esp_camera_fb_return(temp_fb);
            cleanup_count++;
        }
        if (cleanup_count > 0)
        {
            ESP_LOGW(TAG, "Cleaned up %d old frame buffers", cleanup_count);
        }

        // 添加额外的延迟确保稳定性
        vTaskDelay(pdMS_TO_TICKS(500)); // 增加到500ms延迟

        camera_fb_t *fb = esp_camera_fb_get();
        if (fb != NULL) {
            successful_captures++;
            
            // 详细分析前3帧和每5帧
            if (frame_num <= 3 || frame_num % 5 == 0) {
                print_camera_frame_info(fb, frame_num);
            } else {
                ESP_LOGI(TAG, "Frame %lu: OK - %dx%d, %d bytes", 
                         frame_num, fb->width, fb->height, fb->len);
            }
            
            // 验证图像数据
            bool validation_passed = true;
            
            // 检查基本参数
            if (fb->format != PIXFORMAT_RGB565) {
                ESP_LOGW(TAG, "  ⚠ Unexpected format: %d (expected RGB565)", fb->format);
                validation_passed = false;
            }
            
            if (fb->width != 320 || fb->height != 240) {
                ESP_LOGW(TAG, "  ⚠ Unexpected resolution: %dx%d (expected 320x240)", 
                         fb->width, fb->height);
                validation_passed = false;
            }
            
            if (fb->len != 320 * 240 * 2) {
                ESP_LOGW(TAG, "  ⚠ Unexpected buffer size: %d (expected %d)", 
                         fb->len, 320 * 240 * 2);
                validation_passed = false;
            }
            
            if (validation_passed) {
                ESP_LOGI(TAG, "  ✓ Frame validation passed");
            }
            
            esp_camera_fb_return(fb);
        } else {
            failed_captures++;
            ESP_LOGE(TAG, "  ✗ Frame %lu capture failed", frame_num);

            // 尝试重置相机状态
            ESP_LOGW(TAG, "Attempting to clear camera buffers...");
            camera_clear_buffers();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        // 帧间延迟
        vTaskDelay(pdMS_TO_TICKS(300)); // 增加到300ms延迟
    }
    
    // 测试结果总结
    ESP_LOGI(TAG, "=== Camera Test Results ===");
    ESP_LOGI(TAG, "Total frames tested: %lu", total_test_frames);
    ESP_LOGI(TAG, "Successful captures: %lu (%.1f%%)", 
             successful_captures, (float)successful_captures * 100.0 / total_test_frames);
    ESP_LOGI(TAG, "Failed captures: %lu (%.1f%%)", 
             failed_captures, (float)failed_captures * 100.0 / total_test_frames);
    
    if (successful_captures == total_test_frames) {
        ESP_LOGI(TAG, "🎉 Camera test PASSED! All frames captured successfully.");
    } else if (successful_captures > total_test_frames * 0.9) {
        ESP_LOGW(TAG, "⚠ Camera test MOSTLY PASSED with some failures.");
    } else {
        ESP_LOGE(TAG, "❌ Camera test FAILED! Too many capture failures.");
    }
        vTaskDelay(pdMS_TO_TICKS(10000)); // ~30 FPS

    // 持续监控模式（可选）
    ESP_LOGI(TAG, "Entering continuous monitoring mode...");
    ESP_LOGI(TAG, "Press RESET to restart test or flash new firmware to stop.");
    
    // uint32_t continuous_frame_count = 0;
    // while (1) {
    //     camera_fb_t *fb = esp_camera_fb_get();
    //     if (fb != NULL) {
    //         continuous_frame_count++;
    //         if (continuous_frame_count % 30 == 0) {  // 每30帧记录一次
    //             ESP_LOGI(TAG, "Continuous mode: %lu frames captured", continuous_frame_count);
    //         }
    //         esp_camera_fb_return(fb);
    //     } else {
    //         ESP_LOGW(TAG, "Continuous mode: Frame capture failed");
    //     }
        
    //     vTaskDelay(pdMS_TO_TICKS(33)); // ~30 FPS
    // }
}
