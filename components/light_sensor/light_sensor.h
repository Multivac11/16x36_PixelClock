#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/adc_channel.h"

#define CONVERSIONS_PER_PIN 8

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

   private:
    QueueHandle_t queue_;
    uint16_t light_value_;
};