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
 * @file qw_ep_fnt_7segment.h
 */

#pragma once

#include "qwiic_resdef.h"

class QwEpFont7Segment final : public fontEpSingleton<QwEpFont7Segment> {

public:
    const uint8_t* data(void)
    {
        // include font data (static const), and attribute defines.
        // Doing this here makes the data variable a static (aka only one instance ever)
        // variable in this method.
#include "_fnt_7segment.h"

        return segment7_data;
    }

    QwEpFont7Segment()
        : fontEpSingleton<QwEpFont7Segment>(FONT_7SEG_WIDTH,
            FONT_7SEG_HEIGHT,
            FONT_7SEG_START,
            FONT_7SEG_NCHAR,
            FONT_7SEG_MAP_WIDTH,
            FONT_7SEG_NAME)
    {
    }
};

#define QW_EP_FONT_7SEGMENT QwEpFont7Segment::instance()