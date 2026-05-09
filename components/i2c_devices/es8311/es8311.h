#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c.h"

#define ES8311_ADDR 0x18

class Es8311
{
   public:
    static Es8311 &GetInstance()
    {
        static Es8311 instance;

        return instance;
    }

    Es8311() = default;

    ~Es8311() = default;

    void InitEs8311();

    bool RegisterEs8311();

   private:
    i2c_master_dev_handle_t dev_handle_ = nullptr;
};
