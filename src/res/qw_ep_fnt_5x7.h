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
 * @file qw_ep_fnt_5x7.h
 */

#pragma once

#include "qwiic_resdef.h"

class QwEpFont5x7 final : public fontEpSingleton<QwEpFont5x7> {

public:
    const uint8_t* data(void)
    {
        // include font data (static const), and attribute defines.
        // Doing this here makes the data variable a static (aka only one instance ever)
        // variable in this method.
#include "_fnt_5x7.h"

        return font5x7_data;
    }

    QwEpFont5x7()
        : fontEpSingleton<QwEpFont5x7>(FONT_5X7_WIDTH,
            FONT_5X7_HEIGHT,
            FONT_5X7_START,
            FONT_5X7_NCHAR,
            FONT_5X7_MAP_WIDTH,
            FONT_5X7_NAME)
    {
    }
};

#define QW_EP_FONT_5X7 QwEpFont5x7::instance()