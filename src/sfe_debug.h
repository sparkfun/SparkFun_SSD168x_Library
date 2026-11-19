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
 * @file sfe_debug.h
 */

/*
  debugPrint()/debugPrintln() live in their own small base class, SfeDebugPrint, so that any class
  can inherit real, working debug-print support. This is the same class as used in the SparkFun
  u-blox GNSS v4 library.

  The SSD168x device classes (SpiSsd1680, SpiSsd1680Rotated, SpiSsd1681) inherit SfeDebugPrint.
  The user-facing classes (SSD1680SPI184x88 etc.) own the device class BY VALUE (m_device) and
  provide enableDebugging() / disableDebugging(), which select the default debug port for the
  platform and pass it on to m_device.

  Plain (non-static) inheritance means each object gets its OWN copy of _debugSerial/_printDebug/
  _printLimitedDebug. So debug can be enabled for one display and not another. copyDebugStateFrom()
  allows the state to be pushed from one object to another if needed.
*/

#pragma once

#include "sfe_platform.h"
#include "sfe_bus.h"

namespace SparkFun_SSD168x
{

  class SfeDebugPrint
  {
  public:
    /**
     * @brief Enable debug messages, sent to the given port.
     *
     * @param debugPort The port to print debug messages to (Arduino: a Print, e.g. Serial.
     *                  ESP-IDF: an SfeOutput, e.g. SparkFun_SSD168x::sfeStdout).
     * @param printLimitedDebug If true, only print messages flagged as "important". Defaults to false (print everything).
     */
    void enableDebugging(sfe_print_t &debugPort, bool printLimitedDebug = false)
    {
      _debugSerial.init(debugPort); // Grab which port the user wants us to use for debugging
      _printDebug = true;           // Should we print the debug messages?
      _printLimitedDebug = printLimitedDebug;
    }

    /**
     * @brief Turn off debug message printing.
     */
    void disableDebugging(void)
    {
      _printDebug = false;
      _printLimitedDebug = false;
    }

    /**
     * @brief Print a debug message (no trailing newline), if debugging is enabled.
     *
     * A no-op unless _printDebug is true; further suppressed if _printLimitedDebug is true and
     * 'important' is false - see enableDebugging().
     *
     * @param message Null-terminated string to print.
     * @param important If true, print even when limited debugging is active. Defaults to false.
     */
    void debugPrint(const char *message, bool important = false);             // Safely print debug statements
    /**
     * @brief Print a debug value in decimal (no trailing newline), if debugging is enabled.
     *
     * @param value The value to print.
     * @param important If true, print even when limited debugging is active. Defaults to false.
     */
    void debugPrint(uint32_t value, bool important = false);                  // Safely print debug values
    /**
     * @brief Print a debug value in a given base (no trailing newline), if debugging is enabled.
     *
     * @param value The value to print.
     * @param printBase Number base to print in (e.g. HEX, DEC).
     * @param important If true, print even when limited debugging is active. Defaults to false.
     */
    void debugPrint(uint32_t value, int printBase, bool important = false);   // Safely print debug values in a given base (e.g. HEX)
    /**
     * @brief Print a debug message followed by a newline, if debugging is enabled.
     *
     * @param message Null-terminated string to print.
     * @param important If true, print even when limited debugging is active. Defaults to false.
     */
    void debugPrintln(const char *message, bool important = false);           // Safely print debug statements
    /**
     * @brief Print a debug value in decimal followed by a newline, if debugging is enabled.
     *
     * @param value The value to print.
     * @param important If true, print even when limited debugging is active. Defaults to false.
     */
    void debugPrintln(uint32_t value, bool important = false);                // Safely print debug values
    /**
     * @brief Print a debug value in a given base followed by a newline, if debugging is enabled.
     *
     * @param value The value to print.
     * @param printBase Number base to print in (e.g. HEX, DEC).
     * @param important If true, print even when limited debugging is active. Defaults to false.
     */
    void debugPrintln(uint32_t value, int printBase, bool important = false); // Safely print debug values in a given base (e.g. HEX)
    /**
     * @brief Print a blank debug line, if debugging is enabled and not in limited-debug mode.
     */
    void debugPrintln(void);                                                  // Safely print a blank debug line

    /**
     * @brief Copy debug state (output stream and enabled flags) from another SfeDebugPrint.
     *
     * @param other The SfeDebugPrint object whose debug state to copy.
     */
    void copyDebugStateFrom(const SfeDebugPrint &other)
    {
      _debugSerial = other._debugSerial;
      _printDebug = other._printDebug;
      _printLimitedDebug = other._printLimitedDebug;
    }

  protected:
    SfePrint _debugSerial;           // The stream to send debug messages to if enabled
    bool _printDebug = false;        // Flag to print debug messages
    bool _printLimitedDebug = false; // Flag to print limited (important-only) debug messages
  };

} // namespace SparkFun_SSD168x
