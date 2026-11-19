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
 * @file qwiic_ssd1681_200x200.h
 *
 * Implementation for a generic 200x200 SSD1681 display.
 */

#pragma once

#include "spi_ssd1681.h"

//////////////////////////////////////////////////////////////////
// Set the defaults for a generic 200x200 SSD1681 display

#define kSpiSsd1681200x200Width 200
#define kSpiSsd1681200x200Height 200

#define kSpiSsd1681200x200XOffset 0
#define kSpiSsd1681200x200YOffset 0

class SpiSsd1681_200x200 : public SpiSsd1681 {

public:
    /**
     * @brief Construct the device - setup the viewport for this device.
     */
    SpiSsd1681_200x200()
        : SpiSsd1681(kSpiSsd1681200x200XOffset, kSpiSsd1681200x200YOffset, kSpiSsd1681200x200Width, kSpiSsd1681200x200Height)
    {
    };

    /**
     * @brief Destroy the device - free the graphics buffer.
     */
    ~SpiSsd1681_200x200()
    {
        if (m_graphicsBuffer != nullptr)
        {
            delete[] m_graphicsBuffer;
            m_graphicsBuffer = nullptr;
        }
    };

    /**
     * @brief Set up the specific device settings: allocate the graphics buffer, then initialize the device.
     *
     * @return true on success, false on failure.
     */
    bool init(void)
    {
        if (m_graphicsBuffer != nullptr)
            delete[] m_graphicsBuffer;
        m_graphicsBuffer = new uint8_t[(uint16_t)kSpiSsd1681200x200Width * (uint16_t)kSpiSsd1681200x200Height / 8];
        this->SpiSsd1681::setBuffer(m_graphicsBuffer); // The buffer to use

        // Call the super class to do all the work
        return this->SpiSsd1681::init();
    };

private:
    // Graphics buffer for this device.
    uint8_t *m_graphicsBuffer = nullptr;
};
