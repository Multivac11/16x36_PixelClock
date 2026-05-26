#include "device_init.h"

#include "i2c_bus.h"
#include "spi_bus.h"

static const char* TAG = "Device";

void DeviceInit::Init()
{
    if (!I2CBusManager::GetInstance().Init())
    {
        ESP_LOGE(TAG, "I2C bus init failed");
        return;
    }

    if (!I2CBusManager::GetInstance().RegisterDS3231(0x68))
    {
        ESP_LOGE(TAG, "DS3231 0x68 register failed");
    }
    if (!I2CBusManager::GetInstance().RegisterSHT40(0x44))
    {
        ESP_LOGE(TAG, "SHT40 0x44 register failed");
    }
    if (!I2CBusManager::GetInstance().RegisterQMI8658(0x6A))
    {
        ESP_LOGE(TAG, "QMI8658 0x6A register failed");
    }
    if (!I2CBusManager::GetInstance().RegisterQMC5883P(0x2C))
    {
        ESP_LOGE(TAG, "QMC5883P 0x2C register failed");
    }
    if (!I2CBusManager::GetInstance().RegisterHUSB238(0x08))
    {
        ESP_LOGE(TAG, "HUSB238 0x08 register failed");
    }
    if (!I2CBusManager::GetInstance().RegisterES8311(0x18))
    {
        ESP_LOGE(TAG, "ES8311 0x18 register failed");
    }
    if (!I2CBusManager::GetInstance().RegisterES7210(0x41))
    {
        ESP_LOGE(TAG, "ES7210 0x41 register failed");
    }

    if (!SPIBusManager::GetInstance().Init())
    {
        ESP_LOGE(TAG, "SPI bus init failed");
        return;
    }

    if (!SPIBusManager::GetInstance().RegisterSpiSdCard(GPIO_NUM_14))
    {
        ESP_LOGE(TAG, "SD card register failed");
    }

    ESP_LOGI(TAG, "Init device successfull!");
}