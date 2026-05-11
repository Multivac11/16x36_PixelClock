#include "es7210codec.h"

#include "esp_heap_caps.h"

static const char* TAG = "ES7210";

// ---- 时钟系数表（从官方 es7210.c 完整复制）----
typedef struct
{
    uint32_t mclk;
    uint32_t lrck;
    uint8_t ss_ds;
    uint8_t adc_div;
    uint8_t dll;
    uint8_t doubler;
    uint8_t osr;
    uint8_t mclk_src;
    uint32_t lrck_h;
    uint32_t lrck_l;
} coeff_div_t;

static const coeff_div_t es7210_coeff_div[] = {
    /* 8k */
    {12288000, 8000, 0x00, 0x03, 0x01, 0x00, 0x20, 0x00, 0x06, 0x00},
    {16384000, 8000, 0x00, 0x04, 0x01, 0x00, 0x20, 0x00, 0x08, 0x00},
    {19200000, 8000, 0x00, 0x1e, 0x00, 0x01, 0x28, 0x00, 0x09, 0x60},
    {4096000, 8000, 0x00, 0x01, 0x01, 0x00, 0x20, 0x00, 0x02, 0x00},
    /* 11.025k */
    {11289600, 11025, 0x00, 0x02, 0x01, 0x00, 0x20, 0x00, 0x01, 0x00},
    /* 12k */
    {12288000, 12000, 0x00, 0x02, 0x01, 0x00, 0x20, 0x00, 0x04, 0x00},
    {19200000, 12000, 0x00, 0x14, 0x00, 0x01, 0x28, 0x00, 0x06, 0x40},
    /* 16k */
    {4096000, 16000, 0x00, 0x01, 0x01, 0x01, 0x20, 0x00, 0x01, 0x00},
    {19200000, 16000, 0x00, 0x0a, 0x00, 0x00, 0x1e, 0x00, 0x04, 0x80},
    {16384000, 16000, 0x00, 0x02, 0x01, 0x00, 0x20, 0x00, 0x04, 0x00},
    {12288000, 16000, 0x00, 0x03, 0x01, 0x01, 0x20, 0x00, 0x03, 0x00},
    /* 22.05k */
    {11289600, 22050, 0x00, 0x01, 0x01, 0x00, 0x20, 0x00, 0x02, 0x00},
    /* 24k */
    {12288000, 24000, 0x00, 0x01, 0x01, 0x00, 0x20, 0x00, 0x02, 0x00},
    {19200000, 24000, 0x00, 0x0a, 0x00, 0x01, 0x28, 0x00, 0x03, 0x20},
    /* 32k */
    {12288000, 32000, 0x00, 0x03, 0x00, 0x00, 0x20, 0x00, 0x01, 0x80},
    {16384000, 32000, 0x00, 0x01, 0x01, 0x00, 0x20, 0x00, 0x02, 0x00},
    {19200000, 32000, 0x00, 0x05, 0x00, 0x00, 0x1e, 0x00, 0x02, 0x58},
    /* 44.1k */
    {11289600, 44100, 0x00, 0x01, 0x01, 0x01, 0x20, 0x00, 0x01, 0x00},
    /* 48k */
    {12288000, 48000, 0x00, 0x01, 0x01, 0x01, 0x20, 0x00, 0x01, 0x00},
    {19200000, 48000, 0x00, 0x05, 0x00, 0x01, 0x28, 0x00, 0x01, 0x90},
    /* 64k */
    {16384000, 64000, 0x01, 0x01, 0x01, 0x00, 0x20, 0x00, 0x01, 0x00},
    {19200000, 64000, 0x00, 0x05, 0x00, 0x01, 0x1e, 0x00, 0x01, 0x2c},
    /* 88.2k */
    {11289600, 88200, 0x01, 0x01, 0x01, 0x01, 0x20, 0x00, 0x00, 0x80},
    /* 96k */
    {12288000, 96000, 0x01, 0x01, 0x01, 0x01, 0x20, 0x00, 0x00, 0x80},
    {19200000, 96000, 0x01, 0x05, 0x00, 0x01, 0x28, 0x00, 0x00, 0xc8},
};

static const coeff_div_t* es7210_get_coeff(uint32_t mclk, uint32_t lrck)
{
    for (size_t i = 0; i < sizeof(es7210_coeff_div) / sizeof(coeff_div_t); i++)
    {
        if (es7210_coeff_div[i].lrck == lrck && es7210_coeff_div[i].mclk == mclk)
        {
            return &es7210_coeff_div[i];
        }
    }
    return NULL;
}

#define IS_ES7210_I2S_FMT(val)                                                                           \
    (((val) == ES7210_I2S_FMT_I2S) || ((val) == ES7210_I2S_FMT_LJ) || ((val) == ES7210_I2S_FMT_DSP_A) || \
     ((val) == ES7210_I2S_FMT_DSP_B))

#define IS_ES7210_I2S_BITS(val)                                                                            \
    (((val) == ES7210_I2S_BITS_24B) || ((val) == ES7210_I2S_BITS_20B) || ((val) == ES7210_I2S_BITS_18B) || \
     ((val) == ES7210_I2S_BITS_16B) || ((val) == ES7210_I2S_BITS_32B))

#define IS_ES7210_MIC_GAIN(val) (((val) >= ES7210_MIC_GAIN_0DB) && ((val) <= ES7210_MIC_GAIN_37_5DB))

#define IS_ES7210_MIC_BIAS(val)                                                                               \
    (((val) == ES7210_MIC_BIAS_2V18) || ((val) == ES7210_MIC_BIAS_2V26) || ((val) == ES7210_MIC_BIAS_2V36) || \
     ((val) == ES7210_MIC_BIAS_2V45) || ((val) == ES7210_MIC_BIAS_2V55) || ((val) == ES7210_MIC_BIAS_2V66) || \
     ((val) == ES7210_MIC_BIAS_2V78) || ((val) == ES7210_MIC_BIAS_2V87))

Es7210Codec::~Es7210Codec()
{
    if (rx_chan_)
    {
        i2s_channel_disable(rx_chan_);
        i2s_del_channel(rx_chan_);
    }
    if (dev_handle_)
    {
        i2c_master_bus_rm_device(dev_handle_);
    }
}

bool Es7210Codec::WriteReg(uint8_t reg_addr, uint8_t reg_value)
{
    if (!dev_handle_)
    {
        ESP_LOGE(TAG, "WriteReg: invalid dev_handle");
        return false;
    }
    uint8_t buf[2] = {reg_addr, reg_value};
    esp_err_t ret = i2c_master_transmit(dev_handle_, buf, 2, pdMS_TO_TICKS(100));
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Write reg 0x%02X=0x%02X failed: %s", reg_addr, reg_value, esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool Es7210Codec::ReadReg(uint8_t reg_addr, uint8_t* reg_value)
{
    if (!dev_handle_)
    {
        ESP_LOGE(TAG, "ReadReg: invalid dev_handle");
        return false;
    }
    esp_err_t ret = i2c_master_transmit_receive(dev_handle_, &reg_addr, 1, reg_value, 1, pdMS_TO_TICKS(100));
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Read reg 0x%02X failed: %s", reg_addr, esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool Es7210Codec::SetI2sFormat(es7210_i2s_fmt_t i2s_format, es7210_i2s_bits_t bit_width, bool tdm_enable)
{
    if (!IS_ES7210_I2S_FMT(i2s_format) || !IS_ES7210_I2S_BITS(bit_width))
    {
        ESP_LOGE(TAG, "Invalid i2s format or bit width");
        return false;
    }

    uint8_t reg_val = 0;
    switch (bit_width)
    {
        case ES7210_I2S_BITS_16B:
            reg_val = 0x60;
            break;
        case ES7210_I2S_BITS_18B:
            reg_val = 0x40;
            break;
        case ES7210_I2S_BITS_20B:
            reg_val = 0x20;
            break;
        case ES7210_I2S_BITS_24B:
            reg_val = 0x00;
            break;
        case ES7210_I2S_BITS_32B:
            reg_val = 0x80;
            break;
        default:
            return false;
    }
    if (!WriteReg(ES7210_SDP_INTERFACE1_REG11, i2s_format | reg_val)) return false;

    switch (i2s_format)
    {
        case ES7210_I2S_FMT_I2S:
        case ES7210_I2S_FMT_LJ:
            reg_val = 0x02;
            break;
        case ES7210_I2S_FMT_DSP_A:
        case ES7210_I2S_FMT_DSP_B:
            reg_val = 0x01;
            break;
        default:
            return false;
    }

    if (tdm_enable)
    {
        if (!WriteReg(ES7210_SDP_INTERFACE2_REG12, reg_val)) return false;
    }
    else
    {
        if (!WriteReg(ES7210_SDP_INTERFACE2_REG12, 0x00)) return false;
    }

    ESP_LOGI(TAG, "format: %s, bit width: %d, tdm mode %s",
             (i2s_format == ES7210_I2S_FMT_I2S)     ? "standard i2s"
             : (i2s_format == ES7210_I2S_FMT_LJ)    ? "left justify"
             : (i2s_format == ES7210_I2S_FMT_DSP_A) ? "DSP-A"
                                                    : "DSP-B",
             bit_width, tdm_enable ? "enabled" : "disabled");
    return true;
}

bool Es7210Codec::SetSampleRate(uint32_t sample_rate_hz, uint32_t mclk_ratio)
{
    uint32_t mclk_freq_hz = sample_rate_hz * mclk_ratio;
    const coeff_div_t* coeff_div = es7210_get_coeff(mclk_freq_hz, sample_rate_hz);
    if (!coeff_div)
    {
        ESP_LOGE(TAG, "unable to set %lu Hz sample rate with %lu Hz MCLK", sample_rate_hz, mclk_freq_hz);
        return false;
    }
    if (!WriteReg(ES7210_OSR_REG07, coeff_div->osr)) return false;
    if (!WriteReg(ES7210_MAINCLK_REG02, (coeff_div->adc_div) | (coeff_div->doubler << 6) | (coeff_div->dll << 7)))
        return false;
    if (!WriteReg(ES7210_LRCK_DIVH_REG04, coeff_div->lrck_h)) return false;
    if (!WriteReg(ES7210_LRCK_DIVL_REG05, coeff_div->lrck_l)) return false;

    ESP_LOGI(TAG, "sample rate: %lu Hz, mclk frequency: %lu Hz", sample_rate_hz, mclk_freq_hz);
    return true;
}

bool Es7210Codec::SetMicGain(es7210_mic_gain_t mic_gain)
{
    if (!IS_ES7210_MIC_GAIN(mic_gain))
    {
        ESP_LOGE(TAG, "invalid mic gain value");
        return false;
    }
    if (!WriteReg(ES7210_MIC1_GAIN_REG43, mic_gain | 0x10)) return false;
    if (!WriteReg(ES7210_MIC2_GAIN_REG44, mic_gain | 0x10)) return false;
    if (!WriteReg(ES7210_MIC3_GAIN_REG45, mic_gain | 0x10)) return false;
    if (!WriteReg(ES7210_MIC4_GAIN_REG46, mic_gain | 0x10)) return false;
    return true;
}

bool Es7210Codec::SetMicBias(es7210_mic_bias_t mic_bias)
{
    if (!IS_ES7210_MIC_BIAS(mic_bias))
    {
        ESP_LOGE(TAG, "invalid mic bias value");
        return false;
    }
    if (!WriteReg(ES7210_MIC12_BIAS_REG41, mic_bias)) return false;
    if (!WriteReg(ES7210_MIC34_BIAS_REG42, mic_bias)) return false;
    return true;
}

bool Es7210Codec::ConfigVolume(int8_t volume_db)
{
    if (volume_db < -95 || volume_db > 32)
    {
        ESP_LOGE(TAG, "invalid volume range");
        return false;
    }
    uint8_t reg_val = 191 + volume_db * 2;
    if (!WriteReg(ES7210_ADC1_DIRECT_DB_REG1B, reg_val)) return false;
    if (!WriteReg(ES7210_ADC2_DIRECT_DB_REG1C, reg_val)) return false;
    if (!WriteReg(ES7210_ADC3_DIRECT_DB_REG1D, reg_val)) return false;
    if (!WriteReg(ES7210_ADC4_DIRECT_DB_REG1E, reg_val)) return false;
    return true;
}

bool Es7210Codec::ConfigCodec()
{
    /* Perform software reset */
    if (!WriteReg(ES7210_RESET_REG00, 0xFF)) return false;
    if (!WriteReg(ES7210_RESET_REG00, 0x32)) return false;
    /* Set the initialization time when device powers up */
    if (!WriteReg(ES7210_TIME_CONTROL0_REG09, 0x30)) return false;
    if (!WriteReg(ES7210_TIME_CONTROL1_REG0A, 0x30)) return false;
    /* Configure HPF for ADC1-4 */
    if (!WriteReg(ES7210_ADC12_HPF1_REG23, 0x2A)) return false;
    if (!WriteReg(ES7210_ADC12_HPF2_REG22, 0x0A)) return false;
    if (!WriteReg(ES7210_ADC34_HPF1_REG21, 0x2A)) return false;
    if (!WriteReg(ES7210_ADC34_HPF2_REG20, 0x0A)) return false;
    /* Set bits per sample to 16, data protocal to I2S, enable 1xFS TDM */
    if (!SetI2sFormat(ES7210_I2S_FORMAT, ES7210_BIT_WIDTH, true)) return false;
    /* Configure analog power and VMID voltage */
    if (!WriteReg(ES7210_ANALOG_REG40, 0xC3)) return false;
    /* Set MIC14 bias to 2.87V */
    if (!SetMicBias(ES7210_MIC_BIAS)) return false;
    /* Set MIC1-4 gain to 30dB */
    if (!SetMicGain(ES7210_MIC_GAIN)) return false;
    /* Power on MIC1-4 */
    if (!WriteReg(ES7210_MIC1_POWER_REG47, 0x08)) return false;
    if (!WriteReg(ES7210_MIC2_POWER_REG48, 0x08)) return false;
    if (!WriteReg(ES7210_MIC3_POWER_REG49, 0x08)) return false;
    if (!WriteReg(ES7210_MIC4_POWER_REG4A, 0x08)) return false;
    /* Set ADC sample rate to 48kHz */
    if (!SetSampleRate(ES7210_SAMPLE_RATE, ES7210_MCLK_MULTIPLE)) return false;
    /* Power down DLL */
    if (!WriteReg(ES7210_POWER_DOWN_REG06, 0x04)) return false;
    /* Power on MIC1-4 bias & ADC1-4 & PGA1-4 Power */
    if (!WriteReg(ES7210_MIC12_POWER_REG4B, 0x0F)) return false;
    if (!WriteReg(ES7210_MIC34_POWER_REG4C, 0x0F)) return false;
    /* Enable device */
    if (!WriteReg(ES7210_RESET_REG00, 0x71)) return false;
    if (!WriteReg(ES7210_RESET_REG00, 0x41)) return false;

    return true;
}

void Es7210Codec::InitEs7210Codec()
{
    if (!RegisterEs7210Codec())
    {
        ESP_LOGE(TAG, "Failed to register ES7210");
        return;
    }
    xTaskCreatePinnedToCore(AudioCaptureTask, "AudioCaptureTask", 4096, this, 1, nullptr, 1);
    ESP_LOGI(TAG, "Init ES7210 module successful!");
}

bool Es7210Codec::RegisterEs7210Codec()
{
    esp_err_t ret = i2c_master_probe(I2CMaster::GetInstance().bus_handle_, ES7210_I2C_ADDR, -1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "ES7210 not found at 0x%02X", ES7210_I2C_ADDR);
        return false;
    }
    ESP_LOGI(TAG, "ES7210 found at 0x%02X", ES7210_I2C_ADDR);

    i2c_device_config_t dev_config = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                      .device_address = ES7210_I2C_ADDR,
                                      .scl_speed_hz = I2C_MASTER_FREQ_HZ,
                                      .scl_wait_us = 100,
                                      .flags = {.disable_ack_check = false}};
    ret = i2c_master_bus_add_device(I2CMaster::GetInstance().bus_handle_, &dev_config, &dev_handle_);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add ES7210 to I2C bus");
        return false;
    }

    if (!ConfigCodec())
    {
        return false;
    }

    if (!ConfigVolume(ES7210_ADC_VOLUME))
    {
        return false;
    }

    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ret = i2s_new_channel(&rx_chan_cfg, NULL, &rx_chan_);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "i2s_new_channel failed: %s", esp_err_to_name(ret));
        return false;
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

    return true;
}

void Es7210Codec::AudioCaptureTask(void* pvParameters)
{
    static_cast<Es7210Codec*>(pvParameters)->GetAudios();
}

void Es7210Codec::GetAudios()
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