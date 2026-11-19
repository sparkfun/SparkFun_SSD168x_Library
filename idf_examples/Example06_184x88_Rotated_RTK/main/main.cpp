// Example06 : RTK display emulator (184x88) using partial updates
//
// Written by P.C. @ SparkFun Electronics, April 2026
// SPI version: November 2026
//
// ESP-IDF version of the Arduino example Example06_184x88_Rotated_RTK.ino
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

SSD1680SPI184x88Rotated myDevice;

// Fonts
#include "res/qw_ep_fnt_5x7.h"
#include "res/qw_ep_fnt_8x16.h"

// Icons
#include "icons.h"

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

// Change this to invert the colors
const bool invertColors = false;

// Change this to change how often the full update is performed
// Useful for evaluating the ghosting from successive partial updates
const int fullUpdateEvery = 60;

// Top-Left corner
const int startX = 28;
const int startY = 12;

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
    if (myDevice.begin(SPI2_HOST, (gpio_num_t)pin_CS, (gpio_num_t)pin_DC, (gpio_num_t)pin_RES, (gpio_num_t)pin_BUSY) == false)
    {
        printf("Device begin failed. Freezing...\n");
        while (true)
            delay(1000);
    }
    printf("Begin success\n");

    // Fill the whole screen
    myDevice.rectangleFill(0, 0, myDevice.getWidth(), myDevice.getHeight(), invertColors ? COLOR_ON : COLOR_OFF);

    // Add the BT MAC top right
    myDevice.setFont(QW_EP_FONT_5X7);
    myDevice.text( startX + 0, startY + 0, "A4DA", invertColors ? COLOR_OFF : COLOR_ON);

    // More text
    myDevice.text( startX + 0, startY + 56, "192.168.1.100", invertColors ? COLOR_OFF : COLOR_ON);

    // Add the WiFi3 icon
    myDevice.setDrawMode( invertColors ? grEpROPNotCopy : grEpROPCopy );
    myDevice.bitmap( startX + 34, startY + 0, (uint8_t *)WiFi_Symbol_3, WiFi_Symbol_Width, WiFi_Symbol_Height );

    // More icons
    myDevice.bitmap( startX + 108, startY + 0, (uint8_t *)Battery_3, Battery_Width, Battery_Height );
    myDevice.bitmap( startX + 0, startY + 26, (uint8_t *)CrossHairDual, CrossHairDual_Width, CrossHairDual_Height );
    myDevice.bitmap( startX + 74, startY + 26, (uint8_t *)SIV_Antenna_PPP, SIV_Antenna_PPP_Width, SIV_Antenna_PPP_Height );
    myDevice.bitmap( startX + 96, startY + 50, (uint8_t *)Corr_TCP_Icon, Corr_Icon_Width, Corr_Icon_Height );
    //myDevice.bitmap( startX + , startY + , (uint8_t *), _Width, _Height );

    // There's nothing on the screen yet
    // Send the graphics to the device and also set the background for partial updates
    myDevice.display();

    // Wait for display to update
    while (myDevice.isBusy())
        delay(10); // Check BUSY every 10ms
}

static void loop()
{
    static int loopCount = 0;
    char theText[8];

    // Add the SIV and HPA
    
    if (invertColors)
        myDevice.setDrawMode( invertColors ? grEpROPCopy : grEpROPNotCopy );

    myDevice.setFont(QW_EP_FONT_8X16);

    myDevice.rectangleFill( startX + 16, startY + 27, 5 * 8, 16, invertColors ? COLOR_ON : COLOR_OFF );
    sprintf(theText, ":.%03d", loopCount % 1000);
    myDevice.text( startX + 16, startY + 27, theText, invertColors ? COLOR_OFF : COLOR_ON);

    myDevice.rectangleFill( startX + 90, startY + 26, 3 * 8, 16, invertColors ? COLOR_ON : COLOR_OFF );
    sprintf(theText, ":%d", loopCount % 100);
    myDevice.text( startX + 90, startY + 26, theText, invertColors ? COLOR_OFF : COLOR_ON);

    if (loopCount % 2 == 1)
        myDevice.rectangleFill( startX + 74, startY + 0, DownloadArrow_Width, DownloadArrow_Height, invertColors ? COLOR_ON : COLOR_OFF );

    if (invertColors)
        myDevice.setDrawMode( invertColors ? grEpROPNotCopy : grEpROPCopy );

    // Blink the DownloadArrow
    if (loopCount % 2 == 0)
        myDevice.bitmap( startX + 74, startY + 0, (uint8_t *)DownloadArrow, DownloadArrow_Width, DownloadArrow_Height );

    // Update the logging icon
    if (loopCount % 4 == 0)
        myDevice.bitmap( startX + 119, startY + 52, (uint8_t *)Logging_0, Logging_Width, Logging_Height );
    else if (loopCount % 4 == 1)
        myDevice.bitmap( startX + 119, startY + 52, (uint8_t *)Logging_1, Logging_Width, Logging_Height );
    else if (loopCount % 4 == 2)
        myDevice.bitmap( startX + 119, startY + 52, (uint8_t *)Logging_2, Logging_Width, Logging_Height );
    else // if (loopCount % 4 == 3)
        myDevice.bitmap( startX + 119, startY + 52, (uint8_t *)Logging_3, Logging_Width, Logging_Height );

    // Perform a full update when loopCount reaches fullUpdateEvery
    bool partial = ((loopCount % fullUpdateEvery) != 0);

    myDevice.display(partial);

    // Wait for display to update
    do {
        delay(10); // Check BUSY every 10ms
    } while (myDevice.isBusy());

    // Put display into deep sleep
    myDevice.deepSleep();

    loopCount++;

    delay(1000 - (millis() % 1000)); // Wait for the next second to elapse
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
