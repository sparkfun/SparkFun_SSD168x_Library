// Example06 : RTK display emulator (184x88) using partial updates
//
// Written by P.C. @ SparkFun Electronics, April 2026
// SPI version: November 2026
//
// This is a library to control SSD1680/1 e-Paper displays via SPI.
//
// This example is written for the ESP32-S3 on the SparkFun RockBLOCK 9604 Shield, where the
// GoodDisplay GDEM0097T61 0.97" (184x88 pixels, SSD1680) e-Paper display is connected
// directly to the ESP32-S3 using SPI. The SPI bus is shared with the microSD card.
// It can also run on the GoodDisplay ESP32-L Dev Kit: uncomment "#define ESP32_L_DEV_KIT" below.
// Change the pin definitions below to suit your hardware.
//
// The e-Paper display needs: SCLK, PICO (MOSI / SDIN), CS#, D/C#, RES# and BUSY.
//
// SparkFun code, firmware, and software is released under the MIT License(http://opensource.org/licenses/MIT).
//
// SPDX-License-Identifier: MIT

#include <SparkFun_SSD168x_Library.h> // http://librarymanager/All#SparkFun_SSD168x_Library

SSD1680SPI184x88Rotated myDevice;

// Fonts
#include <res/qw_ep_fnt_5x7.h>
#include <res/qw_ep_fnt_8x16.h>

// Icons
#include "icons.h"

// Change this to invert the colors
const bool invertColors = false;

// Change this to change how often the full update is performed
// Useful for evaluating the ghosting from successive partial updates
const int fullUpdateEvery = 60;

// Top-Left corner
const int startX = 28;
const int startY = 12;

// Adjust these values according to your configuration
//------------------------------------------------------------------------------

// Pre-defined boards:
// By default, the pins are set for the SparkFun RockBLOCK 9604 Shield (ESP32-S3)
// Uncomment the next line to run the example on the GoodDisplay ESP32-L Dev Kit (ESP32)
//#define ESP32_L_DEV_KIT

#ifdef  ESP32_L_DEV_KIT

// GoodDisplay ESP32-L Dev Kit (ESP32), with the DESPI-C02 adapter (+ the CV0097 adapter for the 0.97" display)
// Arduino IDE: select the "ESP32 Dev Module" board
// These are the pins used by GoodDisplay's own example code (A14 = GPIO13, A15 = GPIO12, A16 = GPIO14, A17 = GPIO27)
const int pin_PICO = 23;     // SPI PICO (MOSI) - connected to the e-Paper SDIN pin
const int pin_SCLK = 18;     // SPI SCLK
const int pin_POCI = -1;     // SPI POCI (MISO) - not used by the e-Paper display
const int pin_CS = 27;       // e-Paper CS# (chip select)
const int pin_DC = 14;       // e-Paper D/C# (data / command)
const int pin_RES = 12;      // e-Paper RES# (reset). Use -1 if not connected
const int pin_BUSY = 13;     // e-Paper BUSY
const int pin_VP_EN = -1;    // Not used on the Dev Kit
const int pin_SD_CS = -1;    // Not used on the Dev Kit
const int pin_EXT_CS_1 = -1; // Not used on the Dev Kit
const int pin_EXT_CS_2 = -1; // Not used on the Dev Kit
const char * platform = "GoodDisplay ESP32-L Dev Kit";

#else   // ESP32_L_DEV_KIT

// SparkFun RockBLOCK 9604 Shield (ESP32-S3)
// The e-Paper display is connected directly to the ESP32-S3 using SPI
// Arduino IDE: select the "ESP32S3 Dev Module" board
const int pin_PICO = 11;     // SPI PICO (MOSI) - connected to the e-Paper SDIN pin
const int pin_SCLK = 12;     // SPI SCLK
const int pin_POCI = 13;     // SPI POCI (MISO) - not used by the e-Paper display. Used by the microSD card
const int pin_CS = 15;       // e-Paper CS# (chip select)
const int pin_DC = 7;        // e-Paper D/C# (data / command)
const int pin_RES = 6;       // e-Paper RES# (reset). Use -1 if not connected
const int pin_BUSY = 5;      // e-Paper BUSY
const int pin_VP_EN = 4;     // Peripheral power enable. Set high to power the e-Paper display
const int pin_SD_CS = 10;    // microSD CS# - shares the SPI bus. Keep high (deselected)
const int pin_EXT_CS_1 = 1;  // External SPI connector CS#1 - shares the SPI bus. Keep high (deselected)
const int pin_EXT_CS_2 = 2;  // External SPI connector CS#2 - shares the SPI bus. Keep high (deselected)
const char * platform = "SparkFun RockBLOCK 9604 Shield";

#endif  // ESP32_L_DEV_KIT

// Configure a pin as an output and drive it high. Ignored if the pin is -1 (not used)
void pinOutputHigh(int pin)
{
    if (pin < 0)
        return;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
}

void setup()
{
    delay(1000);
    
    // Start serial
    Serial.begin(115200);
    Serial.print("Running SSD168x example on ");
    Serial.println(platform);

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

    // Start SPI
#if defined(ARDUINO_ARCH_ESP32)
    SPI.begin(pin_SCLK, pin_POCI, pin_PICO);
#else
    SPI.begin();
#endif

    // Initialize the device and related graphics system
    if (myDevice.begin(pin_CS, pin_DC, pin_RES, pin_BUSY) == false)
    {
        Serial.println("Device begin failed. Freezing...");
        while (true)
            ;
    }
    Serial.println("Begin success");

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

void loop()
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

    while (millis() % 1000 != 0)
        ; // Wait for the next second to elapse
}
