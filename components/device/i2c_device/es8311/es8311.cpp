#include "es8311.h"

static const char* TAG = "ES8311";

ES8311::ES8311(i2c_master_bus_handle_t bus, uint16_t addr) : I2CDevice(bus, addr, TAG)
{
}

bool ES8311::Init()
{
    if (!I2CDevice::Init())
    {
        return false;
    }

    ESP_LOGI(TAG, "Init QMI8658 success");
    return true;
}