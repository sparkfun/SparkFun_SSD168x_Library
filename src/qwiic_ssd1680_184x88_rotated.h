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
 * @file qwiic_ssd1680_184x88_rotated.h
 *
 * Implementation for a generic 184x88 SSD1680 display, rotated through 90 degrees.
 */

#pragma once

#include "spi_ssd1680_rotated.h"

//////////////////////////////////////////////////////////////////
// Set the defaults for a generic 184x88 SSD1680 display, rotated through 90 degrees

#define kSpiSsd1680184x88RotatedWidth 184
#define kSpiSsd1680184x88RotatedHeight 88

#define kSpiSsd1680184x88RotatedXOffset 0
#define kSpiSsd1680184x88RotatedYOffset 0

class SpiSsd1680_184x88_Rotated : public SpiSsd1680Rotated {

public:
    /**
     * @brief Construct the device - setup the viewport for this device.
     */
    SpiSsd1680_184x88_Rotated()
        : SpiSsd1680Rotated(kSpiSsd1680184x88RotatedXOffset, kSpiSsd1680184x88RotatedYOffset, kSpiSsd1680184x88RotatedWidth, kSpiSsd1680184x88RotatedHeight)
    {
    };

    /**
     * @brief Destroy the device - free the graphics buffer.
     */
    ~SpiSsd1680_184x88_Rotated()
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
        m_graphicsBuffer = new uint8_t[(uint16_t)kSpiSsd1680184x88RotatedWidth * (uint16_t)kSpiSsd1680184x88RotatedHeight / 8];
        this->SpiSsd1680Rotated::setBuffer(m_graphicsBuffer); // The buffer to use

        // Call the super class to do all the work
        return this->SpiSsd1680Rotated::init();
    };

private:
    // Graphics buffer for this device.
    uint8_t *m_graphicsBuffer = nullptr;
};
