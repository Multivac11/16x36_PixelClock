#include "qmc5883p.h"

void Qmc5883P::InitQmc5883P()
{
    if (!RegisterQmc5883P())
    {
        ESP_LOGE("Qmc5883P", "Failed to register Qmc5883P sensor");
        return;
    }
    ESP_LOGI("Qmc5883P", "Init Qmc5883P module successfull!");
}

bool Qmc5883P::RegisterQmc5883P()
{
    esp_err_t ret = i2c_master_probe(I2CMaster::GetInstance().bus_handle_, QMC5883P_ADDR, -1);
    if (ret != ESP_OK)
    {
        ESP_LOGE("Qmc5883P", "Qmc5883P not found at 0x%02X", QMC5883P_ADDR);
        return false;
    }
    ESP_LOGI("Qmc5883P", "Qmc5883P found at 0x%02X", QMC5883P_ADDR);
    i2c_device_config_t dev_config = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                      .device_address = QMC5883P_ADDR,
                                      .scl_speed_hz = I2C_MASTER_FREQ_HZ,
                                      .scl_wait_us = 100,
                                      .flags = {.disable_ack_check = false}};

    ret = i2c_master_bus_add_device(I2CMaster::GetInstance().bus_handle_, &dev_config, &dev_handle_);
    if (ret != ESP_OK)
    {
        ESP_LOGE("Qmc5883P", "Failed to add Qmc5883P to I2C bus");
        return false;
    }

    return true;
}
