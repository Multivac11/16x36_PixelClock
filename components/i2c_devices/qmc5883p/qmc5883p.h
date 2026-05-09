#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c.h"

#define QMC5883P_ADDR 0x2C

class Qmc5883P
{
   public:
    static Qmc5883P &GetInstance()
    {
        static Qmc5883P instance;

        return instance;
    }
    Qmc5883P() = default;
    ~Qmc5883P() = default;

    void InitQmc5883P();

    bool RegisterQmc5883P();

   private:
    i2c_master_dev_handle_t dev_handle_ = nullptr;
};
