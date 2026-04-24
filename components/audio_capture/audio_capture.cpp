#include "audio_capture.h"

void AudioCapture::InitAudioCapture()
{
    if (!RegisterI2sDevice())
    {
        ESP_LOGE("AudioCapture", "Failed to register I2S device");
        return;
    }
    xTaskCreatePinnedToCore(AudioCaptureTask, "AudioCaptureTask", 4096, this, 1, nullptr, 1);
    ESP_LOGI("AudioCapture", "Init AudioCapture module successfull!");
}

bool AudioCapture::RegisterI2sDevice()
{
    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    esp_err_t ret = i2s_new_channel(&rx_chan_cfg, NULL, &rx_chan_);
    if (ret != ESP_OK)
    {
        ESP_LOGI("AudioCapture", "i2s_new_channel failed: %s", esp_err_to_name(ret));
        return false;
    }

    i2s_config_ = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg =
            {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = I2S_MIC_BCK,
                .ws = I2S_MIC_WS,
                .dout = I2S_GPIO_UNUSED,
                .din = I2S_MIC_SD,
                .invert_flags =
                    {
                        .mclk_inv = false,
                        .bclk_inv = false,
                        .ws_inv = false,
                    },
            },
    };

    i2s_config_.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    ret = i2s_channel_init_std_mode(rx_chan_, &i2s_config_);
    if (ret != ESP_OK)
    {
        ESP_LOGI("AudioCapture", "i2s_channel_init_std_mode failed: %s", esp_err_to_name(ret));
        return false;
    }

    ret = i2s_channel_enable(rx_chan_);
    if (ret != ESP_OK)
    {
        ESP_LOGI("AudioCapture", "i2s_channel_enable failed: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

void AudioCapture::AudioCaptureTask(void *pvParameters)
{
    static_cast<AudioCapture *>(pvParameters)->GetAudios();
}

void AudioCapture::GetAudios()
{
    while (true)
    {
        result_ = i2s_channel_read(rx_chan_, buffer_, sizeof(buffer_), &bytesIn_, 1000);

        if (result_ == ESP_OK && bytesIn_)
        {
            int samples_read = bytesIn_ / sizeof(uint32_t);
            if (samples_read)
            {
                int64_t sum = 0;
                for (int i = 0; i < samples_read; ++i)
                {
                    int32_t val = (int32_t)buffer_[i] >> 8;
                    sum += val;
                }
                float mean = (float)sum / samples_read;
                (void)mean;
                // ESP_LOGI("AudioCapture", "mean: %f", mean);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}