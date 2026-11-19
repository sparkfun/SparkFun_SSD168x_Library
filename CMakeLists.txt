# ESP-IDF component build file for the SparkFun SSD168x Library.
# (The Arduino IDE ignores this file. The library builds as an Arduino Library too.)
#
# Native ESP-IDF builds use the ESP-IDF bus back end: src/sfe_bus_esp_idf.cpp
# If arduino-esp32 is also used as an ESP-IDF component, ARDUINO is defined and the Arduino back end is used instead.

set(requires esp_driver_spi esp_driver_uart esp_driver_gpio esp_timer)

# If arduino-esp32 is part of this build ("Arduino as an ESP-IDF component"), depend on it so that Arduino.h is available
idf_build_get_property(build_components BUILD_COMPONENTS)
foreach(arduino_component arduino arduino-esp32 espressif__arduino-esp32)
    if(arduino_component IN_LIST build_components)
        list(APPEND requires ${arduino_component})
    endif()
endforeach()

idf_component_register(
    SRCS "src/spi_ssd1680.cpp"
         "src/spi_ssd1680_rotated.cpp"
         "src/spi_ssd1681.cpp"
         "src/qwiic_grbuffer.cpp"
         "src/sfe_debug.cpp"
         "src/sfe_bus.cpp"
         "src/sfe_bus_esp_idf.cpp"
    INCLUDE_DIRS "src"
    REQUIRES ${requires})
