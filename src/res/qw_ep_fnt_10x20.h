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
 * @file qw_ep_fnt_10x20.h
 */

#pragma once

#include "qwiic_resdef.h"

class QwEpFont10x20 final : public fontEpSingleton<QwEpFont10x20> {

public:
    const uint8_t* data(void)
    {
        // include font data (static const), and attribute defines.
        // Doing this here makes the data variable a static (aka only one instance ever)
        // variable in this method.
#include "_fnt_10x20.h"

        return font10x20_data;
    }

    QwEpFont10x20()
        : fontEpSingleton<QwEpFont10x20>(FONT_10x20_WIDTH,
            FONT_10x20_HEIGHT,
            FONT_10x20_START,
            FONT_10x20_NCHAR,
            FONT_10x20_MAP_WIDTH,
            FONT_10x20_NAME)
    {
    }
};

#define QW_EP_FONT_10X20 QwEpFont10x20::instance()