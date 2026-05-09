#include "es7210.h"

void Es7210::InitEs7210()
{
    if (!RegisterEs7210())
    {
        ESP_LOGE("Es7210", "Failed to register Es7210 sensor");
        return;
    }
    ESP_LOGI("Es7210", "Init Es7210 module successfull!");
}

bool Es7210::RegisterEs7210()
{
    esp_err_t ret = i2c_master_probe(I2CMaster::GetInstance().bus_handle_, ES7210_ADDR, -1);
    if (ret != ESP_OK)
    {
        ESP_LOGE("Es7210", "Es7210 not found at 0x%02X", ES7210_ADDR);
        return false;
    }
    ESP_LOGI("Es7210", "Es7210 found at 0x%02X", ES7210_ADDR);
    i2c_device_config_t dev_config = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                      .device_address = ES7210_ADDR,
                                      .scl_speed_hz = I2C_MASTER_FREQ_HZ,
                                      .scl_wait_us = 100,
                                      .flags = {.disable_ack_check = false}};

    ret = i2c_master_bus_add_device(I2CMaster::GetInstance().bus_handle_, &dev_config, &dev_handle_);
    if (ret != ESP_OK)
    {
        ESP_LOGE("Es7210", "Failed to add Es7210 to I2C bus");
        return false;
    }

    return true;
}