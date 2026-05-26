#include "i2c_bus.h"

static const char* TAG = "I2C";

bool I2CBusManager::Init()
{
    i2c_master_bus_config_t bus_config = {.i2c_port = I2C_MASTER_NUM,
                                          .sda_io_num = I2C_MASTER_SDA_IO,
                                          .scl_io_num = I2C_MASTER_SCL_IO,
                                          .clk_source = I2C_CLK_SRC_DEFAULT,
                                          .glitch_ignore_cnt = 7,
                                          .intr_priority = 0,
                                          .trans_queue_depth = 0,
                                          .flags = {.enable_internal_pullup = false, .allow_pd = false}};

    if (i2c_new_master_bus(&bus_config, &bus_handle_) != ESP_OK)
    {
        ESP_LOGE(TAG, "Bus init failed");
        return false;
    }
    ESP_LOGI(TAG, "Bus init OK");
    return true;
}

void I2CBusManager::Deinit()
{
    devices_.clear();
    if (bus_handle_)
    {
        i2c_del_master_bus(bus_handle_);
        bus_handle_ = nullptr;
    }
}

bool I2CBusManager::RegisterDS3231(uint16_t addr)
{
    for (const auto& dev : devices_)
    {
        auto* i2c = static_cast<I2CDevice*>(dev.get());
        if (i2c->GetAddress() == addr)
        {
            ESP_LOGE(TAG, "Device already registered at 0x%02X", addr);
            return false;
        }
    }

    auto dev = std::make_unique<DS3231>(bus_handle_, addr);
    if (!dev->Init())
    {
        ESP_LOGE(TAG, "DS3231 init failed at 0x%02X", addr);
        return false;
    }

    devices_.push_back(std::move(dev));
    ESP_LOGI(TAG, "DS3231 registered at 0x%02X", addr);
    return true;
}

bool I2CBusManager::RegisterSHT40(uint16_t addr)
{
    for (const auto& dev : devices_)
    {
        auto* i2c = static_cast<I2CDevice*>(dev.get());
        if (i2c->GetAddress() == addr)
        {
            ESP_LOGE(TAG, "Device already registered at 0x%02X", addr);
            return false;
        }
    }

    auto dev = std::make_unique<SHT40>(bus_handle_, addr);
    if (!dev->Init())
    {
        ESP_LOGE(TAG, "SHT40 init failed at 0x%02X", addr);
        return false;
    }

    devices_.push_back(std::move(dev));
    ESP_LOGI(TAG, "SHT40 registered at 0x%02X", addr);
    return true;
}

bool I2CBusManager::RegisterQMI8658(uint16_t addr)
{
    for (const auto& dev : devices_)
    {
        auto* i2c = static_cast<I2CDevice*>(dev.get());
        if (i2c->GetAddress() == addr)
        {
            ESP_LOGE(TAG, "Device already registered at 0x%02X", addr);
            return false;
        }
    }

    auto dev = std::make_unique<QMI8658>(bus_handle_, addr);
    if (!dev->Init())
    {
        ESP_LOGE(TAG, "QMI8658 init failed at 0x%02X", addr);
        return false;
    }

    devices_.push_back(std::move(dev));
    ESP_LOGI(TAG, "QMI8658 registered at 0x%02X", addr);
    return true;
}

bool I2CBusManager::RegisterQMC5883P(uint16_t addr)
{
    for (const auto& dev : devices_)
    {
        auto* i2c = static_cast<I2CDevice*>(dev.get());
        if (i2c->GetAddress() == addr)
        {
            ESP_LOGE(TAG, "Device already registered at 0x%02X", addr);
            return false;
        }
    }

    auto dev = std::make_unique<QMC5883P>(bus_handle_, addr);
    if (!dev->Init())
    {
        ESP_LOGE(TAG, "QMC5883P init failed at 0x%02X", addr);
        return false;
    }

    devices_.push_back(std::move(dev));
    ESP_LOGI(TAG, "QMC5883P registered at 0x%02X", addr);
    return true;
}

bool I2CBusManager::RegisterHUSB238(uint16_t addr)
{
    for (const auto& dev : devices_)
    {
        auto* i2c = static_cast<I2CDevice*>(dev.get());
        if (i2c->GetAddress() == addr)
        {
            ESP_LOGE(TAG, "Device already registered at 0x%02X", addr);
            return false;
        }
    }

    auto dev = std::make_unique<HUSB238>(bus_handle_, addr);
    if (!dev->Init())
    {
        ESP_LOGE(TAG, "HUSB238 init failed at 0x%02X", addr);
        return false;
    }

    devices_.push_back(std::move(dev));
    ESP_LOGI(TAG, "HUSB238 registered at 0x%02X", addr);
    return true;
}

bool I2CBusManager::RegisterES8311(uint16_t addr)
{
    for (const auto& dev : devices_)
    {
        auto* i2c = static_cast<I2CDevice*>(dev.get());
        if (i2c->GetAddress() == addr)
        {
            ESP_LOGE(TAG, "Device already registered at 0x%02X", addr);
            return false;
        }
    }

    auto dev = std::make_unique<ES8311>(bus_handle_, addr);
    if (!dev->Init())
    {
        ESP_LOGE(TAG, "ES8311 init failed at 0x%02X", addr);
        return false;
    }

    devices_.push_back(std::move(dev));
    ESP_LOGI(TAG, "ES8311 registered at 0x%02X", addr);
    return true;
}

bool I2CBusManager::RegisterES7210(uint16_t addr)
{
    for (const auto& dev : devices_)
    {
        auto* i2c = static_cast<I2CDevice*>(dev.get());
        if (i2c->GetAddress() == addr)
        {
            ESP_LOGE(TAG, "Device already registered at 0x%02X", addr);
            return false;
        }
    }

    auto dev = std::make_unique<ES7210>(bus_handle_, addr);
    if (!dev->Init())
    {
        ESP_LOGE(TAG, "ES7210 init failed at 0x%02X", addr);
        return false;
    }

    devices_.push_back(std::move(dev));
    ESP_LOGI(TAG, "ES7210 registered at 0x%02X", addr);
    return true;
}
