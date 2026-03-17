#pragma once

#include <cmath>

#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SAMPLE_RATE (44100)

#define I2S_MIC_WS GPIO_NUM_6
#define I2S_MIC_SD GPIO_NUM_7
#define I2S_MIC_BCK GPIO_NUM_15
#define I2S_PORT I2S_NUM_0
#define bufferLen 64

class AudioCapture
{
   public:
    static AudioCapture &GetInstance()
    {
        static AudioCapture instance;
        return instance;
    }

    AudioCapture() = default;
    ~AudioCapture() = default;

    void InitAudioCapture();

   private:
    static void AudioCaptureTask(void *);

    bool RegisterI2sDevice();

    void GetAudios();

   private:
    QueueHandle_t queue_;
    i2s_std_config_t i2s_config_;
    uint32_t buffer_[bufferLen];
    size_t bytesIn_ = 0;
    esp_err_t result_;
    i2s_chan_handle_t rx_chan_;
};