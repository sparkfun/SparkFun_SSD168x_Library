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
 * @file spi_ssd1681.h
 *
 * Header file for the SSD1681 bitmap graphics driver device.
 */

#pragma once

#include "spi_ssd1681_defs.h"
#include "qwiic_grbuffer.h"
#include "sfe_platform.h"
#include "sfe_bus.h"
#include "sfe_debug.h"
#include "res/qwiic_resdef.h"
#include "qwiic_grcommon.h"

/////////////////////////////////////////////////////////////////////////////
// Buffer Management
/////////////////////////////////////////////////////////////////////////////
//
// SSD1681 supports up to 200 rows and 200 columns
// Each memory byte contains the pixels for 8 columns
// (For the SSD1306: each byte contains the pixels for 8 rows / COM lines)
// The MSB is the left-most pixel
// By default: white pixels are '1', black are '0', but UpdateCtrl1 can invert this
// Because this library is hard-wired to expect 'off' pixels to be '0' and 'on' pixels to be '1',
// it is MUCH easier if we use the inversion...
// On SSD1681, the 25 'pages' run vertically, not horizontally
// By default: addresses increase in the X direction: byte 0 is row 0 cols 0-7, byte 1 is row 0 cols 8-15, etc.
// But Data Entry Mode can select Y-increase instead: byte 0 is row 0 cols 0-7, byte 1 is row 1 cols 0-7
// Basically, the memory is rotated and flipped compared to the SSD1306, and the pixels are inverted too...
//
// The 'pages' are arranged like this:
//
//  _  _  _  _  _  _  _
// | || || || || || || | < Row 0
// | || || || || || || | < Row 1
// | || || || || || || | < Row 2
// | || || || || || || |
// | || || || || || || |
// | || || || || || || |
// |_||_||_||_||_||_||_|
//      | FPC |
//
//        ^- Page 2
//     ^- Page 1
//  ^- Page 0
//
// The memory/back buffer of the SSD1681 is based on the concept of pages -
// each page is a stream of bytes, and - with addresses increasing in Y - is defined as follows:
//
//      - X pixel position is a bit in a byte, so a page can have 8 X locations
//      - Y pixel position is an offset in a byte array
//
// With inversion, a pixel value of 0 is 'off' (white), 1 is 'on' (black)
//
// This implementation uses the Page mode for buffer transfer. This is defined by:
//     - A start position is set - a page number and row in that page.
//     - As data is transferred, it is written to the screenbuffer, based on this start
//       position
//     - If the end of the page is reached, the next entry location is the start of the next page
//
// >> Implementation <<
//
// This implementation uses the concept of "dirty rects" at the page level to minimize data
// transfers to the device. The min and max Y locations set for each page is recorded as
// graphics are drawn to the graphics buffer. When transferring the display buffer to
// the devices screen buffer, the following takes place:
//
//      For each page:
//          - if page is dirty
//              - Set the screen buffer current location to this page, min dirty value
//              - Write buffer bytes to the device - starting at min for the page, ending at max
//              - Mark the buffer as "clean"
//
//

/////////////////////////////////////////////////////////////////////////////
// SpiSsd1681
// A buffer graphics device to support the SSD1681 graphics hardware

class SpiSsd1681 : public QwEpGrBufferDevice, public SparkFun_SSD168x::SfeDebugPrint
{
  private:
    void setupDefaults(void);

  public:
    SpiSsd1681()
    {
        setupDefaults(); // default constructor - always called
    }
    SpiSsd1681(uint8_t width, uint8_t height) : SpiSsd1681(0, 0, width, height){};

    // call super class
    SpiSsd1681(uint8_t x0, uint8_t y0, uint8_t width, uint8_t height) : QwEpGrBufferDevice(x0, y0, width, height)
    {
        setupDefaults();
    };

    // Public draw methods
    /**
     * @brief Send the changes in the graphics buffer to the device and perform a full or partial update.
     *
     * @param partial false (default) : full update, true : partial update
     * @param dirtyOnly true (default) : send only the dirty pixels, false : send all pixels
     */
    void display(bool partial = false, bool dirtyOnly = true);
    /**
     * @brief Erase the graphics buffer. The erase is not sent to the device until the next display().
     */
    void erase(void);

    // Device setup
    /**
     * @brief Initialize the device: reset it, send the initialization sequence, clear it and perform a full update.
     *
     * setCommBus() and setBuffer() must have been called first.
     *
     * @return true on success, false if the bus / pins / buffer are not set or the BUSY pin timed out.
     */
    virtual bool init(void);

    /**
     * @brief Check if init() has been called successfully.
     *
     * @return true if initialized.
     */
    bool isInitialized(void)
    {
        return m_isInitialized;
    };
    /**
     * @brief Wake and reset the device: clear the graphics and screen buffers and perform a full update.
     *
     * @return true on success, false on failure.
     */
    bool reset(void);

    /**
     * @brief Set the communication bus this object should use, plus the RES# and BUSY pins.
     *
     * @param theBus The SPI bus object (SparkFun_SSD168x::SfeSPI), already initialized with the CS# and D/C# pins.
     * @param resetPin The RES# (reset) pin. Can be -1 if not connected: a software reset is used instead.
     * @param busyPin The BUSY pin (required).
     */
    void setCommBus(SparkFun_SSD168x::SSD168xDeviceBus &theBus, int resetPin, int busyPin);

    // Set the current color/pixel write operation
    void setColor(uint8_t color);

    /**
     * @brief Set the current raster operation (draw mode).
     *
     * @param rop The raster operation.
     */
    void setRasterOp(grEpRasterOp_t rop)
    {
        m_rop = rop;
    }

    /**
     * @brief Get the current raster operation (draw mode).
     *
     * @return The current raster operation.
     */
    grEpRasterOp_t rasterOp(void)
    {
        return m_rop;
    }

    /**
     * @brief Put the SSD168x into deep sleep. Only a hardware reset can wake it again (display() does this).
     *
     * @param mode2 false : Deep Sleep Mode 1 (RAM retained), true : Deep Sleep Mode 2 (RAM not retained).
     */
    void deepSleep(bool mode2);

    /**
     * @brief Read the state of the BUSY pin.
     *
     * @return true if the SSD168x is busy (BUSY is high). Always false if not initialized.
     */
    bool isBusy(void);

    /**
     * @brief Wait until the SSD168x BUSY pin goes low.
     *
     * @param pollInterval_ms How often to check the BUSY pin (ms). Defaults to 10ms.
     * @param timeout_ms The maximum time to wait (ms). Defaults to kSsd168xBusyTimeout_ms.
     * @return true when BUSY is low, false on timeout.
     */
    bool waitWhileBusy(unsigned long pollInterval_ms = 10, unsigned long timeout_ms = kSsd168xBusyTimeout_ms);

  protected:
    // Subclasses of this class define the specifics of the device, including size.
    // Subclass needs to define the graphics buffer array - stack based - and pass in
    void setBuffer(uint8_t *pBuffer);

    ///////////////////////////////////////////////////////////////////////////
    // Internal, fast draw routines - this are used in the overall
    // draw interface (_QwEpIDraw) for this object/device/system.
    //
    // >> Pixels <<
    void drawPixel(uint8_t x, uint8_t y, uint8_t clr);

    // >> Fast Lines <<
    void drawLineHorz(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t clr);
    void drawLineVert(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t clr);

    // fast rect fill
    void drawRectFilled(uint8_t x0, uint8_t y0, uint8_t width, uint8_t height, uint8_t clr);

    // >> Fast Bitmap <<
    void drawBitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t *pBitmap, uint8_t bmp_width,
                    uint8_t bmp_height);

    ///////////////////////////////////////////////////////////////////////////
    // configuration methods for sub-classes. Settings unique to a device

  private:
    // Internal buffer management methods
    bool setScreenBufferAddress(uint8_t page, uint8_t rowStart, uint8_t rowEnd);
    void initBuffers(void); // clear graphics and screen buffer
    void clearScreenBuffer(void); // clear screen buffer
    void setupEpaperDevice(void); // always calls initBuffers

    // device communication methods
    void sendDevCommand(uint8_t command);
    void sendDevCommand(uint8_t command, uint8_t value);
    void sendDevCommand(uint8_t command, uint8_t *values, uint16_t n_values);
    void sendDevReset(void);

    /////////////////////////////////////////////////////////////////////////////
    // instance vars

    // Buffer variables
    uint8_t *m_pBuffer;                      // Pointer to the graphics buffer
    uint8_t m_nPages;                        // number of pages for current device
    // m_pageState
    // The current "dirty" areas of the graphics [local] buffer.
    // Areas that haven't been sent to the screen/device but are
    // "dirty" (have been changed)
    pageStateEp_t m_pageState[kMaxPageNumberSSD1681]; // page state descriptors
    // m_pageErase
    // erase() erases all graphics on the device, placing the display in a blank state.
    // ** The erase update isn't sent to the device until the next display() call
    // on the device. **
    // m_pageErase holds the area of each page which need to be erased on the next
    // call of display()
    pageStateEp_t m_pageErase[kMaxPageNumberSSD1681]; // keep track of erase boundaries
    // m_pendingErase
    // Flags to indicate which of the screen pages need to be erased.
    // In the original OLED library, this is a single flag which is set
    // when erase() is called and cleared by the next display().
    // In this library, m_pendingErase has been expanded to provide
    // a flag for each screen bank / page.
    bool m_pendingErase[kMaxPageNumberSSD1681]; // indicates if this page needs erase
    // The GoodDisplay e-paper displays have ping-pong mode enabled.
    // When using partial updates, we need to ensure display() always writes
    // the current "dirty" area(s) _and_ the previous "dirty" area(s) to the
    // screen. (Otherwise the two SSD168x RAM banks get out of sync and we
    // get 'flashing' / 'blinking' areas of pixels.)
    // Thinking through how we need to handle erase():
    // display() needs to send:
    //   the current "dirty" area stored in m_pageState
    //   expanded to include:
    //     the current erased area stored in m_pageErase (if flagged)
    //     the previous combined "dirty" area in m_pagePrevious
    pageStateEp_t m_pagePrevious[kMaxPageNumberSSD1681]; // previous page state descriptors

    // display variables
    uint8_t m_color;    // current color (really 0 or 1)
    grEpRasterOp_t m_rop; // current raster operation code

    // SPI things
    SparkFun_SSD168x::SSD168xDeviceBus *m_spiBus; // pointer to our SPI bus object
    int m_pinReset;                               // the RES# pin. -1 if not connected
    int m_pinBusy;                                // the BUSY pin

    // Stash values for settings that are unique to each device.

    bool m_isInitialized; // general init flag

    // Handy helper
    const uint8_t gfx_byte_bits[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
};
