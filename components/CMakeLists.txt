set(src_dirs
    status_led
    key
    sht40
    I2C
)

set(include_dirs
    status_led
    key
    sht40
    I2C
)

set(required_components
    driver
    esp_timer
    esp_driver_i2c
)

idf_component_register(
    SRC_DIRS ${src_dirs}
    INCLUDE_DIRS ${include_dirs}
    REQUIRES ${required_components}
)

component_compile_options(
    -ffast-math -O3 -Wno-error=format=-Wno-format
)
