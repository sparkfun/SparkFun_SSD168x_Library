# SparkFun SSD168x Library: ESP-IDF Examples

These are ESP-IDF versions of the Arduino examples in the [examples](https://github.com/sparkfun/SparkFun_SSD168x_Library/blob/main/examples) folder. Each one is a complete ESP-IDF project, with the application in `main/main.cpp`. The library is C++, so the examples are C++ too, with `extern "C" void app_main()`. Each example keeps the Arduino `setup()` / `loop()` structure: `app_main()` calls `setup()` once, then calls `loop()` forever.

The examples are written for the ESP32-S3 on the SparkFun RockBLOCK 9604 Shield, where the GoodDisplay GDEM0097T61 0.97" (184x88 pixels, SSD1680) e-Paper display is connected directly to the ESP32-S3 using SPI. They can also be run on the GoodDisplay ESP32-L Dev Kit (ESP32). Examples 01 and 05 need a 200x200 SSD1681 display (e.g. the GoodDisplay GDEY0154D67).

To build, flash and monitor an example:

```
cd idf_examples/Example03_BasicUsage_184x88_Rotated
idf.py set-target esp32s3    # SparkFun RockBLOCK 9604 Shield. Use esp32 for the GoodDisplay ESP32-L Dev Kit
idf.py menuconfig    # Optional: "Example Configuration" sets the GPIO pins
idf.py -p PORT flash monitor
```

Each example's `main/idf_component.yml` pulls in the library with `override_path: "../../../"`, so it builds against the library in this repository. (`override_path` is removed automatically when an example is downloaded from the ESP-IDF Component Registry.) Each example's `sdkconfig.defaults` sets `CONFIG_FREERTOS_HZ=1000` - recommended: it makes the library's short (1ms) delays true sleeps - and increases the main task stack size to 8192 bytes. `sdkconfig.defaults.esp32s3` adds the RockBLOCK 9604 Shield settings: 16MB flash and the native USB (USB Serial/JTAG) console. `sdkconfig.defaults.esp32` selects the ESP32-L Dev Kit.

**Important:** ESP-IDF uses the directory name as the component name. The component is called `sparkfun_ssd168x_library`, so the folder containing this repository must be called `sparkfun_ssd168x_library` (all lower case) when you build these examples. Otherwise the build fails with: `Failed to resolve component 'sparkfun__sparkfun_ssd168x_library' required by component 'main': unknown name`. If you cloned the repo or installed it with the Arduino Library Manager, the folder is called `SparkFun_SSD168x_Library`: clone it again into a folder with the lower-case name, or rename it. On Windows, renaming only the case takes two steps:

```
cd C:\Users\<you>\Documents\Arduino\libraries
Rename-Item SparkFun_SSD168x_Library sparkfun_ssd168x_library_tmp
Rename-Item sparkfun_ssd168x_library_tmp sparkfun_ssd168x_library
```

The Arduino IDE does not mind the lower-case name. This only affects building the examples from this repository: when you add the component from the Component Registry (`idf.py add-dependency`), the Component Manager names the folder for you.

The user creates the SPI bus (`spi_bus_initialize()`) and passes the SPI host and the CS#, D/C#, RES# and BUSY pins to `begin()`. The library adds the display to the bus as a device, so the bus can be shared with other devices (e.g. the microSD card). Alternatively, add the device yourself (with `spics_io_num = -1`, SPI mode 0) and pass the `spi_device_handle_t` to `begin()`.

The default SPI clock speed is 4MHz. Change it with `idf.py menuconfig` -> "SparkFun SSD168x Library", or pass it to `begin()`.

## Boards and Pins

The "Board" choice in `idf.py menuconfig` -> "Example Configuration" sets the default pins. It is selected automatically by the target: `esp32s3` selects the SparkFun RockBLOCK 9604 Shield (`CONFIG_ROCKBLOCK_9604_SHIELD`); `esp32` selects the GoodDisplay ESP32-L Dev Kit (`CONFIG_ESP32_L_DEV_KIT`). `idf.py set-target` deletes `sdkconfig`, so the pins are updated when you change target. If you change the board in menuconfig instead, delete `sdkconfig` so that the pin defaults are updated. See the [main README](../README.md) for more details about the Dev Kit.

| Signal | RockBLOCK 9604 Shield (esp32s3) | ESP32-L Dev Kit (esp32) | Notes |
| :--- | :--- | :--- | :--- |
| SPI SCLK | 12 | 18 | |
| SPI PICO (MOSI) | 11 | 23 | Connected to the e-Paper SDIN pin |
| SPI POCI (MISO) | 13 | -1 | Not used by the e-Paper display. Used by the microSD card on the Shield |
| e-Paper CS# | 15 | 27 | |
| e-Paper D/C# | 7 | 14 | |
| e-Paper RES# | 6 | 12 | -1 if not connected: a software reset is used instead |
| e-Paper BUSY | 5 | 13 | |
| VP_EN | 4 | -1 | Driven high to enable the 3.3V_P peripheral power for the e-Paper display |
| microSD CS# | 10 | -1 | Driven high (deselected) |
| External SPI CS#1 / CS#2 | 1 / 2 | -1 / -1 | Driven high (deselected) |

-1 means "not used".

## Examples

| Example | Summary |
| :--- | :--- |
| [Example01_BasicUsage_200x200](./Example01_BasicUsage_200x200) | Basic usage (200x200) |
| [Example02_BasicUsage_184x88](./Example02_BasicUsage_184x88) | Basic usage (184x88) |
| [Example03_BasicUsage_184x88_Rotated](./Example03_BasicUsage_184x88_Rotated) | Basic usage (184x88 rotated) |
| [Example04_184x88_Rotated_Clock](./Example04_184x88_Rotated_Clock) | 184x88 clock emulator using partial updates |
| [Example05_200x200_Clock](./Example05_200x200_Clock) | 200x200 clock emulator using partial updates |
| [Example06_184x88_Rotated_RTK](./Example06_184x88_Rotated_RTK) | RTK display emulator (184x88) using partial updates |
| [Example07_184x88_Rotated_Font10x20](./Example07_184x88_Rotated_Font10x20) | Font 10x20 (184x88 rotated) |
| [Example08_184x88_Rotated_Partial](./Example08_184x88_Rotated_Partial) | 184x88 testing partial updates |
| [Example09_184x88_Clock](./Example09_184x88_Clock) | 184x88 clock emulator using partial updates |
| [Example10_184x88_Rotated_Partial_Text_Erase](./Example10_184x88_Rotated_Partial_Text_Erase) | 184x88 testing partial updates - text erase and display bitmap |
