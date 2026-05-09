#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c.h"

#define ES7210_ADDR 0x41

class Es7210
{
   public:
    static Es7210 &GetInstance()
    {
        static Es7210 instance;

        return instance;
    }

    Es7210() = default;

    ~Es7210() = default;

    void InitEs7210();

    bool RegisterEs7210();

   private:
    i2c_master_dev_handle_t dev_handle_ = nullptr;
};