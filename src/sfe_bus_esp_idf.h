/**
 * @date 2026
 * @copyright Copyright (c) 2026, SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 *
 * Please see LICENSE.md for more details
 *
 * https://github.com/sparkfun/SparkFun_SSD168x_Library
 *
 * @file sfe_bus_esp_idf.h
 *
 * Native ESP-IDF implementation of the hardware bus class (SfeSPI) and of the debug output
 * ports (SfeOutput, SfePrint).
 *
 * Included by sfe_bus.h when building as an ESP-IDF component (SFE_SSD168X_ESP_IDF). Do not include directly.
 * The Arduino equivalents are in sfe_bus.h / sfe_bus.cpp.
 *
 * The user creates and owns the bus (spi_bus_initialize()). This class attaches the e-Paper display
 * to that bus. The bus can be shared with other devices (e.g. a microSD card).
 */

#pragma once

#include "sfe_platform.h"

#if defined(SFE_SSD168X_ESP_IDF)

#include "sdkconfig.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "driver/gpio.h"

// Defaults - can be changed with idf.py menuconfig (see Kconfig)
#ifndef CONFIG_SFE_SSD168X_SPI_CLOCK_HZ
#define CONFIG_SFE_SSD168X_SPI_CLOCK_HZ 4000000
#endif

namespace SparkFun_SSD168x
{

  /**
   * @brief The default SPI clock speed (Hz). Set with idf.py menuconfig (CONFIG_SFE_SSD168X_SPI_CLOCK_HZ).
   */
  const uint32_t kDefaultSpiClockHz = CONFIG_SFE_SSD168X_SPI_CLOCK_HZ;

  // SfeSPI: SPI, using the ESP-IDF spi_master driver.
  // Chip select (CS#) and data / command (D/C#) are driven manually as GPIOs, so that a command
  // and its data can be sent in a single CS# assertion with D/C# changing part way through.
  class SfeSPI : public SSD168xDeviceBus
  {
  public:
    /**
     * @brief Construct an SfeSPI bus object with no device / pins set yet.
     */
    SfeSPI(void);
    /**
     * @brief Destroy the SfeSPI bus object, removing the device from the bus if this class added it.
     */
    ~SfeSPI(void);

    /**
     * @brief Attach the e-Paper display to an SPI bus initialized by the user with spi_bus_initialize().
     *
     * Adds the display to the bus as a device (spi_bus_add_device), SPI mode 0, with chip select
     * and data / command driven by this class as GPIOs.
     *
     * @param host The SPI host the bus was initialized on (e.g. SPI2_HOST).
     * @param cs The chip select (CS#) GPIO.
     * @param dc The data / command (D/C#) GPIO.
     * @param clockHz The SPI clock speed. Defaults to CONFIG_SFE_SSD168X_SPI_CLOCK_HZ (4MHz).
     * @return true on success.
     */
    bool init(spi_host_device_t host, gpio_num_t cs, gpio_num_t dc, uint32_t clockHz = kDefaultSpiClockHz);

    /**
     * @brief Use an SPI device the user has already added to the bus.
     *
     * The device must have been added with spics_io_num = -1 (chip select is driven by this class)
     * and SPI mode 0.
     *
     * @param device The SPI device handle.
     * @param cs The chip select (CS#) GPIO.
     * @param dc The data / command (D/C#) GPIO.
     * @return true on success.
     */
    bool init(spi_device_handle_t device, gpio_num_t cs, gpio_num_t dc);

    /**
     * @brief Write a command byte, followed by zero or more data bytes, in a single CS# assertion.
     *
     * @param command The SSD168x command byte.
     * @param data Pointer to the data / parameter bytes. Can be nullptr if length is 0.
     * @param length The number of data bytes to write after the command.
     * @return true on success, false if not initialized or the transfer failed.
     */
    bool writeCommand(uint8_t command, const uint8_t *data, uint16_t length);

  private:
    bool configurePins(gpio_num_t cs, gpio_num_t dc);
    void removeDevice(void);

    spi_device_handle_t _dev;
    bool _ownDevice; // true if we added the device - and so must remove it
    gpio_num_t _cs;
    gpio_num_t _dc;
  };

  // SfeOutput: the ESP-IDF equivalent of the Arduino Print class. It is the destination for debug
  // messages (enableDebugging).
  // Derive your own class from SfeOutput to send the data anywhere you like (a file, a socket, a queue).
  class SfeOutput
  {
  public:
    virtual ~SfeOutput() {}
    /**
     * @brief Write a block of bytes to the output.
     *
     * @param buffer Pointer to the bytes to write.
     * @param length Number of bytes to write.
     * @return The number of bytes written.
     */
    virtual size_t write(const uint8_t *buffer, size_t length) = 0;
  };

  // Output to stdout - the ESP-IDF console (UART0 or USB Serial/JTAG)
  class SfeStdoutOutput : public SfeOutput
  {
  public:
    /** @brief Write to stdout. @param buffer The bytes. @param length Number of bytes. @return Bytes written. */
    size_t write(const uint8_t *buffer, size_t length);
  };

  // Output to a UART. The UART driver must already be installed.
  class SfeUartOutput : public SfeOutput
  {
  public:
    /** @brief Construct an output which writes to a UART. @param port The UART port (driver already installed). */
    SfeUartOutput(uart_port_t port) : _port(port) {}
    /** @brief Write to the UART. @param buffer The bytes. @param length Number of bytes. @return Bytes written. */
    size_t write(const uint8_t *buffer, size_t length);

  private:
    uart_port_t _port;
  };

  // Output to a user callback function
  class SfeCallbackOutput : public SfeOutput
  {
  public:
    typedef void (*callback_t)(const uint8_t *buffer, size_t length, void *userContext);
    /** @brief Construct an output which calls a user callback. @param callback The callback. @param userContext Passed to the callback. */
    SfeCallbackOutput(callback_t callback, void *userContext = nullptr) : _callback(callback), _userContext(userContext) {}
    /** @brief Pass the bytes to the callback. @param buffer The bytes. @param length Number of bytes. @return length. */
    size_t write(const uint8_t *buffer, size_t length)
    {
      if (_callback != nullptr)
        _callback(buffer, length, _userContext);
      return length;
    }

  private:
    callback_t _callback;
    void *_userContext;
  };

  // The default debug output port
  extern SfeStdoutOutput sfeStdout;

  // SfePrint: the library's internal print helper. Wraps an SfeOutput and does the formatting.
  class SfePrint
  {
  public:
    SfePrint(void) { _outputPort = nullptr; }

    void init(SfeOutput &outputPort) { _outputPort = &outputPort; }
    inline bool operator==(SfePrint const &other) const { return _outputPort == other._outputPort; }
    inline bool operator!=(SfePrint const &other) const { return !(*this == other); }

    void write(uint8_t c)
    {
      if (_outputPort != nullptr)
        _outputPort->write(&c, 1);
    }
    void print(const char *c)
    {
      if ((_outputPort != nullptr) && (c != nullptr))
        _outputPort->write((const uint8_t *)c, strlen(c));
    }
    void print(uint32_t value, int base = DEC)
    {
      char buf[12];
      if (base == HEX)
        snprintf(buf, sizeof(buf), "%lX", (unsigned long)value);
      else
        snprintf(buf, sizeof(buf), "%lu", (unsigned long)value);
      print(buf);
    }
    void println() { print("\r\n"); }
    void println(const char *c)
    {
      print(c);
      println();
    }
    void println(uint32_t value, int base = DEC)
    {
      print(value, base);
      println();
    }

  private:
    SfeOutput *_outputPort;
  };

};

// The type used by the public API for the debug output port: enableDebugging()
namespace SparkFun_SSD168x
{
typedef SfeOutput sfe_print_t;
} // namespace SparkFun_SSD168x

#endif // SFE_SSD168X_ESP_IDF
