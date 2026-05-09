#include "qmi8658.h"

void Qmi8658::InitQmi8658()
{
    if (!RegisterQmi8658())
    {
        ESP_LOGE("Qmi8658", "Failed to register Qmi8658 sensor");
        return;
    }
    ESP_LOGI("Qmi8658", "Init Qmi8658 module successfull!");
}

bool Qmi8658::RegisterQmi8658()
{
    esp_err_t ret = i2c_master_probe(I2CMaster::GetInstance().bus_handle_, QMI8658_ADDR, -1);
    if (ret != ESP_OK)
    {
        ESP_LOGE("Qmi8658", "Qmi8658 not found at 0x%02X", QMI8658_ADDR);
        return false;
    }
    ESP_LOGI("Qmi8658", "Qmi8658 found at 0x%02X", QMI8658_ADDR);
    i2c_device_config_t dev_config = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                      .device_address = QMI8658_ADDR,
                                      .scl_speed_hz = I2C_MASTER_FREQ_HZ,
                                      .scl_wait_us = 100,
                                      .flags = {.disable_ack_check = false}};

    ret = i2c_master_bus_add_device(I2CMaster::GetInstance().bus_handle_, &dev_config, &dev_handle_);
    if (ret != ESP_OK)
    {
        ESP_LOGE("Qmi8658", "Failed to add Qmi8658 to I2C bus");
        return false;
    }

    return true;
}