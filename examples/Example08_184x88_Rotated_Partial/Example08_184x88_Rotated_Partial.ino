// Example08 : 184x88 testing partial updates
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
// Notes on the GoodDisplay GDEM0097T61:
//
// Command 0x2D (Read SSD1680 OTP Register) returns:
// VCOM OTP Selection (Command 0x37, Byte A):  0x00
// VCOM Register (Command 0x2C):               0x00
// Display Mode (Command 0x37, Bytes B-F):     0x00 0x01 0x00 0x00 0x40
// Waveform version (Command 0x37, Bytes G-J): 0x00 0x00 0x00 0x00
//
// Mode 2 ping-pong is enabled: bit F[6] is set
//
// Command 0x2E (Read SSD1680 User ID) returns: 0xCA 0xFE 0x00 0x16 0x80 0x00 0x55 0x01 0x00 0xDB
//
// The ping-pong causes some unexpected results with partial updates
// If we want to move or scroll a digit by: erasing it with a single space; and adding the digit at a new position
// We need to perform the erase-and-add twice, so that both halves of the SSD1680 RAM are updated
//
// SparkFun code, firmware, and software is released under the MIT License(http://opensource.org/licenses/MIT).
//
// SPDX-License-Identifier: MIT

#include <SparkFun_SSD168x_Library.h> // http://librarymanager/All#SparkFun_SSD168x_Library

SSD1680SPI184x88Rotated myDevice;

// Fonts
#include <res/qw_ep_fnt_largenum.h> // 12x48

// Define the start coordinates for displaying the digits
// xStart 23 allows for half-digit overlap ((184 - (12 * 11.5)) / 2)
// To check the rectangleFill() erase is working correctly, set xStart
// to (e.g.) 70 to print the 9 over the screen boundary
// Or set yStart to (e.g.) 62 to print all digits over the boundary
const int xStart = 23;
const int yStart = 20;

// Change this to invert the colors
// The ghosting from the partial updates is more evident when inverted
const bool invertColors = false;

// Change this to false to display stripes instead of digits
const bool displayDigits = true;

const int numDigits = 11; // Print 0123456789: using partial updates
const int numLoops = 3; // Print digits this many times before doing a full refresh
const int numWrites = 1; // Write each partial update this many times - useful to check ghosting

// Change this to true to overlap the digits by half a digit - useful for erase tests
const bool overlapDigits = false;

// Change this to true to use erase() to erase the digits / stripes instead of rectangleFill()
const bool useErase = false;

const unsigned long deepSleep_ms = 1000;

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

    myDevice.setFont(QW_EP_FONT_LARGENUM);
}

void loop()
{
    static int digitCount = 0;
    static int loopCount = 0;
    static int digitLimit = overlapDigits ? numDigits * 2 : numDigits;

    if ((digitCount == 0) && (loopCount == 0))
    {
        // Fill the whole screen
        myDevice.rectangleFill(0, 0, myDevice.getWidth(), myDevice.getHeight(), invertColors ? COLOR_ON : COLOR_OFF);

        // Send the graphics to the device and also set the background for partial updates
        myDevice.display();

        // Wait for display to update
        do {
            delay(10); // Check BUSY every 10ms
        } while (myDevice.isBusy());
    }

    // Digits 0123456789:
    if (displayDigits)
    {
        int xinc = FONT_LARGENUM_WIDTH;
        if (overlapDigits)
            xinc /= 2;

        for (int w = 0; w < numWrites; w++) // Do each partial update this many times
        {
            // Erase the previous digit
            if ((digitCount == 0) && (loopCount == 0))
            {
                // Nothing to do
            }
            else
            {
                if (useErase)
                {
                    // Use erase() to clear the previous digit
                    myDevice.erase();
                }
                else
                {
                    // Draw over the previous digit with rectangleFill
                    int i = digitCount - 1;
                    if (i < 0)
                        i = digitLimit - 1;
        
                    myDevice.rectangleFill(xStart + i * xinc, yStart,
                                        FONT_LARGENUM_WIDTH, FONT_LARGENUM_HEIGHT,
                                        invertColors ? COLOR_ON : COLOR_OFF);
                }
            }

            // Write the new digit
            char newChar[2];
            sprintf(newChar, "%c", '0' + (digitCount % numDigits));
            myDevice.text(xStart + digitCount * xinc, yStart, newChar,
                          invertColors ? COLOR_OFF : COLOR_ON);

            // Partial update
            myDevice.display(true);

            // Wait for display to update
            do {
                delay(10); // Check BUSY every 10ms
            } while (myDevice.isBusy());
        }
    }

    // Stripes : 11 8-pixel stripes, matches the display width of 88 pixels in bytes
    else
    {
        int yinc = 8;
        if (overlapDigits)
            yinc /= 2;

        for (int w = 0; w < numWrites; w++) // Do each partial update this many times
        {
            // Erase the previous stripe
            if ((digitCount == 0) && (loopCount == 0))
            {
                // Nothing to do
            }
            else
            {
                if (useErase)
                {
                    // Use erase() to clear the previous digit
                    myDevice.erase();
                }
                else
                {
                    // Draw over the previous digit with rectangleFill
                    int i = digitCount - 1;
                    if (i < 0)
                        i = digitLimit - 1;

                    myDevice.rectangleFill(0, i * yinc, 184, 8, invertColors ? COLOR_ON : COLOR_OFF);
                }
            }

            // Write the new stripe
            myDevice.rectangleFill(0, digitCount * yinc, 184, 8, invertColors ? COLOR_OFF : COLOR_ON);

            // Partial update
            myDevice.display(true);

            // Wait for display to update
            do {
                delay(10); // Check BUSY every 10ms
            } while (myDevice.isBusy());
        }
    }

    // Increment the counters
    digitCount++;
    digitCount %= digitLimit;
    if (digitCount == 0)
    {
        loopCount++;
        loopCount %= numLoops;
    }

    // Put display into deep sleep - just to prove we can
    myDevice.deepSleep();

    // Delay
    delay(deepSleep_ms);
}