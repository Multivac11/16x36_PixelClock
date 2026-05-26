#pragma once
#include <memory>
#include <vector>

#include "driver/i2c_master.h"
#include "ds3231.h"
#include "es7210.h"
#include "es8311.h"
#include "esp_log.h"
#include "husb238.h"
#include "i2c_device.h"
#include "qmc5883p.h"
#include "qmi8658.h"
#include "sht40.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_47
#define I2C_MASTER_SDA_IO GPIO_NUM_48
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000

class I2CBusManager
{
   public:
    static I2CBusManager& GetInstance()
    {
        static I2CBusManager instance;
        return instance;
    }

    bool Init();

    void Deinit();

    bool RegisterDS3231(uint16_t addr);

    bool RegisterSHT40(uint16_t addr);

    bool RegisterQMI8658(uint16_t addr);

    bool RegisterQMC5883P(uint16_t addr);

    bool RegisterHUSB238(uint16_t addr);

    bool RegisterES8311(uint16_t addr);

    bool RegisterES7210(uint16_t addr);

    template <typename T>
    T* GetDeviceByAddr(uint16_t addr)
    {
        for (auto& dev : devices_)
        {
            auto* i2c = static_cast<I2CDevice*>(dev.get());
            if (i2c->GetAddress() == addr) return static_cast<T*>(dev.get());
        }
        return nullptr;
    }

    i2c_master_bus_handle_t GetBusHandle() const { return bus_handle_; }

   private:
    I2CBusManager() = default;

    ~I2CBusManager() = default;

    i2c_master_bus_handle_t bus_handle_ = nullptr;

    std::vector<std::unique_ptr<Device>> devices_;
};
