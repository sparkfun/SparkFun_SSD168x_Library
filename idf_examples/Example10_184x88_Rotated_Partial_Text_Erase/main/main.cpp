// Example10 : 184x88 testing partial updates - text erase and display bitmap
//
// Written by P.C. @ SparkFun Electronics, April 2026
// SPI version: November 2026
//
// ESP-IDF version of the Arduino example Example10_184x88_Rotated_Partial_Text_Erase.ino
//
// This is a library to control SSD1680/1 e-Paper displays via SPI.
//
// This example is written for the ESP32-S3 on the SparkFun RockBLOCK 9604 Shield, where the
// GoodDisplay GDEM0097T61 0.97" (184x88 pixels, SSD1680) e-Paper display is connected
// directly to the ESP32-S3 using SPI. The SPI bus is shared with the microSD card.
// It can also run on the GoodDisplay ESP32-L Dev Kit: use idf.py set-target esp32
// Set the GPIO pins with: idf.py menuconfig -> "Example Configuration"
// Build, flash and monitor with: idf.py -p PORT flash monitor
//
// The e-Paper display needs: SCLK, PICO (MOSI / SDIN), CS#, D/C#, RES# and BUSY.
//
// SparkFun code, firmware, and software is released under the MIT License(http://opensource.org/licenses/MIT).
//
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "SparkFun_SSD168x_Library.h"

SSD1680SPI184x88Rotated *theDisplay;

// Fonts
#include "res/qw_ep_fnt_10x20.h" // 10x20

// BMP
#include "res/qw_ep_bmp_sparkfun.h"

// Arduino-style helpers
static inline void delay(unsigned long ms)
{
    TickType_t ticks = pdMS_TO_TICKS(ms);
    vTaskDelay(ticks > 0 ? ticks : 1);
}
static inline unsigned long millis(void)
{
    return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

// Configure a GPIO as an output and drive it high. Ignored if the pin is -1 (not used)
static void pinOutputHigh(int pin)
{
    if (pin < 0)
        return;
    gpio_reset_pin((gpio_num_t)pin);
    gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)pin, 1);
}

const unsigned long deepSleep_ms = 100;

// GPIO pins - set with: idf.py menuconfig -> "Example Configuration"
// The board is selected automatically by the target:
//   idf.py set-target esp32s3 : SparkFun RockBLOCK 9604 Shield (ESP32-S3) - the e-Paper display is connected directly to the ESP32-S3 using SPI
//   idf.py set-target esp32   : GoodDisplay ESP32-L Dev Kit (ESP32) - CONFIG_ESP32_L_DEV_KIT
const int pin_PICO = CONFIG_EXAMPLE_SPI_PICO_GPIO;     // SPI PICO (MOSI) - connected to the e-Paper SDIN pin
const int pin_SCLK = CONFIG_EXAMPLE_SPI_SCLK_GPIO;     // SPI SCLK
const int pin_POCI = CONFIG_EXAMPLE_SPI_POCI_GPIO;     // SPI POCI (MISO) - not used by the e-Paper display. Used by the microSD card on the Shield
const int pin_CS = CONFIG_EXAMPLE_EPAPER_CS_GPIO;      // e-Paper CS# (chip select)
const int pin_DC = CONFIG_EXAMPLE_EPAPER_DC_GPIO;      // e-Paper D/C# (data / command)
const int pin_RES = CONFIG_EXAMPLE_EPAPER_RES_GPIO;    // e-Paper RES# (reset). -1 if not connected
const int pin_BUSY = CONFIG_EXAMPLE_EPAPER_BUSY_GPIO;  // e-Paper BUSY
const int pin_VP_EN = CONFIG_EXAMPLE_VP_EN_GPIO;       // Peripheral power enable. Set high to power the e-Paper display
const int pin_SD_CS = CONFIG_EXAMPLE_SD_CS_GPIO;       // microSD CS# - shares the SPI bus. Keep high (deselected)
const int pin_EXT_CS_1 = CONFIG_EXAMPLE_EXT_CS_1_GPIO; // External SPI connector CS#1 - shares the SPI bus. Keep high (deselected)
const int pin_EXT_CS_2 = CONFIG_EXAMPLE_EXT_CS_2_GPIO; // External SPI connector CS#2 - shares the SPI bus. Keep high (deselected)
#ifdef CONFIG_ESP32_L_DEV_KIT
const char * platform = "GoodDisplay ESP32-L Dev Kit";
#else
const char * platform = "SparkFun RockBLOCK 9604 Shield";
#endif

void printTextCenter(const char *text, uint8_t yPos, QwiicEpFont &fontEpType, uint8_t kerning, bool highlight);

static void setup()
{
    delay(1000);
    
    printf("Running SSD168x example on %s\n", platform);

    // Deselect the other devices on the shared SPI bus
    pinOutputHigh(pin_SD_CS);
    pinOutputHigh(pin_EXT_CS_1);
    pinOutputHigh(pin_EXT_CS_2);

    // Enable the peripheral power for the e-Paper display
    if (pin_VP_EN >= 0)
    {
        pinOutputHigh(pin_VP_EN);
        delay(100); // Let the power stabilize
    }

    // Initialize the SPI bus
    spi_bus_config_t busConfig = {};
    busConfig.mosi_io_num = pin_PICO;
    busConfig.miso_io_num = pin_POCI;
    busConfig.sclk_io_num = pin_SCLK;
    busConfig.quadwp_io_num = -1;
    busConfig.quadhd_io_num = -1;
    busConfig.max_transfer_sz = 0; // Use the default
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &busConfig, SPI_DMA_CH_AUTO));

    // myDevice.enableDebugging(); // Uncomment this line to enable helpful debug messages on the console

    // Initialize the device and related graphics system
    theDisplay = new SSD1680SPI184x88Rotated();
    if (theDisplay->begin(SPI2_HOST, (gpio_num_t)pin_CS, (gpio_num_t)pin_DC, (gpio_num_t)pin_RES, (gpio_num_t)pin_BUSY) == false)
    {
        printf("Device begin failed. Freezing...\n");
        while (true)
            delay(1000);
    }
    printf("Begin success\n");

}

static void loop()
{
    static uint8_t y = 0;

    // ------------------------------------------------

    if (y == 0)
    {
        // Fill the whole screen
        theDisplay->rectangleFill(0, 0, theDisplay->getWidth(), theDisplay->getHeight(), COLOR_OFF);

        // Send the graphics to the device and also set the background for partial updates
        theDisplay->display();

        // Wait for display to update
        do {
            delay(10); // Check BUSY every 10ms
        } while (theDisplay->isBusy());

        theDisplay->deepSleep();

        delay(deepSleep_ms);
    }

    // ------------------------------------------------

    // erase existing text
    theDisplay->erase();

    const char hello[] = "Hello";
    printTextCenter(hello, y++, QW_EP_FONT_10X20, 1, false);

    // Partial update
    theDisplay->display(true);

    // Wait for display to update
    do {
        delay(10); // Check BUSY every 10ms
    } while (theDisplay->isBusy());

    theDisplay->deepSleep();

    delay(deepSleep_ms);

    // ------------------------------------------------

    // erase existing text
    theDisplay->erase();

    const char world[] = "World";
    printTextCenter(world, y++, QW_EP_FONT_10X20, 2, true);

    // Partial update
    theDisplay->display(true);

    // Wait for display to update
    do {
        delay(10); // Check BUSY every 10ms
    } while (theDisplay->isBusy());

    theDisplay->deepSleep();

    delay(deepSleep_ms);

    // ------------------------------------------------

    // erase existing text
    theDisplay->erase();

    theDisplay->bitmap((theDisplay->getWidth() / 2) - (BMP_SPARKFUN_WIDTH / 2), y++, QW_EP_BMP_SPARKFUN);

    // Partial update
    theDisplay->display(true);

    // Wait for display to update
    do {
        delay(10); // Check BUSY every 10ms
    } while (theDisplay->isBusy());

    theDisplay->deepSleep();

    delay(deepSleep_ms);

    // ------------------------------------------------

    if (y >= theDisplay->getHeight())
        y = 0;
}

// Given text, and location, print text center of the screen.
void printTextCenter(const char *text, uint8_t yPos, QwiicEpFont &fontEpType,
                     uint8_t kerning, bool highlight) // text, y, font type, kerning, inverted
{
    theDisplay->setFont(fontEpType);
    theDisplay->setDrawMode(grEpROPXOR);

    uint8_t boxHeight = fontEpType.height;

    uint8_t fontWidth = fontEpType.width;

    uint8_t textPixelWidth = strlen(text) * (fontWidth + kerning);

    // E.g.:
    // 8 chars in the 8X16 font, with kerning 1
    // ((strlen(text) * (fontWidth + kerning)) / 2) = 32
    // (theDisplay->getWidth() / 2) = 32
    // xStart = 0
    // But that looks rubbish if highlight is true
    int xStart = ((int)(theDisplay->getWidth() / 2)) - ((int)(textPixelWidth / 2));
    if (xStart < 0)
        xStart = 0;

    // So, add a gap of 1 pixel if highlight is true and xStart is zero
    if (highlight && (xStart == 0))
        xStart = 1;

    uint8_t xPos = xStart;
    for (int x = 0; x < strlen(text); x++)
    {
        theDisplay->setCursor(xPos, yPos);
        theDisplay->print(text[x]);
        xPos += fontWidth + kerning;
    }

    if (highlight) // Draw a box, inverted over text
    {
        // Error check
        int xBoxStart = xStart - 5;
        int xBoxWidth = textPixelWidth + 9;
        if (xBoxStart < 0)
        {
            xBoxWidth += xBoxStart * 2; // Shrink the width by twice the excess
            xBoxStart = 0;
        }
        if ((xBoxStart + xBoxWidth) > theDisplay->getWidth())
            xBoxWidth = theDisplay->getWidth() - xBoxStart;

        theDisplay->rectangleFill(xBoxStart, yPos, xBoxWidth, boxHeight, 1); // x, y, width, height, color
    }
}

extern "C" void app_main(void)
{
    setup();

    while (true)
    {
        loop();
        delay(1); // Let other tasks run
    }
}
