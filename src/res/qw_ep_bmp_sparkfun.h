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
 * @file qw_ep_bmp_sparkfun.h
 */

#pragma once

#include "qwiic_resdef.h"

class QwEpBMPSparkFun final : public bmpEpSingleton<QwEpBMPSparkFun> {

public:
    const uint8_t* data(void)
    {

#include "_bmp_sparkfun.h"

        return bmp_sparkfun_data;
    }

    QwEpBMPSparkFun()
        : bmpEpSingleton<QwEpBMPSparkFun>(BMP_SPARKFUN_WIDTH, BMP_SPARKFUN_HEIGHT)
    {
    }
};

#define QW_EP_BMP_SPARKFUN QwEpBMPSparkFun::instance()