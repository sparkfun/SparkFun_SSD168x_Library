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
 * @file qw_ep_fnt_31x48.h
 */

#pragma once

#include "qwiic_resdef.h"

class QwEpFont31x48 final : public fontEpSingleton<QwEpFont31x48> {

public:
    const uint8_t* data(void)
    {
        // include font data (static const), and attribute defines.
        // Doing this here makes the data variable a static (aka only one instance ever)
        // variable in this method.
#include "_fnt_31x48.h"

        return font31x48_data;
    }

    QwEpFont31x48()
        : fontEpSingleton<QwEpFont31x48>(FONT_31X48_WIDTH,
            FONT_31X48_HEIGHT,
            FONT_31X48_START,
            FONT_31X48_NCHAR,
            FONT_31X48_MAP_WIDTH,
            FONT_31X48_NAME)
    {
    }
};

#define QW_EP_FONT_31X48 QwEpFont31x48::instance()