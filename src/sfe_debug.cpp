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
 * @file sfe_debug.cpp
 *
 * Definitions for SparkFun_SSD168x::SfeDebugPrint - see sfe_debug.h.
 * These seven methods are the same as those in the SparkFun u-blox GNSS v4 library.
 */

#include "sfe_debug.h"

using namespace SparkFun_SSD168x;

// Safely print messages
void SfeDebugPrint::debugPrint(const char *message, bool important)
{
  if (_printDebug == true) // if _printDebug is true, prepare to print
    // Don't print if _printLimitedDebug is true and this message is not important
    if (!(_printLimitedDebug && !important))
      _debugSerial.print(message);
}
// Safely print messages
void SfeDebugPrint::debugPrintln(const char *message, bool important)
{
  if (_printDebug == true) // if _printDebug is true, prepare to print
    // Don't print if _printLimitedDebug is true and this message is not important
    if (!(_printLimitedDebug && !important))
      _debugSerial.println(message);
}
// Safely print debug values
void SfeDebugPrint::debugPrint(uint32_t value, bool important)
{
  if (_printDebug == true) // if _printDebug is true, prepare to print
    // Don't print if _printLimitedDebug is true and this message is not important
    if (!(_printLimitedDebug && !important))
      _debugSerial.print(value);
}
// Safely print debug values in a given base (e.g. HEX)
void SfeDebugPrint::debugPrint(uint32_t value, int printBase, bool important)
{
  if (_printDebug == true) // if _printDebug is true, prepare to print
    // Don't print if _printLimitedDebug is true and this message is not important
    if (!(_printLimitedDebug && !important))
      _debugSerial.print(value, printBase);
}
// Safely print debug values
void SfeDebugPrint::debugPrintln(uint32_t value, bool important)
{
  if (_printDebug == true) // if _printDebug is true, prepare to print
    // Don't print if _printLimitedDebug is true and this message is not important
    if (!(_printLimitedDebug && !important))
      _debugSerial.println(value);
}
// Safely print debug values in a given base (e.g. HEX)
void SfeDebugPrint::debugPrintln(uint32_t value, int printBase, bool important)
{
  if (_printDebug == true) // if _printDebug is true, prepare to print
    // Don't print if _printLimitedDebug is true and this message is not important
    if (!(_printLimitedDebug && !important))
      _debugSerial.println(value, printBase);
}
// Safely print a blank debug line
void SfeDebugPrint::debugPrintln(void)
{
  if (_printDebug == true) // if _printDebug is true, prepare to print
    // Not important - don't print if doing limited debugging
    if (!_printLimitedDebug)
      _debugSerial.println();
}
