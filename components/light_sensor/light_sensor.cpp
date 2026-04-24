#include "light_sensor.h"

void LightSensor::InitLightSensor()
{
    if (!RegisterAdcDevice())
    {
        ESP_LOGE("LightSensor", "Failed to register ADC device");
        return;
    }
    queue_ = xQueueCreate(1, sizeof(uint16_t));
    xTaskCreatePinnedToCore(LightSensorTask, "LightSensorTask", 4096, this, 1, nullptr, 1);
    ESP_LOGI("LightSensor", "Init LightSensor module successfull!");
}

void LightSensor::LightSensorTask(void *pvParameters)
{
    static_cast<LightSensor *>(pvParameters)->GetLightValue();
}

void LightSensor::GetLightValue()
{
    while (true)
    {
        adc_continuous_data_t parsed_data[64];  // 用户指定最大样本数
        uint32_t num_samples = 0;

        esp_err_t ret = adc_continuous_read_parse(handle_, parsed_data, 64, &num_samples, 1000);
        if (ret == ESP_OK)
        {
            uint32_t sum = 0;
            uint32_t valid_count = 0;
            for (int i = 0; i < num_samples; i++)
            {
                if (parsed_data[i].valid)
                {
                    sum += parsed_data[i].raw_data;
                    valid_count++;
                }
            }
            if (valid_count > 0)
            {
                uint32_t avg = sum / valid_count;
                (void)avg;
                // ESP_LOGI("LightSensor", "Light Value: %d", avg);
                // xQueueOverwrite(queue_, &voltage);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

bool LightSensor::RegisterAdcDevice()
{
    handle_ = NULL;
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = 256,
        .flags = {.flush_pool = true},
    };
    esp_err_t ret = adc_continuous_new_handle(&adc_config, &handle_);
    if (ret != ESP_OK)
    {
        ESP_LOGE("LightSensor", "Failed to create ADC handle: %s", esp_err_to_name(ret));
        return false;
    }

    adc_channel_t adc_channel;
    adc_unit_t adc_unit = LIGHT_ADC_UNIT;
    adc_continuous_io_to_channel(LIGHT_ADC_IO_NUM, &adc_unit, &adc_channel);
    ESP_LOGI("LightSensor", "ADC channel: %d", adc_channel);

    adc_digi_pattern_config_t adc_pattern[1];
    adc_pattern[0].atten = LIGHT_ADC_ATTEN;
    adc_pattern[0].channel = adc_channel & 0x7;
    adc_pattern[0].unit = LIGHT_ADC_UNIT;
    adc_pattern[0].bit_width = LIGHT_ADC_BIT_WIDTH;

    adc_continuous_config_t dig_cfg = {
        .pattern_num = 1,
        .adc_pattern = adc_pattern,
        .sample_freq_hz = SAMPLE_FREQ_HZ,
        .conv_mode = LIGHT_ADC_CONV_MODE,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
    };

    ret = adc_continuous_config(handle_, &dig_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE("LightSensor", "Failed to config ADC handle: %s", esp_err_to_name(ret));
        return false;
    }
    ret = adc_continuous_start(handle_);
    if (ret != ESP_OK)
    {
        ESP_LOGE("LightSensor", "Failed to start ADC handle: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool LightSensor::Available()
{
    return xQueueReceive(queue_, &light_value_, 0) == pdTRUE;
}

uint16_t LightSensor::ReadLightValue()
{
    return light_value_;
}