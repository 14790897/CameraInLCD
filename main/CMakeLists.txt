# 重要: 需要添加依赖: idf.py add-dependency "teriyakigod/esp_lcd_st7735^0.0.1"
# 选择要编译的源文件
# 取消注释以下行之一来选择要测试的模块：

# 1. 摄像头测试（推荐先测试）
# idf_component_register(SRCS "camera_test.c"
#                        INCLUDE_DIRS "."
#                        REQUIRES esp_mm esp_driver_spi esp_lcd esp32-camera driver esp_lcd_ili9341 log
#                        )


# 2. LCD st7735
# idf_component_register(SRCS "st7735s_official_test.c"
#                        INCLUDE_DIRS "."
#                        REQUIRES esp_mm esp_driver_spi esp_lcd esp32-camera driver log esp_lcd_st7735
#                        )

# 3. 原始组合测试
idf_component_register(SRCS "dvp_lcd_main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_mm esp_driver_spi esp_lcd esp32-camera driver log  esp_lcd_st7735
                       )


