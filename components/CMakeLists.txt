set(src_dirs
    status_led
    key
    i2c_devices
    i2c_devices/sht40
    i2c_devices/husb238
    i2c_devices/ds3231
    i2c_devices/qmi8658
    i2c_devices/qmc5883p
    i2c_devices/es8311
    i2c_devices/es7210codec
    spi_devices
    spi_devices/sdio
    light_sensor
    ws_matrix
    wifi_manager
)

set(include_dirs
    status_led
    key
    i2c_devices
    i2c_devices/sht40
    i2c_devices/husb238
    i2c_devices/ds3231
    i2c_devices/qmi8658
    i2c_devices/qmc5883p
    i2c_devices/es8311
    i2c_devices/es7210codec
    spi_devices
    spi_devices/sdio
    light_sensor
    ws_matrix
    wifi_manager
)

set(required_components
    driver
    esp_timer
    esp_driver_i2c
    esp_driver_spi
    esp_adc
    esp_event
    esp_wifi
    esp_http_server
    spiffs
    nvs_flash
    lwip
    json
    led_strip
    fatfs
)

idf_component_register(
    SRC_DIRS ${src_dirs}
    INCLUDE_DIRS ${include_dirs}
    REQUIRES ${required_components}
)

component_compile_options(
    -ffast-math -O3 -Wno-error=format=-Wno-format
)
