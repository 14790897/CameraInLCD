/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "sdkconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define EXAMPLE_RGB565_BITS_PER_PIXEL 16

// ILI9341 SPI LCD Configuration
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_SCLK 13
#define EXAMPLE_PIN_NUM_MOSI 14
#define EXAMPLE_PIN_NUM_MISO 17
#define EXAMPLE_PIN_NUM_LCD_DC 48
#define EXAMPLE_PIN_NUM_LCD_RST 42
#define EXAMPLE_PIN_NUM_LCD_CS 16
#define EXAMPLE_PIN_NUM_BK_LIGHT -1
// #define EXAMPLE_LCD_H_RES 320
// #define EXAMPLE_LCD_V_RES 240

// OV7670 Camera Sensor Configuration
#define EXAMPLE_ISP_DVP_CAM_PWDN_IO (12)
#define EXAMPLE_ISP_DVP_CAM_RESET_IO (11)
#define EXAMPLE_ISP_DVP_CAM_SCCB_SCL_IO (2)
#define EXAMPLE_ISP_DVP_CAM_SCCB_SDA_IO (1)

#define EXAMPLE_ISP_DVP_CAM_XCLK_FREQ_HZ (20000000)

#define EXAMPLE_ISP_DVP_CAM_DATA_WIDTH (8)
#define EXAMPLE_ISP_DVP_CAM_D0_IO (8)
#define EXAMPLE_ISP_DVP_CAM_D1_IO (9)
#define EXAMPLE_ISP_DVP_CAM_D2_IO (10)
#define EXAMPLE_ISP_DVP_CAM_D3_IO (4)
#define EXAMPLE_ISP_DVP_CAM_D4_IO (3)
#define EXAMPLE_ISP_DVP_CAM_D5_IO (45)
#define EXAMPLE_ISP_DVP_CAM_D6_IO (47)
#define EXAMPLE_ISP_DVP_CAM_D7_IO (46)
#define EXAMPLE_ISP_DVP_CAM_XCLK_IO (38)
#define EXAMPLE_ISP_DVP_CAM_PCLK_IO (21)
#define EXAMPLE_ISP_DVP_CAM_VSYNC_IO (39)
#define EXAMPLE_ISP_DVP_CAM_HSYNC_IO (40)// OV7670支持640 x 480 color Raw Bayer RGB Processed Bayer RGB
// YUV/YCbCr422 GRB422 RGB565/555

// #define EXAMPLE_CAM_FORMAT "DVP_8bit_20Minput_RGB565_320x240_30fps"

#ifdef __cplusplus
}
#endif
