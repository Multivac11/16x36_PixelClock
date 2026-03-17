#include "light_sensor.h"

void LightSensor::InitLightSensor()
{
    queue_ = xQueueCreate(1, sizeof(uint16_t));
    xTaskCreatePinnedToCore(LightSensorTask, "LightSensorTask", 2048, this, 1, nullptr, 1);
}

void LightSensor::LightSensorTask(void *pvParameters)
{
    static_cast<LightSensor *>(pvParameters)->GetLightValue();
}

void LightSensor::GetLightValue()
{
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

bool LightSensor::Available()
{
    return xQueueReceive(queue_, &light_value_, 0) == pdTRUE;
}

uint16_t LightSensor::ReadLightValue()
{
    return light_value_;
}