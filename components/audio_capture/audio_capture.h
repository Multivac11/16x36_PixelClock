#pragma once

#include "driver/i2s_tdm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bus.h"

// ---- GPIO 定义 ----
#define I2S_MIC_WS GPIO_NUM_6
#define I2S_MIC_DI GPIO_NUM_4
#define I2S_MIC_BCK GPIO_NUM_7
#define I2S_MIC_MCK GPIO_NUM_15
#define I2S_MIC_DO GPIO_NUM_5

class AudioCapture
{
   public:
    static AudioCapture& GetInstance()
    {
        static AudioCapture instance;

        return instance;
    }

    AudioCapture() = default;

    ~AudioCapture() = default;

    void InitAudioCapture();

    static void AudioCaptureTask(void* pvParameters);

    void GetAudios();

   private:
    i2s_chan_handle_t rx_chan_ = nullptr;
};