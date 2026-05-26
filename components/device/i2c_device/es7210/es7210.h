#pragma once

#include "driver/i2s_tdm.h"
#include "i2c_device.h"

#define ES7210_SAMPLE_RATE 48000
#define ES7210_MCLK_MULTIPLE I2S_MCLK_MULTIPLE_256
#define ES7210_I2S_SAMPLE_BITS I2S_DATA_BIT_WIDTH_16BIT
#define ES7210_I2S_TDM_SLOT_MASK (static_cast<i2s_tdm_slot_mask_t>(I2S_TDM_SLOT0 | I2S_TDM_SLOT1))

#define ES7210_RESET_REG00 0x00
#define ES7210_CLOCK_OFF_REG01 0x01
#define ES7210_MAINCLK_REG02 0x02
#define ES7210_MASTER_CLK_REG03 0x03
#define ES7210_LRCK_DIVH_REG04 0x04
#define ES7210_LRCK_DIVL_REG05 0x05
#define ES7210_POWER_DOWN_REG06 0x06
#define ES7210_OSR_REG07 0x07
#define ES7210_MODE_CONFIG_REG08 0x08
#define ES7210_TIME_CONTROL0_REG09 0x09
#define ES7210_TIME_CONTROL1_REG0A 0x0A
#define ES7210_SDP_INTERFACE1_REG11 0x11
#define ES7210_SDP_INTERFACE2_REG12 0x12
#define ES7210_ADC_AUTOMUTE_REG13 0x13
#define ES7210_ADC34_MUTERANGE_REG14 0x14
#define ES7210_ALC_SEL_REG16 0x16
#define ES7210_ADC1_DIRECT_DB_REG1B 0x1B
#define ES7210_ADC2_DIRECT_DB_REG1C 0x1C
#define ES7210_ADC3_DIRECT_DB_REG1D 0x1D
#define ES7210_ADC4_DIRECT_DB_REG1E 0x1E
#define ES7210_ADC34_HPF2_REG20 0x20
#define ES7210_ADC34_HPF1_REG21 0x21
#define ES7210_ADC12_HPF2_REG22 0x22
#define ES7210_ADC12_HPF1_REG23 0x23
#define ES7210_ANALOG_REG40 0x40
#define ES7210_MIC12_BIAS_REG41 0x41
#define ES7210_MIC34_BIAS_REG42 0x42
#define ES7210_MIC1_GAIN_REG43 0x43
#define ES7210_MIC2_GAIN_REG44 0x44
#define ES7210_MIC3_GAIN_REG45 0x45
#define ES7210_MIC4_GAIN_REG46 0x46
#define ES7210_MIC1_POWER_REG47 0x47
#define ES7210_MIC2_POWER_REG48 0x48
#define ES7210_MIC3_POWER_REG49 0x49
#define ES7210_MIC4_POWER_REG4A 0x4A
#define ES7210_MIC12_POWER_REG4B 0x4B
#define ES7210_MIC34_POWER_REG4C 0x4C

#define ES7210_I2S_FORMAT ES7210_I2S_FMT_I2S
#define ES7210_BIT_WIDTH ES7210_I2S_BITS_16B
#define ES7210_MIC_BIAS ES7210_MIC_BIAS_2V87
#define ES7210_MIC_GAIN ES7210_MIC_GAIN_37_5DB
#define ES7210_ADC_VOLUME 0  // 对应官方 0dB (0xBF)

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

class ES7210 : public I2CDevice
{
   public:
    explicit ES7210(i2c_master_bus_handle_t bus, uint16_t addr);

    bool Init() override;

    typedef enum
    {
        ES7210_I2S_FMT_I2S = 0x00,
        ES7210_I2S_FMT_LJ = 0x01,
        ES7210_I2S_FMT_DSP_A = 0x03,
        ES7210_I2S_FMT_DSP_B = 0x13
    } es7210_i2s_fmt_t;

    typedef enum
    {
        ES7210_I2S_BITS_16B = 16,
        ES7210_I2S_BITS_18B = 18,
        ES7210_I2S_BITS_20B = 20,
        ES7210_I2S_BITS_24B = 24,
        ES7210_I2S_BITS_32B = 32
    } es7210_i2s_bits_t;

    typedef enum
    {
        ES7210_MIC_GAIN_0DB = 0,
        ES7210_MIC_GAIN_3DB = 1,
        ES7210_MIC_GAIN_6DB = 2,
        ES7210_MIC_GAIN_9DB = 3,
        ES7210_MIC_GAIN_12DB = 4,
        ES7210_MIC_GAIN_15DB = 5,
        ES7210_MIC_GAIN_18DB = 6,
        ES7210_MIC_GAIN_21DB = 7,
        ES7210_MIC_GAIN_24DB = 8,
        ES7210_MIC_GAIN_27DB = 9,
        ES7210_MIC_GAIN_30DB = 10,
        ES7210_MIC_GAIN_33DB = 11,
        ES7210_MIC_GAIN_34_5DB = 12,
        ES7210_MIC_GAIN_36DB = 13,
        ES7210_MIC_GAIN_37_5DB = 14
    } es7210_mic_gain_t;

    typedef enum
    {
        ES7210_MIC_BIAS_2V18 = 0x00,
        ES7210_MIC_BIAS_2V26 = 0x10,
        ES7210_MIC_BIAS_2V36 = 0x20,
        ES7210_MIC_BIAS_2V45 = 0x30,
        ES7210_MIC_BIAS_2V55 = 0x40,
        ES7210_MIC_BIAS_2V66 = 0x50,
        ES7210_MIC_BIAS_2V78 = 0x60,
        ES7210_MIC_BIAS_2V87 = 0x70
    } es7210_mic_bias_t;

    bool SetI2sFormat(es7210_i2s_fmt_t i2s_format, es7210_i2s_bits_t bit_width, bool tdm_enable);

    bool SetSampleRate(uint32_t sample_rate_hz, uint32_t mclk_ratio);

    bool SetMicGain(es7210_mic_gain_t mic_gain);

    bool SetMicBias(es7210_mic_bias_t mic_bias);

    bool ConfigCodec();

    bool ConfigVolume(int8_t volume_db);
};