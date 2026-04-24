#pragma once

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SHT40_ADDR 0x44
#define I2C_MASTER_FREQ_HZ 100000

class Sht40
{
   public:
    static Sht40 &GetInstance()
    {
        static Sht40 instance;
        return instance;
    }

    Sht40() = default;
    ~Sht40() = default;

    struct EnvParamsStruct
    {
        float temperature;
        float humidity;
    };

    void InitSht40();

    bool Available();

    EnvParamsStruct ReadEnvParams();

   private:
    static void GetEnvParamsTask(void *);

    void GetEnvParams();

    void ReadRawData(uint8_t *data);

    bool RegisterSht40();

   private:
    EnvParamsStruct env_params_;

    QueueHandle_t queue_;
};