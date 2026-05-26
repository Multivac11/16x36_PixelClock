#include "audio_capture.h"

static const char* TAG = "AudioCapture";

void AudioCapture::InitAudioCapture()
{
    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    esp_err_t ret = i2s_new_channel(&rx_chan_cfg, NULL, &rx_chan_);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "i2s_new_channel failed: %s", esp_err_to_name(ret));
        return;
    }
    if (I2CBusManager::GetInstance().GetDeviceByAddr<ES7210>(0x41) == nullptr)
    {
        ESP_LOGE(TAG, "ES7210 register failed");
        return;
    }

    i2s_tdm_config_t i2s_tdm_rx_conf = {};
    i2s_tdm_rx_conf.clk_cfg = I2S_TDM_CLK_DEFAULT_CONFIG(ES7210_SAMPLE_RATE);
    i2s_tdm_rx_conf.slot_cfg =
        I2S_TDM_PHILIPS_SLOT_DEFAULT_CONFIG(ES7210_I2S_SAMPLE_BITS, I2S_SLOT_MODE_STEREO, ES7210_I2S_TDM_SLOT_MASK);
    i2s_tdm_rx_conf.gpio_cfg = {
        .mclk = I2S_MIC_MCK,
        .bclk = I2S_MIC_BCK,
        .ws = I2S_MIC_WS,
        .dout = I2S_MIC_DO,
        .din = I2S_MIC_DI,
        .invert_flags = {false, false, false},
    };

    ESP_ERROR_CHECK(i2s_channel_init_tdm_mode(rx_chan_, &i2s_tdm_rx_conf));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan_));

    xTaskCreatePinnedToCore(AudioCaptureTask, "AudioCaptureTask", 4096, this, 1, nullptr, 1);

    ESP_LOGI(TAG, "AudioCaptureTask created");
}

void AudioCapture::AudioCaptureTask(void* pvParameters)
{
    static_cast<AudioCapture*>(pvParameters)->GetAudios();
}

void AudioCapture::GetAudios()
{
    const size_t buffer_bytes = 1024;
    int16_t* rx_buffer = (int16_t*)heap_caps_malloc(buffer_bytes, MALLOC_CAP_DMA);
    if (!rx_buffer)
    {
        ESP_LOGE(TAG, "DMA buffer alloc failed");
        vTaskDelete(NULL);
        return;
    }

    size_t bytes_read = 0;

    while (true)
    {
        esp_err_t ret = i2s_channel_read(rx_chan_, rx_buffer, buffer_bytes, &bytes_read, portMAX_DELAY);
        if (ret != ESP_OK || bytes_read == 0)
        {
            ESP_LOGE(TAG, "I2S read error");
            continue;
        }

        int total_samples = bytes_read / sizeof(int16_t);
        int frame_count = total_samples / 2;

        int32_t sum_left = 0, sum_right = 0;
        for (int i = 0; i < total_samples; i += 2)
        {
            sum_left += rx_buffer[i];
            sum_right += rx_buffer[i + 1];
        }

        float mean_left = (float)sum_left / frame_count;
        float mean_right = (float)sum_right / frame_count;

        printf("%f,%f\n", mean_left, mean_right);
    }
    heap_caps_free(rx_buffer);
    vTaskDelete(NULL);
}
