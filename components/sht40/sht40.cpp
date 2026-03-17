#include "sht40.h"

#include "i2c.h"

void Sht40::InitSht40()
{
    esp_err_t ret = i2c_master_probe(I2CMaster::GetInstance().bus_handle_, SHT40_ADDR, -1);
    if (ret == ESP_ERR_NOT_FOUND)
    {
        ESP_LOGE("Sht40", "Failed to find SHT40 sensor");
        return;
    }
    else if (ret == ESP_ERR_TIMEOUT)
    {
        ESP_LOGE("Sht40", "Failed to find SHT40 sensor, timeout");
        return;
    }

    ESP_LOGI("Sht40", "Found SHT40 sensor at address 0x%02X", SHT40_ADDR);

    i2c_device_config_t dev_config = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                      .device_address = SHT40_ADDR,
                                      .scl_speed_hz = I2C_MASTER_FREQ_HZ,
                                      .scl_wait_us = 100,
                                      .flags = {.disable_ack_check = true}};

    ret = i2c_master_bus_add_device(I2CMaster::GetInstance().bus_handle_, &dev_config,
                                    &I2CMaster::GetInstance().dev_handle_);
    if (ret != ESP_OK)
    {
        ESP_LOGE("Sht40", "Failed to add SHT40 sensor to I2C bus");
        return;
    }

    queue_ = xQueueCreate(1, sizeof(EnvParamsStruct));
    xTaskCreatePinnedToCore(GetEnvParamsTask, "GetEnvParamsTask", 4096, this, 1, nullptr, 1);

    ESP_LOGI("Sht40", "Init Sht40 module successfull!");
}

void Sht40::GetEnvParamsTask(void *pvParameters)
{
    static_cast<Sht40 *>(pvParameters)->GetEnvParams();
}

void Sht40::GetEnvParams()
{
    while (true)
    {
        uint8_t buffer[6];
        ReadRawData(buffer);

        uint16_t recovery_temper = ((uint16_t)buffer[0] << 8) | buffer[1];
        env_params_.temperature = -45 + 175 * ((float)recovery_temper / 65535);
        uint16_t recovery_hum = ((uint16_t)buffer[3] << 8) | buffer[4];
        env_params_.humidity = -6 + 125 * ((float)recovery_hum / 65535);

        if (env_params_.humidity >= 100)  // 根据数据手册编写
        {
            env_params_.humidity = 100;
        }
        else if (env_params_.humidity <= 0)
        {
            env_params_.humidity = 0;
        }

        xQueueOverwrite(queue_, &env_params_);
        // ESP_LOGI("Sht40", "temperature: %.2f, humidity: %.2f", env_params_.temperature, env_params_.humidity);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void Sht40::ReadRawData(uint8_t *data)
{
    uint8_t controlid = 0xFD;  // 读取数据指令
    uint8_t ret;
    ret = i2c_master_transmit(I2CMaster::GetInstance().dev_handle_, &controlid, 1, -1);
    if (ret != ESP_OK)
    {
        ESP_LOGE("Sht40", "I2C transmit failed!");
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    ret = i2c_master_receive(I2CMaster::GetInstance().dev_handle_, data, 6, -1);
    if (ret != ESP_OK)
    {
        ESP_LOGE("Sht40", "I2C receive failed!");
    }
}

bool Sht40::Available()
{
    return xQueueReceive(queue_, &env_params_, 0) == pdTRUE;
}

Sht40::EnvParamsStruct Sht40::ReadEnvParams()
{
    return env_params_;
}