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
 * @file sfe_bus.h
 *
 * The hardware bus abstraction.
 *
 * The SSD1680/1 is driven over a 4-wire SPI interface: SCLK and SDIN (PICO / MOSI), plus a chip
 * select (CS#) and a data / command select (D/C#). D/C# is low while a command byte is clocked in,
 * and high while the data / parameter bytes which follow it are clocked in. The SSD168x does not
 * use POCI (MISO): the library never reads from the display. Its state is read through the BUSY pin.
 * The reset (RES#) and BUSY pins are simple GPIO and are handled by the device class.
 *
 * Because CS# and D/C# are used by every transfer, they belong to the bus class.
 */

#pragma once

#include "sfe_platform.h"

namespace SparkFun_SSD168x
{

  /**
   * @brief The abstract hardware bus interface used by the SSD168x device classes.
   *
   * Arduino: implemented by SfeSPI in sfe_bus.cpp. ESP-IDF: implemented by SfeSPI in sfe_bus_esp_idf.cpp.
   */
  class SSD168xDeviceBus
  {
  public:
    /**
     * @brief Write a command byte, followed by zero or more data / parameter bytes, in a single CS# assertion.
     *
     * CS# is pulled low. D/C# is pulled low and the command byte is clocked out. If length is non-zero,
     * D/C# is pulled high and the data bytes are clocked out. CS# is then returned high.
     *
     * The data is not modified (the bytes read back simultaneously over SPI are discarded).
     *
     * @param command The SSD168x command byte.
     * @param data Pointer to the data / parameter bytes. Can be nullptr if length is 0.
     * @param length The number of data bytes to write after the command.
     * @return true on success, false if the bus is not initialized or the transfer failed.
     */
    virtual bool writeCommand(uint8_t command, const uint8_t *data, uint16_t length) = 0;
  };

#if defined(SFE_SSD168X_ARDUINO)

  /**
   * @brief The default SPI clock speed (Hz). The SSD168x supports up to 20MHz for writes.
   */
  const uint32_t kDefaultSpiClockHz = 4000000;

  // The SfeSPI class defines behavior for SPI implementation based around the SPIClass class (SPI).
  // This is Arduino specific.
  class SfeSPI : public SSD168xDeviceBus
  {
  public:
    /**
     * @brief Construct an SfeSPI bus object with no port / pins set yet.
     */
    SfeSPI(void);

    /**
     * @brief Initialize this SPI bus using the default SPI port at kDefaultSpiClockHz, mode 0.
     *
     * @param cs The chip select (CS#) pin (required - platform / project-specific).
     * @param dc The data / command (D/C#) pin (required - platform / project-specific).
     * @return true on success, false if either pin is invalid (negative).
     */
    bool init(int cs, int dc);

    /**
     * @brief Initialize this SPI bus using a caller-supplied port and transaction settings.
     *
     * Configures 'cs' and 'dc' as outputs, both driven HIGH (deselected / data) once initialized.
     *
     * @param spiPort The SPIClass instance to use.
     * @param spiSettings SPISettings (clock, bit order, mode) applied on every transaction.
     *                    The SSD168x uses SPI mode 0, MSB first.
     * @param cs The chip select (CS#) pin (required).
     * @param dc The data / command (D/C#) pin (required).
     * @param bInit If true, also call spiPort.begin(). Defaults to false.
     * @return true on success, false if either pin is invalid (negative).
     */
    bool init(SPIClass &spiPort, SPISettings &spiSettings, int cs, int dc, bool bInit = false);

    /**
     * @brief Initialize this SPI bus using a caller-supplied port and clock speed (mode 0, MSB first).
     *
     * @param spiPort The SPIClass instance to use.
     * @param spiSpeed SPI clock speed in Hz.
     * @param cs The chip select (CS#) pin (required).
     * @param dc The data / command (D/C#) pin (required).
     * @param bInit If true, also call spiPort.begin(). Defaults to false.
     * @return true on success, false if either pin is invalid (negative).
     */
    bool init(SPIClass &spiPort, uint32_t spiSpeed, int cs, int dc, bool bInit = false);

    /**
     * @brief Write a command byte, followed by zero or more data bytes, in a single CS# assertion.
     *
     * @param command The SSD168x command byte.
     * @param data Pointer to the data / parameter bytes. Can be nullptr if length is 0.
     * @param length The number of data bytes to write after the command.
     * @return true on success, false if not initialized.
     */
    bool writeCommand(uint8_t command, const uint8_t *data, uint16_t length);

  private:
    SPIClass *_spiPort;
    // Settings are used for every transaction.
    SPISettings _sfeSPISettings;
    int _cs;
    int _dc;
  };

  // The sfePrint device defines behavior for Serial diagnostic prints based around the Print class.
  // This is Arduino specific.
  class SfePrint
  {
  public:
    /**
     * @brief Construct an SfePrint with no output port set yet (prints are silently dropped).
     */
    SfePrint(void) { _outputPort = nullptr; }

    /**
     * @brief Set the Print-derived stream that debug / diagnostic output is sent to.
     *
     * @param outputPort The stream to print to (e.g. Serial).
     */
    void init(Print &outputPort) { _outputPort = &outputPort; }

    /**
     * @brief Compare two SfePrint objects by their underlying output port pointer.
     *
     * @param other The SfePrint to compare against.
     * @return true if both wrap the same underlying Print pointer (including both null).
     */
    inline bool operator==(SfePrint const &other) const { return _outputPort == other._outputPort; }

    /**
     * @brief Inverse of operator==().
     *
     * @param other The SfePrint to compare against.
     * @return true if the two objects wrap different underlying Print pointers.
     */
    inline bool operator!=(SfePrint const &other) const { return !(*this == other); }

    /**
     * @brief Write a single raw byte to the output port, if one is set.
     *
     * @param c The byte to write.
     */
    void write(uint8_t c)
    {
      if (_outputPort != nullptr)
        _outputPort->write(c);
    }
    /**
     * @brief Print a null-terminated string to the output port, if one is set.
     *
     * @param c The string to print.
     */
    void print(const char *c)
    {
      if (_outputPort != nullptr)
        _outputPort->print(c);
    }
    /**
     * @brief Print a PROGMEM (F()) string to the output port, if one is set.
     *
     * @param c The flash-stored string to print.
     */
    void print(const __FlashStringHelper *c)
    {
      if (_outputPort != nullptr)
        _outputPort->print(c);
    }
    /**
     * @brief Print an unsigned value in decimal to the output port, if one is set.
     *
     * @param c The value to print.
     */
    void print(uint32_t c)
    {
      if (_outputPort != nullptr)
        _outputPort->print(c);
    }
    /**
     * @brief Print an unsigned value in a given base to the output port, if one is set.
     *
     * @param c The value to print.
     * @param f Number base to print in (e.g. HEX, DEC).
     */
    void print(uint32_t c, int f)
    {
      if (_outputPort != nullptr)
        _outputPort->print(c, f);
    }
    /**
     * @brief Print a blank line to the output port, if one is set.
     */
    void println()
    {
      if (_outputPort != nullptr)
        _outputPort->println();
    }
    /**
     * @brief Print a null-terminated string followed by a newline, if an output port is set.
     *
     * @param c The string to print.
     */
    void println(const char *c)
    {
      if (_outputPort != nullptr)
        _outputPort->println(c);
    }
    /**
     * @brief Print a PROGMEM (F()) string followed by a newline, if an output port is set.
     *
     * @param c The flash-stored string to print.
     */
    void println(const __FlashStringHelper *c)
    {
      if (_outputPort != nullptr)
        _outputPort->println(c);
    }
    /**
     * @brief Print an unsigned value in decimal followed by a newline, if an output port is set.
     *
     * @param c The value to print.
     */
    void println(uint32_t c)
    {
      if (_outputPort != nullptr)
        _outputPort->println(c);
    }
    /**
     * @brief Print an unsigned value in a given base followed by a newline, if an output port is set.
     *
     * @param c The value to print.
     * @param f Number base to print in (e.g. HEX, DEC).
     */
    void println(uint32_t c, int f)
    {
      if (_outputPort != nullptr)
        _outputPort->println(c, f);
    }

  private:
    Print *_outputPort;
  };

#endif // SFE_SSD168X_ARDUINO

};

#if defined(SFE_SSD168X_ARDUINO)
// The type used by the public API for the debug output port: enableDebugging()
namespace SparkFun_SSD168x
{
typedef Print sfe_print_t;
} // namespace SparkFun_SSD168x
#elif defined(SFE_SSD168X_ESP_IDF)
// ESP-IDF bus class (SfeSPI), output ports (SfeOutput) and SfePrint
#include "sfe_bus_esp_idf.h"
#endif
