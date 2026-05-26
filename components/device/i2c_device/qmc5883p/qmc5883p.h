#pragma once

#include "i2c_device.h"

class QMC5883P : public I2CDevice
{
   public:
    explicit QMC5883P(i2c_master_bus_handle_t bus, uint16_t addr);

    bool Init() override;
};