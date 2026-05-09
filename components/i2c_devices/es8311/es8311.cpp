#include "es8311.h"

void Es8311::InitEs8311()
{
    if (!RegisterEs8311())
    {
        ESP_LOGE("Es8311", "Failed to register Es8311 sensor");
        return;
    }
    ESP_LOGI("Es8311", "Init Es8311 module successfull!");
}

bool Es8311::RegisterEs8311()
{
    esp_err_t ret = i2c_master_probe(I2CMaster::GetInstance().bus_handle_, ES8311_ADDR, -1);
    if (ret != ESP_OK)
    {
        ESP_LOGE("Es8311", "Es8311 not found at 0x%02X", ES8311_ADDR);
        return false;
    }
    ESP_LOGI("Es8311", "Es8311 found at 0x%02X", ES8311_ADDR);
    i2c_device_config_t dev_config = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                      .device_address = ES8311_ADDR,
                                      .scl_speed_hz = I2C_MASTER_FREQ_HZ,
                                      .scl_wait_us = 100,
                                      .flags = {.disable_ack_check = false}};

    ret = i2c_master_bus_add_device(I2CMaster::GetInstance().bus_handle_, &dev_config, &dev_handle_);
    if (ret != ESP_OK)
    {
        ESP_LOGE("Es8311", "Failed to add Es8311 to I2C bus");
        return false;
    }

    return true;
}
