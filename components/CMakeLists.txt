set(src_dirs
    status_led
    key
    device
    device/i2c_device
    device/i2c_device/ds3231
    device/i2c_device/es7210
    device/i2c_device/es8311
    device/i2c_device/husb238
    device/i2c_device/qmc5883p
    device/i2c_device/qmi8658
    device/i2c_device/sht40
    device/spi_device
    device/spi_device/sd_card
    audio_capture
    light_sensor
    ws_matrix
    wifi_manager
)

set(include_dirs
    status_led
    key
    device
    device/i2c_device
    device/i2c_device/ds3231
    device/i2c_device/es7210
    device/i2c_device/es8311
    device/i2c_device/husb238
    device/i2c_device/qmc5883p
    device/i2c_device/qmi8658
    device/i2c_device/sht40
    device/spi_device
    device/spi_device/sd_card
    audio_capture
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
