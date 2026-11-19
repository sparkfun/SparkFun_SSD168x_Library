// Example10 : 184x88 testing partial updates - text erase and display bitmap
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

SSD1680SPI184x88Rotated *theDisplay;

// Fonts
#include <res/qw_ep_fnt_10x20.h> // 10x20

// BMP
#include <res/qw_ep_bmp_sparkfun.h>

const unsigned long deepSleep_ms = 100;

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
    theDisplay = new SSD1680SPI184x88Rotated();
    if (theDisplay->begin(pin_CS, pin_DC, pin_RES, pin_BUSY) == false)
    {
        Serial.println("Device begin failed. Freezing...");
        while (true)
            ;
    }
    Serial.println("Begin success");

}

void loop()
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

