ELF file SHA256: 076a91285

Rebooting...
���ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0xc (RTC_SW_CPU_RST),boot:0x8 (SPI_FAST_FLASH_BOOT)
Saved PC:0x40375b01
--- 0x40375b01: esp_restart_noos at C:/git-program/esp-idf/components/esp_system/port/soc/esp32s3/system_internal.c:162

SPIWP:0xee
mode:DIO, clock div:1
load:0x3fce2820,len:0x15d8
load:0x403c8700,len:0xd24
load:0x403cb700,len:0x2f84
entry 0x403c8924
I (29) boot: ESP-IDF v5.5-dev-3236-g465b159cd8 2nd stage bootloader
I (29) boot: compile time Jun 25 2025 14:41:51
I (29) boot: Multicore bootloader
I (31) boot: chip revision: v0.2
I (34) boot: efuse block revision: v1.3
I (37) boot.esp32s3: Boot SPI Speed : 80MHz
I (41) boot.esp32s3: SPI Mode       : DIO
I (45) boot.esp32s3: SPI Flash Size : 8MB
I (49) boot: Enabling RNG early entropy source...
I (53) boot: Partition Table:
I (56) boot: ## Label            Usage          Type ST Offset   Length
I (62) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (69) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (75) boot:  2 factory          factory app      00 00 00010000 00100000
I (82) boot: End of partition table
I (85) esp_image: segment 0: paddr=00010020 vaddr=3c020020 size=0b2f4h ( 45812) map
I (100) esp_image: segment 1: paddr=0001b31c vaddr=3fc93a00 size=02e7ch ( 11900) load
I (103) esp_image: segment 2: paddr=0001e1a0 vaddr=40374000 size=01e78h (  7800) load
I (109) esp_image: segment 3: paddr=00020020 vaddr=42000020 size=1c820h (116768) map
I (135) esp_image: segment 4: paddr=0003c848 vaddr=40375e78 size=0daa4h ( 55972) load
I (148) esp_image: segment 5: paddr=0004a2f4 vaddr=600fe000 size=0001ch (    28) load
I (154) boot: Loaded app from partition at offset 0x10000
I (155) boot: Disabling RNG early entropy source...
I (166) octal_psram: vendor id    : 0x0d (AP)
I (166) octal_psram: dev id       : 0x02 (generation 3)
I (166) octal_psram: density      : 0x03 (64 Mbit)
I (168) octal_psram: good-die     : 0x01 (Pass)
I (172) octal_psram: Latency      : 0x01 (Fixed)
I (177) octal_psram: VCC          : 0x01 (3V)
I (181) octal_psram: SRF          : 0x01 (Fast Refresh)
I (186) octal_psram: BurstType    : 0x01 (Hybrid Wrap)
I (191) octal_psram: BurstLen     : 0x01 (32 Byte)
I (195) octal_psram: Readlatency  : 0x02 (10 cycles@Fixed)
I (200) octal_psram: DriveStrength: 0x00 (1/1)
I (204) esp_psram: Found 8MB PSRAM device
I (208) esp_psram: Speed: 40MHz
I (211) cpu_start: Multicore app
I (945) esp_psram: SPI SRAM memory test OK
I (954) cpu_start: Pro cpu start user code
I (954) cpu_start: cpu freq: 160000000 Hz
I (954) app_init: Application information:
I (954) app_init: Project name:     dvp_isp_dsi
I (958) app_init: App version:      cd2c455-dirty
I (963) app_init: Compile time:     Jun 25 2025 14:41:30
I (968) app_init: ELF file SHA256:  076a91285...
I (972) app_init: ESP-IDF:          v5.5-dev-3236-g465b159cd8
I (978) efuse_init: Min chip rev:     v0.0
I (981) efuse_init: Max chip rev:     v0.99
I (985) efuse_init: Chip rev:         v0.2
I (989) heap_init: Initializing. RAM available for dynamic allocation:
I (996) heap_init: At 3FC97970 len 00051DA0 (327 KiB): RAM
I (1001) heap_init: At 3FCE9710 len 00005724 (21 KiB): RAM
I (1006) heap_init: At 3FCF0000 len 00008000 (32 KiB): DRAM
I (1011) heap_init: At 600FE01C len 00001FCC (7 KiB): RTCRAM
I (1017) esp_psram: Adding pool of 8192K of PSRAM memory to heap allocator
I (1024) spi_flash: detected chip: boya
I (1027) spi_flash: flash io: dio
I (1030) sleep_gpio: Configure to isolate all GPIO pins in sleep state
I (1036) sleep_gpio: Enable automatic switching of GPIO sleep configuration
I (1043) main_task: Started on CPU0
I (1053) esp_psram: Reserving pool of 32K of internal memory for DMA/internal allocations
I (1053) main_task: Calling app_main()
I (1053) dvp_camera_spi: === PSRAM Status Check ===
I (1063) dvp_camera_spi: PSRAM is available
I (1063) dvp_camera_spi: PSRAM total: 8386308 bytes (8.00 MB)
I (1073) dvp_camera_spi: PSRAM free: 8386308 bytes (8.00 MB)
I (1073) dvp_camera_spi: PSRAM used: 0 bytes (0.00 MB)
I (1083) dvp_camera_spi: Internal RAM free: 381363 bytes (372.42 KB)
I (1083) dvp_camera_spi: === End PSRAM Status ===
I (1093) dvp_camera_spi: LCD_H_RES: 320, LCD_V_RES: 240, bits per pixel: 16
I (1103) dvp_camera_spi: frame_buffer_size: 153600
I (1103) dvp_camera_spi: frame_buffer: 0x0
I (1103) dvp_camera_spi: === Camera Initialization ===
I (1113) s3 ll_cam: DMA Channel=0
I (1113) cam_hal: cam init ok
I (1113) sccb-ng: pin_sda 5 pin_scl 6
I (1123) sccb-ng: sccb_i2c_port=1
I (1173) camera: Detected camera at address=0x21
E (1173) i2c.master: I2C transaction unexpected nack detected
E (1173) i2c.master: s_i2c_synchronous_transaction(939): I2C transaction failed
E (1173) i2c.master: i2c_master_transmit_receive(1242): I2C transaction failed
E (1183) sccb-ng: SCCB_Read Failed addr:0x21, reg:0x0a, data:0xc0, ret:259
I (1183) ov7760: Mismatch PID=0xc0
E (1193) camera: Detected camera not supported.
E (1193) camera: Camera probe failed with error 0x106(ESP_ERR_NOT_SUPPORTED)
E (1203) gdma: gdma_disconnect(309): no peripheral is connected to the channel
E (1213) dvp_camera_spi: Camera init failed with error 0x106
ESP_ERROR_CHECK failed: esp_err_t 0x106 (ESP_ERR_NOT_SUPPORTED) at 0x42007a33
--- 0x42007a33: app_main at C:/git-program/Embedded/dvp_lcd/dvp_lcd/main/dvp_lcd_main.c:205 (discriminator 1)

file: "./main/dvp_lcd_main.c" line 205
func: app_main
expression: example_camera_init()

abort() was called at PC 0x4037a79b on core 0
--- 0x4037a79b: _esp_error_check_failed at C:/git-program/esp-idf/components/esp_system/esp_err.c:49



Backtrace: 0x40375bc1:0x3fc99cd0 0x4037a7a5:0x3fc99cf0 0x40381231:0x3fc99d10 0x4037a79b:0x3fc99d80 0x42007a33:0x3fc99db0 0x4201ba0c:0x3fc99e20 0x4037b1dd:0x3fc99e50
--- 0x40375bc1: panic_abort at C:/git-program/esp-idf/components/esp_system/panic.c:468
--- 0x4037a7a5: esp_system_abort at C:/git-program/esp-idf/components/esp_system/port/esp_system_chip.c:87
--- 0x40381231: abort at C:/git-program/esp-idf/components/newlib/src/abort.c:38
--- 0x4037a79b: _esp_error_check_failed at C:/git-program/esp-idf/components/esp_system/esp_err.c:49
--- 0x42007a33: app_main at C:/git-program/Embedded/dvp_lcd/dvp_lcd/main/dvp_lcd_main.c:205 (discriminator 1)
--- 0x4201ba0c: main_task at C:/git-program/esp-idf/components/freertos/app_startup.c:208
--- 0x4037b1dd: vPortTaskWrapper at C:/git-program/esp-idf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:139





ELF file SHA256: 076a91285

Rebooting...