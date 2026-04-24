#pragma once

#include "esp_adc/adc_continuous.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LIGHT_ADC_IO_NUM 1
#define LIGHT_ADC_UNIT ADC_UNIT_1
#define LIGHT_ADC_CONV_MODE ADC_CONV_SINGLE_UNIT_1
#define LIGHT_ADC_ATTEN ADC_ATTEN_DB_12
#define LIGHT_ADC_BIT_WIDTH SOC_ADC_DIGI_MAX_BITWIDTH
#define LIGHT_READ_LEN 256
#define SAMPLE_FREQ_HZ 20000  // 20KHz采样率

class LightSensor
{
   public:
    static LightSensor &GetInstance()
    {
        static LightSensor instance;
        return instance;
    }

    LightSensor() = default;
    ~LightSensor() = default;

    void InitLightSensor();

    bool Available();

    uint16_t ReadLightValue();

   private:
    static void LightSensorTask(void *);

    void GetLightValue();

    bool RegisterAdcDevice();

   private:
    QueueHandle_t queue_;

    uint16_t light_value_;

    adc_continuous_handle_t handle_;
};