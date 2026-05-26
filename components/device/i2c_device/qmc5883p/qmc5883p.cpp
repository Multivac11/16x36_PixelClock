#include "qmc5883p.h"

static const char* TAG = "QMC5883P";

QMC5883P::QMC5883P(i2c_master_bus_handle_t bus, uint16_t addr) : I2CDevice(bus, addr, TAG)
{
}

bool QMC5883P::Init()
{
    if (!I2CDevice::Init())
    {
        return false;
    }

    ESP_LOGI(TAG, "Init QMC5883P success");
    return true;
}