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
 * @file qwiic_ssd1681_custom.h
 *
 * Implementation for a custom-sized SSD1681 display.
 */

#pragma once

#include "spi_ssd1681.h"

//////////////////////////////////////////////////////////////////
// Set the defaults for a custom-sized SSD1681 display
// The width, height and offsets can be changed with the set methods - before calling init()

#define kSpiSsd1681CustomDefaultWidth 200
#define kSpiSsd1681CustomDefaultHeight 200

#define kSpiSsd1681CustomDefaultXOffset 0
#define kSpiSsd1681CustomDefaultYOffset 0

class SpiSsd1681_Custom : public SpiSsd1681 {

public:
    /**
     * @brief Construct the device - setup the default viewport for this device.
     */
    SpiSsd1681_Custom()
        : SpiSsd1681(kSpiSsd1681CustomDefaultXOffset, kSpiSsd1681CustomDefaultYOffset, kSpiSsd1681CustomDefaultWidth, kSpiSsd1681CustomDefaultHeight)
    {
    };

    /**
     * @brief Destroy the device - free the graphics buffer.
     */
    ~SpiSsd1681_Custom()
    {
        if (m_graphicsBuffer != nullptr)
        {
            delete[] m_graphicsBuffer;
            m_graphicsBuffer = nullptr;
        }
    };

    /**
     * @brief Set up the specific device settings: set the viewport, allocate the graphics buffer, then initialize the device.
     *
     * @return true on success, false on failure.
     */
    bool init(void)
    {
        this->SpiSsd1681::setViewport(m_xOffset, m_yOffset, m_displayWidth, m_displayHeight);

        if (m_graphicsBuffer != nullptr)
            delete[] m_graphicsBuffer;
        m_graphicsBuffer = new uint8_t[(uint16_t)m_displayWidth * (uint16_t)m_displayHeight / 8];
        this->SpiSsd1681::setBuffer(m_graphicsBuffer); // The buffer to use

        // Call the super class to do all the work
        return this->SpiSsd1681::init();
    };

    /** @brief Set the X offset. Call before init(). @param xOffset The X offset in pixels. */
    void setXOffset(uint8_t xOffset){ m_xOffset = xOffset; }
    /** @brief Set the Y offset. Call before init(). @param yOffset The Y offset in pixels. */
    void setYOffset(uint8_t yOffset){ m_yOffset = yOffset; }
    /** @brief Set the display width. Call before init(). @param displayWidth The width in pixels. Must be a multiple of 8. */
    void setDisplayWidth(uint8_t displayWidth){ m_displayWidth = displayWidth; }
    /** @brief Set the display height. Call before init(). @param displayHeight The height in pixels. */
    void setDisplayHeight(uint8_t displayHeight){ m_displayHeight = displayHeight; }

private:
    uint8_t m_xOffset = kSpiSsd1681CustomDefaultXOffset;
    uint8_t m_yOffset = kSpiSsd1681CustomDefaultYOffset;
    uint8_t m_displayWidth = kSpiSsd1681CustomDefaultWidth;
    uint8_t m_displayHeight = kSpiSsd1681CustomDefaultHeight;

    // Graphics buffer for this device.
    uint8_t *m_graphicsBuffer = nullptr;
};
