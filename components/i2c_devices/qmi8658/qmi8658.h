#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c.h"

#define QMI8658_ADDR 0x6A

class Qmi8658
{
   public:
    static Qmi8658 &GetInstance()
    {
        static Qmi8658 instance;

        return instance;
    }
    Qmi8658() = default;

    ~Qmi8658() = default;

    void InitQmi8658();

    bool RegisterQmi8658();

   private:
    i2c_master_dev_handle_t dev_handle_ = nullptr;
};
