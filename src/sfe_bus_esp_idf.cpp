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
 * @file sfe_bus_esp_idf.cpp
 *
 * Native ESP-IDF implementation of the hardware bus class. See sfe_bus_esp_idf.h.
 * The Arduino implementation is in sfe_bus.cpp.
 */

#include "sfe_platform.h"

#if defined(SFE_ESP_IDF) // This file contains the ESP-IDF implementation. See sfe_bus.cpp for Arduino

#include "sfe_bus.h"
#include "esp_log.h"

static const char *TAG = "sfe_ssd168x";

namespace SparkFun_SSD168x
{

  SfeStdoutOutput sfeStdout;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // SPI

  // The data is copied into a small (word-aligned, stack-based, so DMA-capable) buffer and sent in
  // chunks of this size. This works with any bus max_transfer_sz, with or without DMA, and with
  // data stored anywhere (internal RAM, PSRAM or flash).
  static const size_t kSpiMaxChunk = 64;

  SfeSPI::SfeSPI(void) : _dev{nullptr}, _ownDevice{false}, _cs{GPIO_NUM_NC}, _dc{GPIO_NUM_NC}
  {
  }

  SfeSPI::~SfeSPI(void)
  {
    removeDevice();
  }

  void SfeSPI::removeDevice(void)
  {
    if ((_dev != nullptr) && _ownDevice)
      spi_bus_remove_device(_dev);
    _dev = nullptr;
    _ownDevice = false;
  }

  bool SfeSPI::configurePins(gpio_num_t cs, gpio_num_t dc)
  {
    // The chip select and data / command pins can vary from platform to platform and
    // project to project and so they must be given by the user.
    if ((cs == GPIO_NUM_NC) || !GPIO_IS_VALID_OUTPUT_GPIO(cs))
    {
      ESP_LOGE(TAG, "SPI: invalid chip select GPIO %d", (int)cs);
      return false;
    }
    if ((dc == GPIO_NUM_NC) || !GPIO_IS_VALID_OUTPUT_GPIO(dc))
    {
      ESP_LOGE(TAG, "SPI: invalid data / command GPIO %d", (int)dc);
      return false;
    }

    _cs = cs;
    sfe_pin_output((int)_cs);
    sfe_pin_write((int)_cs, true); // Deselect

    _dc = dc;
    sfe_pin_output((int)_dc);
    sfe_pin_write((int)_dc, true); // Data
    return true;
  }

  bool SfeSPI::init(spi_host_device_t host, gpio_num_t cs, gpio_num_t dc, uint32_t clockHz)
  {
    if (_dev != nullptr) // Already initialized
      return true;

    if (!configurePins(cs, dc))
      return false;

    spi_device_interface_config_t devConfig = {};
    devConfig.mode = 0;             // The SSD168x uses SPI mode 0
    devConfig.clock_speed_hz = (int)clockHz;
    devConfig.spics_io_num = -1;    // Chip select is driven manually
    devConfig.queue_size = 1;

    esp_err_t err = spi_bus_add_device(host, &devConfig, &_dev);
    if (err != ESP_OK)
    {
      ESP_LOGE(TAG, "spi_bus_add_device failed: %s", esp_err_to_name(err));
      _dev = nullptr;
      return false;
    }

    _ownDevice = true;
    return true;
  }

  bool SfeSPI::init(spi_device_handle_t device, gpio_num_t cs, gpio_num_t dc)
  {
    if (device == nullptr)
      return false;

    if (_dev == device) // Already initialized
      return true;

    removeDevice();

    if (!configurePins(cs, dc))
      return false;

    _dev = device;
    _ownDevice = false;
    return true;
  }

  // CS# low, D/C# low, command, D/C# high, data, CS# high
  bool SfeSPI::writeCommand(uint8_t command, const uint8_t *data, uint16_t length)
  {
    if (_dev == nullptr)
      return false;

    if (spi_device_acquire_bus(_dev, portMAX_DELAY) != ESP_OK)
      return false;

    sfe_pin_write((int)_cs, false); // Signal communication start

    // Send the command
    sfe_pin_write((int)_dc, false);
    spi_transaction_t t = {};
    t.flags = SPI_TRANS_USE_TXDATA;
    t.length = 8; // In bits
    t.tx_data[0] = command;
    bool ok = (spi_device_polling_transmit(_dev, &t) == ESP_OK);
    sfe_pin_write((int)_dc, true);

    // Send the data
    if (ok && (data != nullptr) && (length > 0))
    {
      uint32_t chunkBuffer[kSpiMaxChunk / sizeof(uint32_t)]; // Word-aligned
      while (ok && (length > 0))
      {
        size_t chunk = (length > kSpiMaxChunk) ? kSpiMaxChunk : length;
        memcpy(chunkBuffer, data, chunk);

        spi_transaction_t td = {};
        td.length = chunk * 8; // In bits
        td.tx_buffer = chunkBuffer;
        td.rx_buffer = nullptr;
        ok = (spi_device_polling_transmit(_dev, &td) == ESP_OK);

        data += chunk;
        length -= chunk;
      }
    }

    sfe_pin_write((int)_cs, true); // End communication
    spi_device_release_bus(_dev);

    return ok;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Output ports

  size_t SfeStdoutOutput::write(const uint8_t *buffer, size_t length)
  {
    return fwrite(buffer, 1, length, stdout);
  }

  size_t SfeUartOutput::write(const uint8_t *buffer, size_t length)
  {
    int written = uart_write_bytes(_port, (const void *)buffer, length);
    return (written < 0) ? 0 : (size_t)written;
  }

}

#endif // SFE_ESP_IDF
