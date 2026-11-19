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
 * @file qwiic_grcommon.h
 */

// Common defines for multiple devices
// TODO: Probably should rework how this looks eventually...

/////////////////////////////////////////////////////////////////////////////
// The graphics Raster Operator functions (ROPS)
/////////////////////////////////////////////////////////////////////////////
//      - Copy      - copy the pixel value into the buffer (default)
//      - Not Copy  - copy the not of the pixel value to buffer
//      - Not       - Set the buffer value to not its current value
//      - XOR       - XOR of color and current pixel value
//      - Off       - Set value to always be Off
//      - On        - set value to always be On

#pragma once

#include <stdint.h>

typedef enum gr_ep_op_funcs_
{
    grEpROPCopy = 0,
    grEpROPNotCopy = 1,
    grEpROPNot = 2,
    grEpROPXOR = 3,
    grEpROPOff = 4,
    grEpROPOn = 5
} grEpRasterOp_t;
