#pragma once

#include "i2c_device.h"

class QMI8658 : public I2CDevice
{
   public:
    explicit QMI8658(i2c_master_bus_handle_t bus, uint16_t addr);

    bool Init() override;
};