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
 * @file qw_ep_fnt_8x16.h
 */

#pragma once

#include "qwiic_resdef.h"

class QwEpFont8x16 final : public fontEpSingleton<QwEpFont8x16> {

public:
    const uint8_t* data(void)
    {
        // include font data (static const), and attribute defines.
        // Doing this here makes the data variable a static (aka only one instance ever)
        // variable in this method.
#include "_fnt_8x16.h"

        return font8x16_data;
    }

    QwEpFont8x16()
        : fontEpSingleton<QwEpFont8x16>(FONT_8X16_WIDTH,
            FONT_8X16_HEIGHT,
            FONT_8X16_START,
            FONT_8X16_NCHAR,
            FONT_8X16_MAP_WIDTH,
            FONT_8X16_NAME)
    {
    }
};

#define QW_EP_FONT_8X16 QwEpFont8x16::instance()