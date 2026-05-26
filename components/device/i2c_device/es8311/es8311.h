#pragma once

#include "i2c_device.h"

class ES8311 : public I2CDevice
{
   public:
    explicit ES8311(i2c_master_bus_handle_t bus, uint16_t addr);

    bool Init() override;
};