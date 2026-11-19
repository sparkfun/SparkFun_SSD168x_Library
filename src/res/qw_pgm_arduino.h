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
 * @file qw_pgm_arduino.h
 *
 * Declare pgmspace (PROGMEM, pgm_read_byte) in a platform abstract (sort of) way.
 */

#pragma once

#if defined(ARDUINO)

#if defined(TARGET_Apollo3)
// do nothing
#elif defined(__AVR__) || defined(__arm__) || defined(__ARDUINO_ARC__)
	#include <avr/pgmspace.h>

#else
	#include <pgmspace.h>
#endif

#elif defined(ESP_PLATFORM)

// Native ESP-IDF: const data is placed in flash automatically and can be read directly
#include <stdint.h>

#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#endif

#endif

