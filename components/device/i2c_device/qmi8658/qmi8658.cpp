#include "qmi8658.h"

static const char* TAG = "QMI8658";

QMI8658::QMI8658(i2c_master_bus_handle_t bus, uint16_t addr) : I2CDevice(bus, addr, TAG)
{
}

bool QMI8658::Init()
{
    if (!I2CDevice::Init())
    {
        return false;
    }

    ESP_LOGI(TAG, "Init QMI8658 success");
    return true;
}