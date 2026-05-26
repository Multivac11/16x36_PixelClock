#include "es7210.h"

static const char* TAG = "ES7210";

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

ES7210::ES7210(i2c_master_bus_handle_t bus, uint16_t addr) : I2CDevice(bus, addr, TAG)
{
}

bool ES7210::Init()
{
    if (!I2CDevice::Init())
    {
        return false;
    }

    if (!ConfigCodec())
    {
        ESP_LOGE(TAG, "ConfigCodec failed");
        return false;
    }

    if (!ConfigVolume(ES7210_ADC_VOLUME))
    {
        ESP_LOGE(TAG, "ConfigVolume failed");
        return false;
    }

    ESP_LOGI(TAG, "Init ES7210 success");
    return true;
}

bool ES7210::SetI2sFormat(es7210_i2s_fmt_t i2s_format, es7210_i2s_bits_t bit_width, bool tdm_enable)
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

    uint8_t buf[2] = {ES7210_SDP_INTERFACE1_REG11, static_cast<uint8_t>(i2s_format | reg_val)};
    if (Write(buf, 2) != ESP_OK)
    {
        return false;
    }
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
        uint8_t buf1[2] = {ES7210_SDP_INTERFACE2_REG12, reg_val};
        if (Write(buf1, 2) != ESP_OK)
        {
            return false;
        }
    }
    else
    {
        uint8_t buf1[2] = {ES7210_SDP_INTERFACE2_REG12, 0x00};
        if (Write(buf1, 2) != ESP_OK)
        {
            return false;
        }
    }

    ESP_LOGI(TAG, "format: %s, bit width: %d, tdm mode %s",
             (i2s_format == ES7210_I2S_FMT_I2S)     ? "standard i2s"
             : (i2s_format == ES7210_I2S_FMT_LJ)    ? "left justify"
             : (i2s_format == ES7210_I2S_FMT_DSP_A) ? "DSP-A"
                                                    : "DSP-B",
             bit_width, tdm_enable ? "enabled" : "disabled");

    return true;
}

bool ES7210::SetSampleRate(uint32_t sample_rate_hz, uint32_t mclk_ratio)
{
    uint32_t mclk_freq_hz = sample_rate_hz * mclk_ratio;
    const coeff_div_t* coeff_div = es7210_get_coeff(mclk_freq_hz, sample_rate_hz);
    if (!coeff_div)
    {
        ESP_LOGE(TAG, "unable to set %lu Hz sample rate with %lu Hz MCLK", sample_rate_hz, mclk_freq_hz);
        return false;
    }
    uint8_t buff[2] = {ES7210_OSR_REG07, coeff_div->osr};
    if (Write(buff, 2) != ESP_OK)
    {
        return false;
    }
    uint8_t buff1[2] = {ES7210_MAINCLK_REG02,
                        static_cast<uint8_t>((coeff_div->adc_div) | (coeff_div->doubler << 6) | (coeff_div->dll << 7))};
    if (Write(buff1, 2) != ESP_OK)
    {
        return false;
    }
    uint8_t buff2[2] = {ES7210_LRCK_DIVH_REG04, static_cast<uint8_t>(coeff_div->lrck_h)};
    if (Write(buff2, 2) != ESP_OK)
    {
        return false;
    }
    uint8_t buff3[2] = {ES7210_LRCK_DIVL_REG05, static_cast<uint8_t>(coeff_div->lrck_l)};
    if (Write(buff3, 2) != ESP_OK)
    {
        return false;
    }

    ESP_LOGI(TAG, "sample rate: %lu Hz, mclk frequency: %lu Hz", sample_rate_hz, mclk_freq_hz);
    return true;
}

bool ES7210::SetMicGain(es7210_mic_gain_t mic_gain)
{
    if (!IS_ES7210_MIC_GAIN(mic_gain))
    {
        ESP_LOGE(TAG, "invalid mic gain value");
        return false;
    }
    uint8_t buff[2] = {ES7210_MIC1_GAIN_REG43, static_cast<uint8_t>(mic_gain | 0x10)};
    if (Write(buff, 2) != ESP_OK)
    {
        return false;
    }
    uint8_t buff1[2] = {ES7210_MIC2_GAIN_REG44, static_cast<uint8_t>(mic_gain | 0x10)};
    if (Write(buff1, 2) != ESP_OK)
    {
        return false;
    }
    uint8_t buff2[2] = {ES7210_MIC3_GAIN_REG45, static_cast<uint8_t>(mic_gain | 0x10)};
    if (Write(buff2, 2) != ESP_OK)
    {
        return false;
    }
    uint8_t buff3[2] = {ES7210_MIC4_GAIN_REG46, static_cast<uint8_t>(mic_gain | 0x10)};
    if (Write(buff3, 2) != ESP_OK)
    {
        return false;
    }
    return true;
}

bool ES7210::SetMicBias(es7210_mic_bias_t mic_bias)
{
    if (!IS_ES7210_MIC_BIAS(mic_bias))
    {
        ESP_LOGE(TAG, "invalid mic bias value");
        return false;
    }
    uint8_t buff[2] = {ES7210_MIC12_BIAS_REG41, mic_bias};
    if (Write(buff, 2) != ESP_OK)
    {
        return false;
    }
    uint8_t buff1[2] = {ES7210_MIC34_BIAS_REG42, mic_bias};
    if (Write(buff1, 2) != ESP_OK)
    {
        return false;
    }

    return true;
}

bool ES7210::ConfigVolume(int8_t volume_db)
{
    if (volume_db < -95 || volume_db > 32)
    {
        ESP_LOGE(TAG, "invalid volume range");
        return false;
    }
    uint8_t reg_val = 191 + volume_db * 2;
    uint8_t buff[2] = {ES7210_ADC1_DIRECT_DB_REG1B, reg_val};
    if (Write(buff, 2) != ESP_OK)
    {
        return false;
    }
    uint8_t buff1[2] = {ES7210_ADC2_DIRECT_DB_REG1C, reg_val};
    if (Write(buff1, 2) != ESP_OK)
    {
        return false;
    }
    uint8_t buff2[2] = {ES7210_ADC3_DIRECT_DB_REG1D, reg_val};
    if (Write(buff2, 2) != ESP_OK)
    {
        return false;
    }
    uint8_t buff3[2] = {ES7210_ADC4_DIRECT_DB_REG1E, reg_val};
    if (Write(buff3, 2) != ESP_OK)
    {
        return false;
    }
    return true;
}

bool ES7210::ConfigCodec()
{
    /* Perform software reset */
    uint8_t buff[2] = {ES7210_RESET_REG00, 0xFF};
    if (Write(buff, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write reset register failed");
        return false;
    }
    uint8_t buff1[2] = {ES7210_RESET_REG00, 0x32};
    if (Write(buff1, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write reset register failed");
        return false;
    }
    /* Set the initialization time when device powers up */
    uint8_t buff2[2] = {ES7210_TIME_CONTROL0_REG09, 0x30};
    if (Write(buff2, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write time control register failed");
        return false;
    }
    uint8_t buff3[2] = {ES7210_TIME_CONTROL1_REG0A, 0x30};
    if (Write(buff3, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write time control register failed");
        return false;
    }
    /* Configure HPF for ADC1-4 */
    uint8_t buff4[2] = {ES7210_ADC12_HPF1_REG23, 0x2A};
    if (Write(buff4, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write ADC12 HPF1 register failed");
        return false;
    }
    uint8_t buff5[2] = {ES7210_ADC12_HPF2_REG22, 0x0A};
    if (Write(buff5, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write ADC12 HPF2 register failed");
        return false;
    }
    uint8_t buff6[2] = {ES7210_ADC34_HPF1_REG21, 0x2A};
    if (Write(buff6, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write ADC34 HPF1 register failed");
        return false;
    }
    uint8_t buff7[2] = {ES7210_ADC34_HPF2_REG20, 0x0A};
    if (Write(buff7, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write ADC34 HPF2 register failed");
        return false;
    }
    /* Set bits per sample to 16, data protocal to I2S, enable 1xFS TDM */
    if (!SetI2sFormat(ES7210_I2S_FORMAT, ES7210_BIT_WIDTH, true))
    {
        ESP_LOGE(TAG, "Set I2S format failed");
        return false;
    }
    /* Configure analog power and VMID voltage */
    uint8_t buff8[2] = {ES7210_ANALOG_REG40, 0xC3};
    if (Write(buff8, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write analog register failed");
        return false;
    }
    /* Set MIC14 bias to 2.87V */
    if (!SetMicBias(ES7210_MIC_BIAS))
    {
        ESP_LOGE(TAG, "Set mic bias failed");
        return false;
    }
    /* Set MIC1-4 gain to 30dB */
    if (!SetMicGain(ES7210_MIC_GAIN))
    {
        ESP_LOGE(TAG, "Set mic gain failed");
        return false;
    }
    /* Power on MIC1-4 */
    uint8_t buff9[2] = {ES7210_MIC1_POWER_REG47, 0x08};
    if (Write(buff9, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write MIC1 power register failed");
        return false;
    }
    uint8_t buff10[2] = {ES7210_MIC2_POWER_REG48, 0x08};
    if (Write(buff10, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write MIC2 power register failed");
        return false;
    }
    uint8_t buff11[2] = {ES7210_MIC3_POWER_REG49, 0x08};
    if (Write(buff11, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write MIC3 power register failed");
        return false;
    }
    uint8_t buff12[2] = {ES7210_MIC4_POWER_REG4A, 0x08};
    if (Write(buff12, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write MIC4 power register failed");
        return false;
    }
    /* Set ADC sample rate to 48kHz */
    if (!SetSampleRate(ES7210_SAMPLE_RATE, ES7210_MCLK_MULTIPLE))
    {
        ESP_LOGE(TAG, "Set sample rate failed");
        return false;
    }
    /* Power down DLL */
    uint8_t buff13[2] = {ES7210_POWER_DOWN_REG06, 0x04};
    if (Write(buff13, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write power down register failed");
        return false;
    }
    /* Power on MIC1-4 bias & ADC1-4 & PGA1-4 Power */
    uint8_t buff14[2] = {ES7210_MIC12_POWER_REG4B, 0x0F};
    if (Write(buff14, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write MIC12 power register failed");
        return false;
    }
    uint8_t buff15[2] = {ES7210_MIC34_POWER_REG4C, 0x0F};
    if (Write(buff15, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write MIC34 power register failed");
        return false;
    }
    /* Enable device */
    uint8_t buff16[2] = {ES7210_RESET_REG00, 0x71};
    if (Write(buff16, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write reset register failed");
        return false;
    }
    uint8_t buff17[2] = {ES7210_RESET_REG00, 0x41};
    if (Write(buff17, 2) != ESP_OK)
    {
        ESP_LOGE(TAG, "Write reset register failed");
        return false;
    }

    return true;
}
