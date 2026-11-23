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
 * @file sfe_bus.cpp
 *
 * Arduino implementation of the hardware bus class (SfeSPI). See sfe_bus.h.
 * The ESP-IDF implementation is in sfe_bus_esp_idf.cpp.
 */

#include "sfe_platform.h"

#if defined(SFE_SSD168X_ARDUINO) // This file contains the Arduino implementation. See sfe_bus_esp_idf.cpp for ESP-IDF

#include "sfe_bus.h"

namespace SparkFun_SSD168x
{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Constructor
  //

  SfeSPI::SfeSPI(void) : _spiPort{nullptr}, _cs{-1}, _dc{-1}
  {
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // SPI init()
  //
  // Methods to init/setup this device. The caller can provide a SPI Port, or this class
  // will use the default

  bool SfeSPI::init(SPIClass &spiPort, SPISettings &spiSettings, int cs, int dc, bool bInit)
  {
    // The chip select and data / command pins can vary from platform to platform and
    // project to project and so they must be given by the user.
    if ((cs < 0) || (dc < 0))
      return false;

    // if we don't have a SPI port already
    if (!_spiPort)
    {
      _spiPort = &spiPort;

      if (bInit)
        _spiPort->begin();
    }

    // SPI settings are needed for every transaction
    _sfeSPISettings = spiSettings;

    _cs = cs;
    _dc = dc;

    // Initialize the chip select pin - deselected
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);

    // Initialize the data / command pin - data
    pinMode(_dc, OUTPUT);
    digitalWrite(_dc, HIGH);

    return true;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // SPI init()
  //
  // The SPI port defaults to SPI. The settings default to kDefaultSpiClockHz, MSB first, mode 0

  bool SfeSPI::init(int cs, int dc)
  {
    // If the transaction settings are not provided by the user they are built here.
    SPISettings spiSettings = SPISettings(kDefaultSpiClockHz, MSBFIRST, SPI_MODE0);

    // In addition, if the port is not provided by the user, it defaults to SPI here.
    return init(SPI, spiSettings, cs, dc);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // SPI init()
  //
  // The caller provides the SPI port and clock speed. MSB first, mode 0

  bool SfeSPI::init(SPIClass &spiPort, uint32_t spiSpeed, int cs, int dc, bool bInit)
  {
    // If the transaction settings are not provided by the user they are built here.
    SPISettings spiSettings = SPISettings(spiSpeed, MSBFIRST, SPI_MODE0);

    return init(spiPort, spiSettings, cs, dc, bInit);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // writeCommand()
  //
  // CS# low, D/C# low, command, D/C# high, data, CS# high

  bool SfeSPI::writeCommand(uint8_t command, const uint8_t *data, uint16_t length)
  {
    if (!_spiPort)
      return false;

    // Apply settings
    _spiPort->beginTransaction(_sfeSPISettings);

    // Signal communication start
    digitalWrite(_cs, LOW);

    // Send the command
    digitalWrite(_dc, LOW);
    _spiPort->transfer(command);
    digitalWrite(_dc, HIGH);

    // Send the data
    if ((data != nullptr) && (length > 0))
    {
#if defined(ARDUINO_ARCH_ESP32)
      // writeBytes does not overwrite the data with the bytes read back - and is much faster
      _spiPort->writeBytes(data, length);
#else
      // Note: transfer(buf, count) would overwrite data (the graphics buffer!) with the bytes read back.
      // So transfer one byte at a time
      for (uint16_t i = 0; i < length; i++)
        _spiPort->transfer(data[i]);
#endif
    }

    // End communication
    digitalWrite(_cs, HIGH);
    _spiPort->endTransaction();

    return true;
  }

}

#endif // SFE_SSD168X_ARDUINO
