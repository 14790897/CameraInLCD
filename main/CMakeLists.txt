# 选择要编译的源文件
# 取消注释以下行之一来选择要测试的模块：

# 1. 摄像头测试（推荐先测试）
# idf_component_register(SRCS "spi_host_check.c" "i2c_voltage_test.c" "lcd_hardware_diag.c" "camera_test.c"
#                        INCLUDE_DIRS "."
#                        REQUIRES esp_mm esp_driver_spi esp_lcd esp32-camera driver esp_lcd_ili9341 log
#                        )

# 2. LCD基础测试
# idf_component_register(SRCS "lcd_test.c"
#                        INCLUDE_DIRS "."
#                        REQUIRES esp_mm esp_driver_spi esp_lcd esp32-camera driver esp_lcd_ili9341 log
#                        )

# 3. LCD诊断工具（推荐用于排查LCD无显示问题）
# idf_component_register(SRCS "lcd_diagnostic.c"
#                        INCLUDE_DIRS "."
#                        REQUIRES esp_mm esp_driver_spi esp_lcd esp32-camera driver esp_lcd_ili9341 log
#                        )

# 4. LCD无显示修复工具（针对MISO连接问题）- 现在使用纯色填充（无条纹）
# idf_component_register(SRCS "lcd_fix.c"
#                        INCLUDE_DIRS "."
#                        REQUIRES esp_mm esp_driver_spi esp_lcd esp32-camera driver esp_lcd_ili9341 log
#                        )

# 5. LCD st7735
# idf_component_register(SRCS "st7735s_official_test.c"
#                        INCLUDE_DIRS "."
#                        REQUIRES esp_mm esp_driver_spi esp_lcd esp32-camera driver log
#                        )

# 6. 原始组合测试
idf_component_register(SRCS "dvp_lcd_main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_mm esp_driver_spi esp_lcd esp32-camera driver log
                       )

