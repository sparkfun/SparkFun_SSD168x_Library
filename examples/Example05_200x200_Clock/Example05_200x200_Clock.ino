// Example05 : 200x200 clock emulator using partial updates
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
// ******************************************************************************************************
// * NOTE: This example needs (e.g.) the GoodDisplay GDEY0154D67 1.54inch 200x200 pixel e-paper display *
// ******************************************************************************************************
//
// SparkFun code, firmware, and software is released under the MIT License(http://opensource.org/licenses/MIT).
//
// SPDX-License-Identifier: MIT

#include <SparkFun_SSD168x_Library.h> // http://librarymanager/All#SparkFun_SSD168x_Library

SSD1681SPI200x200 myDevice;

// Fonts
#include <res/qw_ep_fnt_largenum.h>

// Static char array to hold the previous time
// Load the previous time with zeros for the background / basemap for partial updates
static char previousTime[strlen("HHHH:MM:SS") + 1] = { '0', '0', '0', '0', ':', '0', '0', ':', '0', '0', '\0' };

// Define the start coordinates for displaying the time
const int xStart = 40;
const int yStart = 76;

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

    // Fill a rectangle on the screen that has a 4 pixel border
    myDevice.rectangleFill(4, 4, myDevice.getWidth() - 8, myDevice.getHeight() - 8, invertColors ? COLOR_OFF : COLOR_ON);

    // Fill a rectangle within that, to leave an 4 pixel frame
    myDevice.rectangleFill(8, 8, myDevice.getWidth() - 16, myDevice.getHeight() - 16, invertColors ? COLOR_ON : COLOR_OFF);

    // Add the 0000:00:00 held in previousTime - this becomes the background for partial updates
    myDevice.setFont(QW_EP_FONT_LARGENUM);
    myDevice.text(xStart, yStart, previousTime, invertColors ? COLOR_OFF : COLOR_ON);

    // There's nothing on the screen yet
    // Send the graphics to the device and also set the background for partial updates
    myDevice.display();

    // Wait for display to update
    while (myDevice.isBusy())
        delay(10); // Check BUSY every 10ms
}

void loop()
{
    // char array to hold the time
    char theTime[strlen("HHHH:MM:SS") + 1];

    // split millis into hours, mins and secs
    // (unsigned, and limited to 9999 hours, so theTime cannot overflow)
    unsigned long secs = millis() / 1000;
    unsigned int hh = (secs / 3600) % 10000;
    unsigned int mm = (secs / 60) % 60;
    unsigned int ss = secs % 60;

    // print into theTime
    snprintf(theTime, sizeof(theTime), "%04u:%02u:%02u", hh, mm, ss);

    // work out how many and which characters have changed
    int numCharsChanged = 0;
    bool charsChanged[strlen("HHHH:MM:SS")];
    for (int i = 0; i < strlen("HHHH:MM:SS"); i++)
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
        Serial.println(theTime);

        // for each character that has changed, erase it and update it
        for (int i = 0; i < strlen("HHHH:MM:SS"); i++)
        {
            if (charsChanged[i]) // Comment this if statement to always update all the characters
            {
                myDevice.rectangleFill(xStart + i * FONT_LARGENUM_WIDTH, yStart,
                                FONT_LARGENUM_WIDTH, FONT_LARGENUM_HEIGHT, invertColors ? COLOR_ON : COLOR_OFF);
                char newChar[2];
                sprintf(newChar, "%c", theTime[i]);
                myDevice.text(xStart + i * FONT_LARGENUM_WIDTH, yStart, newChar, invertColors ? COLOR_OFF : COLOR_ON);
            }
        }

        // Do a partial update if numCharsChanged is less than numCharsChangedLimit
        bool partial = numCharsChanged < numCharsChangedLimit;

        if (!partial)
            Serial.println("Performing full update");

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
