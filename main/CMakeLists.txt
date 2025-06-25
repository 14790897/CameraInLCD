idf_component_register(SRCS "dvp_isp_dsi_main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_mm esp_driver_spi esp_lcd esp32-camera driver esp_lcd_ili9341
                       )
