# SparkFun SSD168x Library

![SparkFun RockBLOCK 9604 Shield](./img/SparkFun_RockBLOCK_9604_Shield.png "SparkFun RockBLOCK 9604 Shield with GoodDisplay GDEM0097T61 0.97 inch e-paper display")

A library to support SSD1680/1 e-paper displays on SPI. We wrote it to support the GDEM0097T61 e-Paper display on the new SparkFun RockBLOCK 9604 Shield, but you can adapt the examples to run on any platform.

This library is based heavily on the [SparkFun Qwiic OLED Arduino Library](https://github.com/sparkfun/SparkFun_Qwiic_OLED_Arduino_Library) and includes the same fonts and graphics support.

It has been tested on the GoodDisplay GDEY0154D67 1.54&quot; (200 x 200) and GDEM0097T61 0.97&quot; (184 x 88) e-paper displays.

![GitHub License](https://img.shields.io/github/license/sparkfun/SparkFun_SSD168x_Library)
![Release](https://img.shields.io/github/v/release/sparkfun/SparkFun_SSD168x_Library)
![Release Date](https://img.shields.io/github/release-date/sparkfun/SparkFun_SSD168x_Library)
![Documentation - build](https://img.shields.io/github/actions/workflow/status/sparkfun/SparkFun_SSD168x_Library/build-deploy-ghpages.yml?label=doc%20build)
[![Compile Test](https://github.com/sparkfun/SparkFun_SSD168x_Library/actions/workflows/compile-sketch.yml/badge.svg)](https://github.com/sparkfun/SparkFun_SSD168x_Library/actions/workflows/compile-sketch.yml)
[![IDF Compile Test](https://github.com/sparkfun/SparkFun_SSD168x_Library/actions/workflows/compile-idf-example.yml/badge.svg)](https://github.com/sparkfun/SparkFun_SSD168x_Library/actions/workflows/compile-idf-example.yml)
![GitHub issues](https://img.shields.io/github/issues/sparkfun/SparkFun_SSD168x_Library)

Repository Contents
-------------------

* **/examples** - Examples for the Arduino IDE
* **/idf_examples** - Examples for the Espressif ESP-IDF (ESP32)
* **/src** - Source code
* **/docs** - Doxygen configuration for the [library documentation](https://docs.sparkfun.com/SparkFun_SSD168x_Library)

Documentation
--------------

* **[GitHub Repo](https://github.com/sparkfun/SparkFun_SSD168x_Library)** - the library source code and examples
* **[Library Documentation](https://docs.sparkfun.com/SparkFun_SSD168x_Library)** - the Doxygen documentation for the library, generated automatically on each release
* **[Hookup Guide](http://docs.sparkfun.com/TODO/)** - TODO: Update URL and description

Arduino Library and ESP-IDF Component
-------------------------------------

This library builds both as an Arduino Library and as an Espressif ESP-IDF Component:

* **Arduino**: install it with the Arduino Library Manager (search for "SparkFun SSD168x"). The examples are in [/examples](./examples)
* **ESP-IDF**: add it to your project with `idf.py add-dependency "sparkfun/sparkfun_ssd168x_library"`. The examples are in [/idf_examples](./idf_examples). ESP-IDF v5.3 or later is required

The platform is selected automatically (see [src/sfe_platform.h](./src/sfe_platform.h)). The SPI hardware interface is abstracted into
`sfe_platform.h` and `sfe_bus.h` / `sfe_bus.cpp` (Arduino) / `sfe_bus_esp_idf.h` / `sfe_bus_esp_idf.cpp` (ESP-IDF), using the same approach as the
[SparkFun u-blox GNSS v4 Library](https://github.com/sparkfun/SparkFun_u-blox_GNSS_v4). Debug messages are printed by the same `sfe_debug` class.

Connecting the Display
----------------------

The SSD1680/1 uses a 4-wire SPI interface plus two GPIO pins:

| Display pin | Function | RockBLOCK 9604 Shield (ESP32-S3) |
| :--- | :--- | :--- |
| SCL / SCLK | SPI clock | GPIO12 |
| SDA / SDIN | SPI data in (PICO / MOSI) | GPIO11 |
| CS# | Chip select - active low | GPIO15 |
| D/C# | Data / command select | GPIO7 |
| RES# | Reset - active low. Optional: pass -1 if not connected and a software reset is used instead | GPIO6 |
| BUSY | High while the display is busy | GPIO5 |

The display does not use POCI (MISO). The SPI bus can be shared with other devices (e.g. a microSD card) - make sure their chip selects are high (deselected).
On the RockBLOCK 9604 Shield, VP_EN (GPIO4) must be driven high to power the display.

The pins are passed to `begin()`. Arduino:

```cpp
#include <SparkFun_SSD168x_Library.h>

SSD1680SPI184x88Rotated myDisplay;

void setup()
{
    SPI.begin(12, 13, 11); // SCLK, POCI, PICO (ESP32)
    myDisplay.begin(15, 7, 6, 5); // CS#, D/C#, RES#, BUSY. Optionally: SPI port, SPI clock speed (default 4MHz)
    myDisplay.text(10, 10, "Hello World!");
    myDisplay.display();
}
```

ESP-IDF:

```cpp
#include "SparkFun_SSD168x_Library.h"

static SSD1680SPI184x88Rotated myDisplay;

extern "C" void app_main(void)
{
    spi_bus_config_t busConfig = {};
    busConfig.sclk_io_num = 12;
    busConfig.mosi_io_num = 11;
    busConfig.miso_io_num = 13;
    busConfig.quadwp_io_num = -1;
    busConfig.quadhd_io_num = -1;
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &busConfig, SPI_DMA_CH_AUTO));

    // CS#, D/C#, RES#, BUSY. Optionally: SPI clock speed (default CONFIG_SFE_SSD168X_SPI_CLOCK_HZ = 4MHz)
    myDisplay.begin(SPI2_HOST, GPIO_NUM_15, GPIO_NUM_7, GPIO_NUM_6, GPIO_NUM_5);
    myDisplay.text(10, 10, "Hello World!");
    myDisplay.display();
}
```

The supported displays are:

| Class | Display |
| :--- | :--- |
| `SSD1680SPI184x88` | 184 x 88 SSD1680 (e.g. GoodDisplay GDEM0097T61) - portrait (88 pixels wide, 184 high) |
| `SSD1680SPI184x88Rotated` | 184 x 88 SSD1680 (e.g. GoodDisplay GDEM0097T61) - landscape (184 pixels wide, 88 high) |
| `SSD1681SPI200x200` | 200 x 200 SSD1681 (e.g. GoodDisplay GDEY0154D67) |
| `SSD1680SPICustom` | Custom size SSD1680 - call `setDisplayWidth()` etc. before `begin()` |
| `SSD1681SPICustom` | Custom size SSD1681 - call `setDisplayWidth()` etc. before `begin()` |

Call `enableDebugging()` to see debug messages on `Serial` (Arduino) or the console (ESP-IDF).

Testing the examples on GoodDisplay's Dev Kit
---------------------------------------------

The examples in this library were written for and have been tested on the SparkFun RockBLOCK 9604 Shield, which uses the GoodDisplay 0.97&quot; 184x88 pixel GDEM0097T61 e-Paper display.

The examples have also been tested on GoodDisplay's [ESP32-L Dev Kit](https://www.good-display.com/product/338.html), with their DESPI-C02 24-pin FPC adapter (for the 1.54&quot; [GDEY0154D67](https://www.good-display.com/product/388.html)) and the extra CV0097 24-pin to 18-pin FPC adapter (for the 0.97&quot; [GDEM0097T61](https://www.good-display.com/product/486.html)). Together, these three parts are called the [ESP32-L(C097) evaluation kit](https://buy-lcd.com/products/097-inch-e-paper-display-development-kit-esp32-epaper-board-esp32-lc097).

Examples 01 and 05 need the 1.54&quot; 200x200 GDEY0154D67 display. The other examples need the 0.97&quot; 184x88 GDEM0097T61 display.

The ESP32-L Dev Kit uses a classic ESP32 (not an ESP32-S3) and the e-Paper display is connected to different pins:

| Signal | RockBLOCK 9604 Shield (ESP32-S3) | ESP32-L Dev Kit (ESP32) |
| :--- | :--- | :--- |
| **SCLK** | GPIO12 | GPIO18 |
| **SDIN** (PICO / MOSI) | GPIO11 | GPIO23 |
| **CS#** | GPIO15 | GPIO27 |
| **D/C#** | GPIO7 | GPIO14 |
| **RES#** | GPIO6 | GPIO12 |
| **BUSY** | GPIO5 | GPIO13 |
| POCI (MISO) | GPIO13 (microSD) | Not used |
| VP_EN (peripheral power) | GPIO4 | Not used |
| microSD CS# | GPIO10 | Not used |
| External SPI CS#1 / CS#2 | GPIO1 / GPIO2 | Not used |

The Dev Kit pins are the ones used in GoodDisplay's own ESP32 example code. Their code uses the Arduino analog pin names A14 (BUSY), A15 (RES), A16 (D/C) and A17 (CS): on the ESP32 these are GPIO13, GPIO12, GPIO14 and GPIO27.

**Important:** do not run the examples on the Dev Kit with the Shield pins. On the classic ESP32, GPIO6 - GPIO11 are connected to the SPI flash and GPIO1 is the UART0 (Serial) TX pin. The examples drive GPIO10, GPIO11 and GPIO1 on the Shield. On the Dev Kit, this would crash the ESP32 or stop the Serial output.

### Arduino IDE

Each Arduino example has a board switch near the top of the file:

1. Open the example in the Arduino IDE
2. Find the "Adjust these values according to your configuration" section and uncomment the `ESP32_L_DEV_KIT` define:

    ```cpp
    // Uncomment the next line to run the example on the GoodDisplay ESP32-L Dev Kit (ESP32)
    #define ESP32_L_DEV_KIT
    ```

    This selects the Dev Kit pins. The Shield-only pins (VP_EN, the microSD and external SPI chip selects) are set to -1 and are not used. POCI (MISO) is -1 too: the display does not need it
3. Select the **ESP32 Dev Module** board (Tools \ Board \ esp32). (For the Shield, select **ESP32S3 Dev Module**)
4. Select the Dev Kit's COM port, upload the example, and open the Serial Monitor at 115200 baud. You should see "Running SSD168x example on GoodDisplay ESP32-L Dev Kit"

The example passes the pins to the library in the normal way:

```cpp
    SPI.begin(pin_SCLK, pin_POCI, pin_PICO); // 18, -1, 23
    myDevice.begin(pin_CS, pin_DC, pin_RES, pin_BUSY); // 27, 14, 12, 13
```

### ESP-IDF

In the ESP-IDF examples, the board is selected automatically by the target. Each example has an "Example Configuration" menu with a "Board" choice (`CONFIG_ROCKBLOCK_9604_SHIELD` or `CONFIG_ESP32_L_DEV_KIT`) which sets the default pins:

* `idf.py set-target esp32` selects the **GoodDisplay ESP32-L Dev Kit** (and the settings in `sdkconfig.defaults.esp32`)
* `idf.py set-target esp32s3` selects the **SparkFun RockBLOCK 9604 Shield** (and the settings in `sdkconfig.defaults.esp32s3`: 16MB flash, USB Serial/JTAG console)

To run an example on the Dev Kit:

```
cd idf_examples/Example03_BasicUsage_184x88_Rotated
idf.py set-target esp32
idf.py menuconfig    # Optional: "Example Configuration" shows the board and pins
idf.py -p PORT flash monitor
```

`idf.py set-target` deletes `sdkconfig`, so the pins are always set correctly when you change target. If you change the board or pins in menuconfig instead, delete `sdkconfig` (or run `idf.py set-target` again) when you change board, so that the pin defaults are updated.

The example passes the pins to the library in the normal way:

```cpp
    // CS#, D/C#, RES#, BUSY - 27, 14, 12, 13 on the Dev Kit
    myDevice.begin(SPI2_HOST, (gpio_num_t)pin_CS, (gpio_num_t)pin_DC, (gpio_num_t)pin_RES, (gpio_num_t)pin_BUSY);
```

Notes about SSD1680/1 Ping-Pong Mode
------------------------------------

The GoodDisplay GDEY0154D67 (1.54&quot;, 200 x 200, SSD1681) and GDEM0097T61 (0.97&quot;, 184 x 88, SSD1680) e-paper displays both use SSD168x Ping-Pong Mode.
If you read the OTP Display Option using command 0x2D, you will find that Bit 6 of Byte F is set, indicating ping-pong mode is enabled.
We believe this is to allow the displays to better support partial updates. The RAM banks are used to identify which pixels have changed from the previous update.
It causes some interesting issues...

If you 'clear' the display - (e.g.) set all the pixels white and do a full update - the white pixels are written to both BW RAM and RED RAM.
GoodDisplay calls this the "BaseMap" or "background color function".
If you then set a group of pixels black and do a _partial_ update, those pixels are displayed - as you would expect.
If you then set a different group of pixels black and do a second _partial_ update, only the second group are displayed - the first group are hidden.
If you then set a third group of pixels black and do a third _partial_ update, the first and third groups are displayed - the second group is hidden.
We believe this is caused by the SSD168x "ping-pong" mode - automatic switching of the RAM banks.

One solution is to re-write all pixels before each partial update. But that takes time.

This library is based on the concept of "dirty pixels". Only pixel areas which have actually changed are written to the display. This means the ping-pong mode can cause some interesting 'blinking' or 'flashing' effects.

Our solution is to record which group(s) of pixels have changed for the current partial update, and to record which group(s) were changed during the previous partial update. The "dirty pixel" areas are expanded to cover both the latest changes (```m_pageState```) and the previous changes (```m_pagePrevious```).

License Information
-------------------

This product is _**open source**_! 

Please review the LICENSE.md file for license information. 

If you have any questions or concerns on licensing, please contact technical support on our [SparkFun forums](https://community.sparkfun.com/c/components-and-accessories/displays/29).

Distributed as-is; no warranty is given.

- Your friends at SparkFun.

_<COLLABORATION CREDIT>_
