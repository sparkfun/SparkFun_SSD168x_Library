// Example09 : 184x88 clock emulator using partial updates
//
// Written by P.C. @ SparkFun Electronics, April 2026
// SPI version: November 2026
//
// ESP-IDF version of the Arduino example Example09_184x88_Clock.ino
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

SSD1680SPI184x88 myDevice;

// Fonts
#include "res/qw_ep_fnt_largenum.h"

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

// Static char array to hold the previous time
// Load the previous time with zeros for the background / basemap for partial updates
static char previousTime[strlen("HHMMSS") + 1] = { '0', '0', '0', '0', '0', '0', '\0' };

// Define the start coordinates for displaying the time digits
// Digit 0 is the 10's of hours
typedef struct {
    const int x;
    const int y;
} topLeft_t;

const topLeft_t topLeftDigitCoords[] = {
    { 32, 15 },
    { 44, 15 },
    { 32, 68 },
    { 44, 68 },
    { 32, 121 },
    { 44, 121 },
};

// Change this to invert the colors
const bool invertColors = false;

// This defines how many digit changes trigger a full update
typedef enum {
    EVERY_SECOND = 1,
    EVERY_10_SECONDS,
    EVERY_MINUTE,
    EVERY_10_MINUTES,
    EVERY_HOUR,
    EVERY_10_HOURS,
} numCharsChangedLimit_e;

// Change numCharsChangedLimit to change how often a full update is performed
const numCharsChangedLimit_e numCharsChangedLimit = EVERY_MINUTE;

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

    // Fill a rectangle on the screen that has a 4 pixel border
    myDevice.rectangleFill(4, 4, myDevice.getWidth() - 8, myDevice.getHeight() - 8, invertColors ? COLOR_OFF : COLOR_ON);

    // Fill a rectangle within that, to leave an 4 pixel frame
    myDevice.rectangleFill(8, 8, myDevice.getWidth() - 16, myDevice.getHeight() - 16, invertColors ? COLOR_ON : COLOR_OFF);

    // Add the 000000 held in previousTime - this becomes the background for partial updates
    myDevice.setFont(QW_EP_FONT_LARGENUM);
    for (int i = 0; i < strlen("HHMMSS"); i++)
    {
        char newChar[2];
        sprintf(newChar, "%c", previousTime[i]);
        myDevice.text(topLeftDigitCoords[i].x, topLeftDigitCoords[i].y, newChar, invertColors ? COLOR_OFF : COLOR_ON);
    }

    // There's nothing on the screen yet
    // Send the graphics to the device and also set the background for partial updates
    myDevice.display();

    // Wait for display to update
    while (myDevice.isBusy())
        delay(10); // Check BUSY every 10ms
}

static void loop()
{
    // char array to hold the time
    char theTime[strlen("HHMMSS") + 1];

    // split millis into hours, mins and secs
    int hh,mm,ss;
    ss = millis() / 1000; // Convert millis to seconds
    ss = ss % 86400; // Limit to 23:59:59
    hh = ss / 3600;
    ss -= hh * 3600;
    mm = ss / 60;
    ss -= mm * 60;

    // print into theTime
    sprintf(theTime, "%02d%02d%02d", hh, mm, ss);

    // Work out how many and which characters have changed
    int numCharsChanged = 0;
    bool charsChanged[strlen("HHMMSS")];
    for (int i = 0; i < strlen("HHMMSS"); i++)
    {
        if (theTime[i] != previousTime[i])
        {
            charsChanged[i] = true;
            numCharsChanged++;
        }
        else
            charsChanged[i] = false;
    }

    // if any characters have changed
    if (numCharsChanged > 0)
    {
        printf("%s\n", theTime);

        // for each character that has changed, erase it and update it
        for (int i = 0; i < strlen("HHMMSS"); i++)
        {
            if (charsChanged[i]) // Comment this if statement if desired to always update all the characters
            {
                myDevice.rectangleFill(topLeftDigitCoords[i].x, topLeftDigitCoords[i].y,
                                FONT_LARGENUM_WIDTH, FONT_LARGENUM_HEIGHT, invertColors ? COLOR_ON : COLOR_OFF);
                char newChar[2];
                sprintf(newChar, "%c", theTime[i]);
                myDevice.text(topLeftDigitCoords[i].x, topLeftDigitCoords[i].y, newChar, invertColors ? COLOR_OFF : COLOR_ON);
            }
        }

        // Do a partial update if numCharsChanged is less than numCharsChangedLimit
        bool partial = numCharsChanged < numCharsChangedLimit;

        if (!partial)
            printf("Performing full update\n");

        myDevice.display(partial);

        // Wait for display to update
        do {
            delay(10); // Check BUSY every 10ms
        } while (myDevice.isBusy());

        myDevice.deepSleep();

        // update previousTime
        strcpy(previousTime, theTime);
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
