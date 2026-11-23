/**
 * @date 2026
 * @copyright Copyright (c) 2026, SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 *
 * Please see LICENSE.md for more details
 *
 * A library to control SSD1680/1 e-Paper displays via SPI
 *
 * Original I2C version by Paul Clark @ SparkFun Electronics, April 2026
 * SPI rework by Claude, directed by Paul Clark @ SparkFun Electronics, November 2026
 *
 * https://github.com/sparkfun/SparkFun_SSD168x_Library
 *
 * @file SparkFun_SSD168x_Library.h
 *
 * The SSD1680/1 is connected using a 4-wire SPI interface plus two GPIO pins:
 *
 *   SCLK      SPI clock
 *   SDIN      SPI data in (PICO / MOSI). The SSD168x does not use POCI (MISO)
 *   CS#       Chip select - active low. Driven by the library
 *   D/C#      Data / command select. Low for command bytes, high for data / parameter bytes. Driven by the library
 *   RES#      Reset - active low. Driven by the library. Optional: if not connected, a software reset is used
 *   BUSY      Busy - high while the SSD168x is busy. Read by the library
 *
 * The CS#, D/C#, RES# and BUSY pins are passed to begin(). The SPI bus can be shared with other devices.
 *
 * Arduino:
 *     SPI.begin(); // (ESP32: SPI.begin(SCLK, POCI, PICO);)
 *     myDisplay.begin(CS, DC, RES, BUSY); // Uses SPI at 4MHz. Optionally pass the SPI port and clock speed too
 *
 * ESP-IDF:
 *     spi_bus_initialize(SPI2_HOST, &busConfig, SPI_DMA_CH_AUTO);
 *     myDisplay.begin(SPI2_HOST, CS, DC, RES, BUSY); // Optionally pass the clock speed too
 */

#pragma once

#include "sfe_platform.h"
#include "sfe_bus.h"
#include "sfe_debug.h"

// include the underlying SDK implementation headers for the SSD168x devices
#include "qwiic_ssd1680_184x88.h"
#include "qwiic_ssd1681_200x200.h"
#include "qwiic_ssd1680_custom.h"
#include "qwiic_ssd1681_custom.h"
#include "qwiic_ssd1680_184x88_rotated.h"

#if defined(SFE_SSD168X_ESP_IDF)
#include <stdarg.h>
#endif

// Friendly typenames
typedef QwEpFont QwiicEpFont;
typedef QwEpBitmap QwiicEpBitmap;

// COLOR_ON and COLOR_OFF are defined in qwiic_grbuffer.cpp:
//   COLOR_ON  = 1 indicates the pixel is 'on': black on e-paper (inverted)
//   COLOR_OFF = 0 indicates the pixel is 'off': white on e-paper (inverted)

#if defined(SFE_SSD168X_ESP_IDF)

/**
 * @brief A minimal equivalent of the Arduino Print class, for native ESP-IDF.
 *
 * Provides print(), println() and printf() for the display text cursor. Each character is sent to write().
 */
class SSD168xPrint
{
  public:
    virtual ~SSD168xPrint() {}

    /**
     * @brief Write one character. Implemented by the display class.
     *
     * @param theChar The character to write.
     * @return The number of characters written (1), or 0 on failure.
     */
    virtual size_t write(uint8_t theChar) = 0;

    /** @brief Print a string. @param str The null-terminated string. @return The number of characters written. */
    size_t print(const char *str)
    {
        size_t n = 0;
        if (str)
            while (*str)
                n += write((uint8_t)*str++);
        return n;
    }
    /** @brief Print a string. @param str The string. @return The number of characters written. */
    size_t print(const std::string &str)
    {
        return print(str.c_str());
    }
    /** @brief Print a single character. @param c The character. @return The number of characters written. */
    size_t print(char c)
    {
        return write((uint8_t)c);
    }
    /** @brief Print a signed integer in decimal. @param value The value. @return The number of characters written. */
    size_t print(long value)
    {
        char buf[24];
        snprintf(buf, sizeof(buf), "%ld", value);
        return print(buf);
    }
    /** @brief Print a signed integer in decimal. @param value The value. @return The number of characters written. */
    size_t print(int value)
    {
        return print((long)value);
    }
    /** @brief Print an unsigned integer in a given base. @param value The value. @param base DEC (default) or HEX. @return The number of characters written. */
    size_t print(unsigned long value, int base = DEC)
    {
        char buf[24];
        snprintf(buf, sizeof(buf), (base == HEX) ? "%lX" : "%lu", value);
        return print(buf);
    }
    /** @brief Print an unsigned integer in a given base. @param value The value. @param base DEC (default) or HEX. @return The number of characters written. */
    size_t print(unsigned int value, int base = DEC)
    {
        return print((unsigned long)value, base);
    }
    /** @brief Print a floating point value. @param value The value. @param decimalPlaces The number of decimal places (default 2). @return The number of characters written. */
    size_t print(double value, int decimalPlaces = 2)
    {
        char buf[40];
        snprintf(buf, sizeof(buf), "%.*f", decimalPlaces, value);
        return print(buf);
    }
    /** @brief Start a new line. @return The number of characters written. */
    size_t println(void)
    {
        return write((uint8_t)'\n');
    }
    /** @brief Print a string followed by a new line. @param str The null-terminated string. @return The number of characters written. */
    size_t println(const char *str)
    {
        return print(str) + println();
    }
    /** @brief Print a string followed by a new line. @param str The string. @return The number of characters written. */
    size_t println(const std::string &str)
    {
        return print(str) + println();
    }
    /** @brief Print a signed integer followed by a new line. @param value The value. @return The number of characters written. */
    size_t println(long value)
    {
        return print(value) + println();
    }
    /** @brief Print a signed integer followed by a new line. @param value The value. @return The number of characters written. */
    size_t println(int value)
    {
        return print(value) + println();
    }
    /** @brief Print an unsigned integer followed by a new line. @param value The value. @param base DEC (default) or HEX. @return The number of characters written. */
    size_t println(unsigned long value, int base = DEC)
    {
        return print(value, base) + println();
    }
    /** @brief Print an unsigned integer followed by a new line. @param value The value. @param base DEC (default) or HEX. @return The number of characters written. */
    size_t println(unsigned int value, int base = DEC)
    {
        return print(value, base) + println();
    }
    /** @brief Print a floating point value followed by a new line. @param value The value. @param decimalPlaces The number of decimal places (default 2). @return The number of characters written. */
    size_t println(double value, int decimalPlaces = 2)
    {
        return print(value, decimalPlaces) + println();
    }
    /**
     * @brief Print formatted text, like printf().
     *
     * @param format The printf format string. The formatted text is limited to 128 characters.
     * @return The number of characters written.
     */
    size_t printf(const char *format, ...) __attribute__((format(printf, 2, 3)))
    {
        char buf[129];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        return print(buf);
    }
};

typedef SSD168xPrint SSD168xPrintBase; // The base class for the text output: SSD168xPrint on ESP-IDF

#else

typedef Print SSD168xPrintBase; // The base class for the text output: Arduino Print

#endif

// The Plan:
//
// For each supported device the following is needed
//
//      - A common Arduino / ESP-IDF interface/implementation. One impl, one area to maintain
//      - Classes that are specialized for each device
//      - No abstract methods that a subclass must implement
//
// The solution
//    A templated base class that device specific sub-classes derive from.
//

// Define the template and fill in the interface methods in-line.

/**
 * @brief The templated base class for all SSD168x SPI e-Paper displays.
 *
 * Implements the Arduino Print interface (or SSD168xPrint on ESP-IDF), so print(), println() (and printf())
 * can be used to write text at the cursor position.
 *
 * @tparam DeviceType The device driver class (e.g. SpiSsd1680_184x88).
 */
template <typename DeviceType> class SSD168xSPIBaseClass : public SSD168xPrintBase
{
  public:
    // The platform types, from namespace SparkFun_SSD168x (see sfe_platform.h and sfe_bus.h)
    typedef SparkFun_SSD168x::sfe_print_t sfe_print_t;   ///< The print type: Print on Arduino; SfeOutput on ESP-IDF
    typedef SparkFun_SSD168x::sfe_string_t sfe_string_t; ///< The string type: String on Arduino; std::string on ESP-IDF

  protected:
    // our device driver
    DeviceType m_device;

  private:
    SparkFun_SSD168x::SfeSPI m_spiBus; // our SPI bus object

    // for the Arduino print functionality
    uint8_t m_cursorX;
    uint8_t m_cursorY;

    uint8_t m_color;

    ///////////////////////////////////////////////////////////////////////
    // initDevice()
    //
    // Common code for all begin() methods - once the SPI bus object has been initialized

    bool initDevice(int resetPin, int busyPin)
    {
        // defaults for Arduino Print
        setCursor(0, 0);
        setColor(COLOR_ON);

        m_device.setCommBus(m_spiBus, resetPin, busyPin);

        // call init on the device
        bool bStatus = m_device.init();

        // Want to start cursor at Y height of the current font, if we have a font.
        //
        // Get our font height ... a default font is set during init ...
        if (bStatus)
        {
            QwiicEpFont *pFont = m_device.font();
            if (pFont)
                m_cursorY = pFont->height;
        }

        return bStatus;
    }

  public:
#if defined(SFE_SSD168X_ARDUINO)

    ///////////////////////////////////////////////////////////////////////
    // begin()
    //
    // This method is called to initialize the SSD168x library and the connection to
    // the e-Paper display. This method must be called before calling any graphics methods.
    //
    // This method follows the standard startup pattern in SparkFun Arduino
    // libraries.
    //
    // The SPI port must have been started (SPI.begin()) before calling begin().

    /**
     * @brief Initialize the SSD168x library and the e-Paper display, using SPI.
     *
     * Must be called before calling any graphics methods. The SPI port must have been started
     * (e.g. SPI.begin(), or SPI.begin(SCLK, POCI, PICO) on ESP32) before calling begin().
     * begin() resets the display, initializes it, clears it and performs a full update.
     *
     * @param csPin The chip select (CS#) pin (required).
     * @param dcPin The data / command (D/C#) pin (required).
     * @param resetPin The reset (RES#) pin. Use -1 if RES# is not connected: a software reset is used instead.
     * @param busyPin The BUSY pin (required).
     * @param spiPort optional. The SPI port. If not provided, the default port (SPI) is used.
     * @param spiClockHz optional. The SPI clock speed in Hz. Defaults to SparkFun_SSD168x::kDefaultSpiClockHz (4MHz).
     * @return true on success, false on startup failure.
     */
    bool begin(int csPin, int dcPin, int resetPin, int busyPin, SPIClass &spiPort = SPI,
               uint32_t spiClockHz = SparkFun_SSD168x::kDefaultSpiClockHz)
    {
        if (!m_spiBus.init(spiPort, spiClockHz, csPin, dcPin))
        {
            m_device.debugPrintln("begin: invalid CS or D/C pin", true);
            return false;
        }

        return initDevice(resetPin, busyPin);
    }

    /**
     * @brief Enable debug messages, sent to the given port.
     *
     * @param debugPort optional. The port to print debug messages to. Defaults to Serial.
     * @param printLimitedDebug optional. If true, only print messages flagged as "important". Defaults to false.
     */
    void enableDebugging(Print &debugPort = Serial, bool printLimitedDebug = false)
    {
        m_device.enableDebugging(debugPort, printLimitedDebug);
    }

#elif defined(SFE_SSD168X_ESP_IDF)

    /**
     * @brief Initialize the SSD168x library and the e-Paper display, using an SPI bus initialized by the user.
     *
     * Must be called before calling any graphics methods. The SPI bus must have been initialized with
     * spi_bus_initialize() before calling begin(). The display is added to the bus as a device.
     * begin() resets the display, initializes it, clears it and performs a full update.
     *
     * @param host The SPI host the bus was initialized on (e.g. SPI2_HOST).
     * @param csPin The chip select (CS#) GPIO (required).
     * @param dcPin The data / command (D/C#) GPIO (required).
     * @param resetPin The reset (RES#) GPIO. Use GPIO_NUM_NC if RES# is not connected: a software reset is used instead.
     * @param busyPin The BUSY GPIO (required).
     * @param spiClockHz optional. The SPI clock speed in Hz. Defaults to CONFIG_SFE_SSD168X_SPI_CLOCK_HZ (4MHz).
     * @return true on success, false on startup failure.
     */
    bool begin(spi_host_device_t host, gpio_num_t csPin, gpio_num_t dcPin, gpio_num_t resetPin, gpio_num_t busyPin,
               uint32_t spiClockHz = SparkFun_SSD168x::kDefaultSpiClockHz)
    {
        if (!m_spiBus.init(host, csPin, dcPin, spiClockHz))
        {
            m_device.debugPrintln("begin: SPI init failed", true);
            return false;
        }

        return initDevice((int)resetPin, (int)busyPin);
    }

    /**
     * @brief Initialize the SSD168x library and the e-Paper display, using an SPI device already added to the bus by the user.
     *
     * The device must have been added with spics_io_num = -1 (chip select is driven by the library) and SPI mode 0.
     *
     * @param device The SPI device handle.
     * @param csPin The chip select (CS#) GPIO (required).
     * @param dcPin The data / command (D/C#) GPIO (required).
     * @param resetPin The reset (RES#) GPIO. Use GPIO_NUM_NC if RES# is not connected: a software reset is used instead.
     * @param busyPin The BUSY GPIO (required).
     * @return true on success, false on startup failure.
     */
    bool begin(spi_device_handle_t device, gpio_num_t csPin, gpio_num_t dcPin, gpio_num_t resetPin, gpio_num_t busyPin)
    {
        if (!m_spiBus.init(device, csPin, dcPin))
        {
            m_device.debugPrintln("begin: SPI init failed", true);
            return false;
        }

        return initDevice((int)resetPin, (int)busyPin);
    }

    /**
     * @brief Enable debug messages, sent to the given port.
     *
     * @param debugPort optional. The port to print debug messages to. Defaults to stdout (the console).
     * @param printLimitedDebug optional. If true, only print messages flagged as "important". Defaults to false.
     */
    void enableDebugging(sfe_print_t &debugPort = SparkFun_SSD168x::sfeStdout, bool printLimitedDebug = false)
    {
        m_device.enableDebugging(debugPort, printLimitedDebug);
    }

#endif

    /**
     * @brief Turn off debug messages.
     */
    void disableDebugging(void)
    {
        m_device.disableDebugging();
    }

    ///////////////////////////////////////////////////////////////////////
    // getWidth()

    /**
     * @brief Get the width, in pixels, of the connected e-Paper device.
     *
     * @return The width in pixels.
     */
    uint8_t getWidth(void)
    {
        return m_device.width();
    }

    ///////////////////////////////////////////////////////////////////////
    // getHeight()

    /**
     * @brief Get the height, in pixels, of the connected e-Paper device.
     *
     * @return The height in pixels.
     */
    uint8_t getHeight(void)
    {
        return m_device.height();
    }

    ///////////////////////////////////////////////////////////////////////
    // reset()

    /**
     * @brief Reset the system and the SSD168x back to an initial state.
     *
     * Clears the graphics buffer and the display RAM, and performs a full update.
     * The user must check isBusy() afterwards.
     *
     * @return true on success, false on failure.
     */
    bool reset(void)
    {
        return m_device.reset();
    }

    ///////////////////////////////////////////////////////////////////////
    // display()

    /**
     * @brief Send any pending display updates to the connected SSD168x device and update the display.
     *
     * This includes drawn graphics and erase commands. To display any graphics, this method must be called.
     *
     * display() will always reset (wake) the SSD168x, even if there is nothing
     * new to display. This avoids it staying in deep sleep and holding BUSY high.
     *
     * The update takes some time. Check isBusy() to see when it is complete.
     *
     * @param partial false (default) : full update, true : partial update
     * @param dirtyOnly true (default) : send dirty pixels only, false : send all pixels
     */
    void display(bool partial = false, bool dirtyOnly = true)
    {
        m_device.display(partial, dirtyOnly);
    }

    ///////////////////////////////////////////////////////////////////////
    // erase()

    /**
     * @brief Erase all graphics on the device, placing the display in a blank state.
     *
     * The erase update isn't sent to the device until the next display() call on the device.
     */
    void erase(void)
    {
        m_device.erase();
    }

    ///////////////////////////////////////////////////////////////////////
    // deepSleep()

    /**
     * @brief Put the SSD168x into deep sleep.
     *
     * Only a hardware reset can wake it again. display() does this automatically.
     *
     * @param mode2 false (default) : Deep Sleep Mode 1 (RAM retained), true : Deep Sleep Mode 2 (RAM not retained).
     */
    void deepSleep(bool mode2 = false)
    {
        m_device.deepSleep(mode2);
    }

    ///////////////////////////////////////////////////////////////////////
    // isBusy()

    /**
     * @brief Read the SSD168x BUSY pin.
     *
     * @return true if the SSD168x is busy (e.g. performing a display update).
     */
    bool isBusy(void)
    {
        return m_device.isBusy();
    }

    ///////////////////////////////////////////////////////////////////////
    // setFont()
    //
    // For the library, fonts are added to your program by including them via include files which
    // are part of this library.
    //
    // The following fonts are included:
    //
    //  Font            Include File               Font Variable          Description
    //  -----------     ---------------------      ----------------       ---------------------
    //  5x7             <res/qw_ep_fnt_5x7.h>      QW_EP_FONT_5X7         5 x 7 font, all 255 characters
    //  8x16            <res/qw_ep_fnt_8x16.h>     QW_EP_FONT_8X16        8 x 16 font, 96 characters from space to ~
    //  10x20           <res/qw_ep_fnt_10x20.h>    QW_EP_FONT_10X20       10 x 20 font, 96 characters from space to ~
    //  31x48           <res/qw_ep_fnt_31x48.h>    QW_EP_FONT_31X48       31 x 48 font, 58 characters from A to z
    //  Seven Segment   <res/qw_ep_fnt_7segment.h> QW_EP_FONT_7SEGMENT    13 characters: ./0123456789:
    //  Large Numbers   <res/qw_ep_fnt_largenum.h> QW_EP_FONT_LARGENUM    11 characters: 0123456789:
    //
    //  For each font, the font variables are objects with the following attributes:
    //
    //  Attribute   Value
    //  ----------  -------------------------------------
    //  width       The font width in pixels
    //  height      The font height in pixels
    //  start       The font start character offset
    //  n_chars     The number of characters
    //  map_width   The width of the font map
    //
    //  Example use of a font object attribute:
    //
    //     #include <res/qw_ep_fnt_31x48.h>
    //
    //     int myFontWidth = QW_EP_FONT_31X48.width;
    //

    /**
     * @brief Set the current font. The current font is used when calling the text() method on this device.
     *
     * The default font for the device is 5x7. See the comments above for the list of fonts.
     *
     * @param theFont The font to set as current in the device.
     */
    void setFont(QwiicEpFont &theFont)
    {
        m_device.setFont(theFont);
    }
    /**
     * @brief Set the current font. The current font is used when calling the text() method on this device.
     *
     * @param theFont A pointer to the font to set as current in the device.
     */
    void setFont(const QwiicEpFont *theFont)
    {
        m_device.setFont(theFont);
    }

    ///////////////////////////////////////////////////////////////////////
    // getFont()

    /**
     * @brief Get the current font for the device.
     *
     * @return A pointer to the current font. See setFont() for font object details.
     */
    QwiicEpFont *getFont(void)
    {
        return m_device.font();
    }

    ///////////////////////////////////////////////////////////////////////
    // getFontName()

    /**
     * @brief Get the name of the current font for the device.
     *
     * @return The font name (an Arduino String, or a std::string on ESP-IDF). Empty if no font is set.
     */
    sfe_string_t getFontName(void)
    {
        QwiicEpFont *pFont = m_device.font();

        if (!pFont)
            return sfe_string_t("");

        return sfe_string_t(pFont->name);
    }

    ///////////////////////////////////////////////////////////////////////
    // getStringWidth()

    /**
     * @brief Get the width of the provided string, using the current font.
     *
     * @param text The string used to determine the width.
     * @return The width of the provided string in pixels, as determined using the current font.
     */
    unsigned int getStringWidth(sfe_string_t &text)
    {
        return getStringWidth(text.c_str());
    }

    /**
     * @brief Get the width of the provided string, using the current font.
     *
     * @param text The null-terminated string used to determine the width.
     * @return The width of the provided string in pixels, as determined using the current font.
     */
    unsigned int getStringWidth(const char *text)
    {
        uint16_t height, width;

        return (m_device.getStringSize(text, width, height) ? width : 0);
    }

    ///////////////////////////////////////////////////////////////////////
    // getStringHeight()

    /**
     * @brief Get the height of the provided string, using the current font.
     *
     * @param text The string used to determine the height.
     * @return The height of the provided string in pixels, as determined using the current font.
     */
    unsigned int getStringHeight(sfe_string_t &text)
    {
        return getStringHeight(text.c_str());
    }

    /**
     * @brief Get the height of the provided string, using the current font.
     *
     * @param text The null-terminated string used to determine the height.
     * @return The height of the provided string in pixels, as determined using the current font.
     */
    unsigned int getStringHeight(const char *text)
    {
        uint16_t height, width;

        return (m_device.getStringSize(text, width, height) ? height : 0);
    }

    ///////////////////////////////////////////////////////////////////////
    // setDrawMode()
    //
    // Raster operations define how source (pixels to draw) are represented on the
    // destination device. The available Raster Operation (ROP) codes are:
    //
    //  ROP Code        Description
    //  ---------       -------------------------------------
    //  grEpROPCopy       default Drawn pixel values are copied to the device screen
    //  grEpROPNotCopy    A not operation is applied to the source value before copying to screen
    //  grEpROPNot        A not operation is applied to the destination (screen) value
    //  grEpROPXOR        A XOR operation is performed between the source and destination values
    //  grEpROPOff        A value of 0, or 'OFF' is drawn to the destination
    //  grEpROPOn         A value of 1, or 'ON' is drawn to the destination

    /**
     * @brief Set the current draw mode. The draw mode determines how pixels are set on the screen during drawing operations.
     *
     * @param rop The raster operation (ROP) to set the graphics system to. See the comments above for the available codes.
     */
    void setDrawMode(grEpRasterOp_t rop)
    {
        m_device.setRasterOp(rop);
    }

    ///////////////////////////////////////////////////////////////////////
    // getDrawMode()

    /**
     * @brief Get the current draw mode. The draw mode determines how pixels are set on the screen during drawing operations.
     *
     * @return The current raster operation (ROP) of the graphics system.
     */
    grEpRasterOp_t getDrawMode(void)
    {
        return m_device.rasterOp();
    }

    ///////////////////////////////////////////////////////////////////////
    // Drawing methods
    ///////////////////////////////////////////////////////////////////////
    //
    // pixel()

    /**
     * @brief Set the value of a pixel on the screen.
     *
     * @param x The X coordinate of the pixel to set.
     * @param y The Y coordinate of the pixel to set.
     * @param clr optional. The color value to set the pixel. Defaults to COLOR_ON (black).
     */
    void pixel(uint8_t x, uint8_t y, uint8_t clr = COLOR_ON)
    {
        m_device.pixel(x, y, clr);
    }

    ///////////////////////////////////////////////////////////////////////
    // line()

    /**
     * @brief Draw a line on the screen.
     *
     * If a line is horizontal (y0 = y1) or vertical (x0 = x1), optimized draw algorithms are used by the library.
     *
     * @param x0 The start X coordinate of the line.
     * @param y0 The start Y coordinate of the line.
     * @param x1 The end X coordinate of the line.
     * @param y1 The end Y coordinate of the line.
     * @param clr optional. The color value to draw the line. Defaults to COLOR_ON (black).
     */
    void line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t clr = COLOR_ON)
    {
        m_device.line(x0, y0, x1, y1, clr);
    }

    ///////////////////////////////////////////////////////////////////////
    // rectangle()

    /**
     * @brief Draw a rectangle on the screen.
     *
     * @param x0 The start X coordinate of the rectangle - upper left corner.
     * @param y0 The start Y coordinate of the rectangle - upper left corner.
     * @param width The width of the rectangle.
     * @param height The height of the rectangle.
     * @param clr optional. The color value to draw the rectangle. Defaults to COLOR_ON (black).
     */
    void rectangle(uint8_t x0, uint8_t y0, uint8_t width, uint8_t height, uint8_t clr = COLOR_ON)
    {
        m_device.rectangle(x0, y0, width, height, clr);
    }

    ///////////////////////////////////////////////////////////////////////
    // rectangleFill()

    /**
     * @brief Draw a filled rectangle on the screen.
     *
     * @param x0 The start X coordinate of the rectangle - upper left corner.
     * @param y0 The start Y coordinate of the rectangle - upper left corner.
     * @param width The width of the rectangle.
     * @param height The height of the rectangle.
     * @param clr optional. The color value to draw the filled rectangle. Defaults to COLOR_ON (black).
     */
    void rectangleFill(uint8_t x0, uint8_t y0, uint8_t width, uint8_t height, uint8_t clr = COLOR_ON)
    {
        m_device.rectangleFill(x0, y0, width, height, clr);
    }

    ///////////////////////////////////////////////////////////////////////
    // circle()

    /**
     * @brief Draw a circle on the screen.
     *
     * @param x0 The X coordinate of the circle center.
     * @param y0 The Y coordinate of the circle center.
     * @param radius The radius of the circle.
     * @param clr optional. The color value to draw the circle. Defaults to COLOR_ON (black).
     */
    void circle(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t clr = COLOR_ON)
    {
        m_device.circle(x0, y0, radius, clr);
    }

    ///////////////////////////////////////////////////////////////////////
    // circleFill()

    /**
     * @brief Draw a filled circle on the screen.
     *
     * @param x0 The X coordinate of the circle center.
     * @param y0 The Y coordinate of the circle center.
     * @param radius The radius of the circle.
     * @param clr optional. The color value to draw the filled circle. Defaults to COLOR_ON (black).
     */
    void circleFill(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t clr = COLOR_ON)
    {
        m_device.circleFill(x0, y0, radius, clr);
    }

    ///////////////////////////////////////////////////////////////////////
    // bitmap()

    /**
     * @brief Draw a bitmap on the screen.
     *
     * @param x0 The X coordinate to place the bitmap - upper left corner.
     * @param y0 The Y coordinate to place the bitmap - upper left corner.
     * @param x1 The end X coordinate of the area to draw - lower right corner. Range will not exceed the bitmap width.
     * @param y1 The end Y coordinate of the area to draw - lower right corner. Range will not exceed the bitmap height.
     * @param pBitmap A pointer to the bitmap array.
     * @param bmp_width The width of the bitmap.
     * @param bmp_height The height of the bitmap.
     */
    void bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t *pBitmap, uint8_t bmp_width, uint8_t bmp_height)
    {
        m_device.bitmap(x0, y0, x1, y1, pBitmap, bmp_width, bmp_height);
    }

    /**
     * @brief Draw a bitmap on the screen.
     *
     * @param x0 The X coordinate to place the bitmap - upper left corner.
     * @param y0 The Y coordinate to place the bitmap - upper left corner.
     * @param pBitmap A pointer to the bitmap array.
     * @param bmp_width The width of the bitmap.
     * @param bmp_height The height of the bitmap.
     */
    void bitmap(uint8_t x0, uint8_t y0, uint8_t *pBitmap, uint8_t bmp_width, uint8_t bmp_height)
    {
        m_device.bitmap(x0, y0, pBitmap, bmp_width, bmp_height);
    }

    /**
     * @brief Draw a bitmap object on the screen.
     *
     * @param x0 The X coordinate to place the bitmap - upper left corner.
     * @param y0 The Y coordinate to place the bitmap - upper left corner.
     * @param bitmap A bitmap object (e.g. QW_EP_BMP_SPARKFUN).
     */
    void bitmap(uint8_t x0, uint8_t y0, QwiicEpBitmap &bitmap)
    {
        m_device.bitmap(x0, y0, bitmap);
    }

    ///////////////////////////////////////////////////////////////////////
    // text()

    /**
     * @brief Draw a string on the screen using the current font.
     *
     * @param x0 The X coordinate to start drawing the text.
     * @param y0 The Y coordinate to start drawing the text.
     * @param text The null-terminated string to draw on the screen.
     * @param clr optional. The color value to draw the text. Defaults to COLOR_ON (black).
     */
    void text(uint8_t x0, uint8_t y0, const char *text, uint8_t clr = COLOR_ON)
    {
        m_device.text(x0, y0, text, clr);
    }

    /**
     * @brief Draw a string on the screen using the current font.
     *
     * @param x0 The X coordinate to start drawing the text.
     * @param y0 The Y coordinate to start drawing the text.
     * @param text The string (an Arduino String, or a std::string on ESP-IDF) to draw on the screen.
     * @param clr optional. The color value to draw the text. Defaults to COLOR_ON (black).
     */
    void text(uint8_t x0, uint8_t y0, sfe_string_t &text, uint8_t clr = COLOR_ON)
    {
        m_device.text(x0, y0, text.c_str(), clr);
    }

    ///////////////////////////////////////////////////////////////////////
    // Methods to support Arduino Print capability
    ///////////////////////////////////////////////////////////////////////
    //
    // setCursor()

    /**
     * @brief Set the "cursor" position in the device.
     *
     * The library supports the Arduino Print interface, enabling the use of the print() and
     * println() methods. The cursor position defines where to start text output for this functionality.
     *
     * @param x The X coordinate of the cursor.
     * @param y The Y coordinate of the cursor.
     */
    void setCursor(uint8_t x, uint8_t y)
    {
        // Note: x < 0 and y < 0 are also illegal but are not possible with uint8_t arguments
        if (x >= m_device.width() || y >= m_device.height())
            return;

        m_cursorX = x;
        m_cursorY = y;
    }

    ///////////////////////////////////////////////////////////////////////
    // setColor()

    /**
     * @brief Set the current color of the system. This is used by the Arduino Print interface functionality.
     *
     * @param clr The color to set. COLOR_ON ('ON') is e-paper BLACK (due to inversion); COLOR_OFF ('OFF') is e-paper WHITE.
     */
    void setColor(uint8_t clr)
    {
        m_color = (clr == COLOR_OFF ? COLOR_OFF : COLOR_ON);
    }

    ///////////////////////////////////////////////////////////////////////
    // getColor()

    /**
     * @brief Get the current color of the system. This is used by the Arduino Print interface functionality.
     *
     * @return The current color. COLOR_OFF (0) = white, COLOR_ON (1) = black.
     */
    uint8_t getColor(void)
    {
        return m_color;
    }

    ///////////////////////////////////////////////////////////////////////
    // write()

    /**
     * @brief Write one character at the cursor position, using the current font and color. For the Arduino Print interface.
     *
     * '\\n' moves the cursor to the start of the next line. '\\r' is ignored.
     *
     * @param theChar The character to write.
     * @return 1 on success, 0 if there is no font.
     */
    virtual size_t write(uint8_t theChar)
    {
        QwiicEpFont *pFont = m_device.font();

        if (!pFont) // no Font?! No dice
            return 0;

        switch (theChar)
        {
        case '\n': // Line feed - move to the start of the next line
            m_cursorX = 0;
            m_cursorY += pFont->height;
            break;
        case '\r': // Carriage return - do nothing
            break;
        default:

            char buffer[2] = {(char)theChar, '\0'}; // text() needs a c string
            m_device.text(m_cursorX, m_cursorY, buffer, m_color);

            m_cursorX += pFont->width + 1;

            if (m_cursorX > m_device.width() - pFont->width)
            { // overflow
                m_cursorX = 0;
                m_cursorY += pFont->height;
            }
            break;
        }
        if (m_cursorY >= m_device.height()) // check for overflow
            m_cursorY = 0;

        return 1;
    }

#if defined(SFE_SSD168X_ARDUINO)
    using Print::write; // Also allow write(const uint8_t *buffer, size_t size) etc.
#endif
};

///////////////////////////////////////////////////////////////////////
// For our actual implementations - just subclass from the above template

/**
 * @brief A 184x88 SSD1680 e-Paper display (e.g. GoodDisplay GDEM0097T61) - portrait: 88 pixels wide, 184 pixels high.
 */
class SSD1680SPI184x88 : public SSD168xSPIBaseClass<SpiSsd1680_184x88>
{
    // nothing here - see above
};

/**
 * @brief A 200x200 SSD1681 e-Paper display (e.g. GoodDisplay GDEY0154D67).
 */
class SSD1681SPI200x200 : public SSD168xSPIBaseClass<SpiSsd1681_200x200>
{
    // nothing here - see above
};

/**
 * @brief A custom-sized SSD1680 e-Paper display. Call the set methods before begin().
 */
class SSD1680SPICustom : public SSD168xSPIBaseClass<SpiSsd1680_Custom>
{
  public:
    /** @brief Set the X offset. Call before begin(). @param xOffset The X offset in pixels. */
    void setXOffset(uint8_t xOffset)
    {
        m_device.setXOffset(xOffset);
    }
    /** @brief Set the Y offset. Call before begin(). @param yOffset The Y offset in pixels. */
    void setYOffset(uint8_t yOffset)
    {
        m_device.setYOffset(yOffset);
    }
    /** @brief Set the display width. Call before begin(). @param displayWidth The width in pixels. Must be a multiple of 8. */
    void setDisplayWidth(uint8_t displayWidth)
    {
        m_device.setDisplayWidth(displayWidth);
    }
    /** @brief Set the display height. Call before begin(). @param displayHeight The height in pixels. */
    void setDisplayHeight(uint8_t displayHeight)
    {
        m_device.setDisplayHeight(displayHeight);
    }
};

/**
 * @brief A custom-sized SSD1681 e-Paper display. Call the set methods before begin().
 */
class SSD1681SPICustom : public SSD168xSPIBaseClass<SpiSsd1681_Custom>
{
  public:
    /** @brief Set the X offset. Call before begin(). @param xOffset The X offset in pixels. */
    void setXOffset(uint8_t xOffset)
    {
        m_device.setXOffset(xOffset);
    }
    /** @brief Set the Y offset. Call before begin(). @param yOffset The Y offset in pixels. */
    void setYOffset(uint8_t yOffset)
    {
        m_device.setYOffset(yOffset);
    }
    /** @brief Set the display width. Call before begin(). @param displayWidth The width in pixels. Must be a multiple of 8. */
    void setDisplayWidth(uint8_t displayWidth)
    {
        m_device.setDisplayWidth(displayWidth);
    }
    /** @brief Set the display height. Call before begin(). @param displayHeight The height in pixels. */
    void setDisplayHeight(uint8_t displayHeight)
    {
        m_device.setDisplayHeight(displayHeight);
    }
};

/**
 * @brief A 184x88 SSD1680 e-Paper display (e.g. GoodDisplay GDEM0097T61) - rotated to landscape: 184 pixels wide, 88 pixels high.
 */
class SSD1680SPI184x88Rotated : public SSD168xSPIBaseClass<SpiSsd1680_184x88_Rotated>
{
    // nothing here - see above
};
