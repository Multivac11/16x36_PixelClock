#pragma once

#include "driver/i2s_tdm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c.h"

// ---- GPIO 定义 ----
#define I2S_MIC_WS GPIO_NUM_6
#define I2S_MIC_DI GPIO_NUM_4
#define I2S_MIC_BCK GPIO_NUM_7
#define I2S_MIC_MCK GPIO_NUM_15
#define I2S_MIC_DO GPIO_NUM_5

// ---- ES7210 基本参数 ----
#define ES7210_I2C_ADDR 0x41
#define ES7210_SAMPLE_RATE 48000
#define ES7210_MCLK_MULTIPLE I2S_MCLK_MULTIPLE_256
#define ES7210_I2S_SAMPLE_BITS I2S_DATA_BIT_WIDTH_16BIT
#define ES7210_I2S_TDM_SLOT_MASK (static_cast<i2s_tdm_slot_mask_t>(I2S_TDM_SLOT0 | I2S_TDM_SLOT1))

// ---- 从官方 es7210.h 移植的枚举 ----
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

// ---- 从官方 es7210_reg.h 移植的寄存器宏 ----
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

// ---- 用户原来的宏 ----
#define ES7210_I2S_FORMAT ES7210_I2S_FMT_I2S
#define ES7210_BIT_WIDTH ES7210_I2S_BITS_16B
#define ES7210_MIC_BIAS ES7210_MIC_BIAS_2V87
#define ES7210_MIC_GAIN ES7210_MIC_GAIN_37_5DB
#define ES7210_ADC_VOLUME 0  // 对应官方 0dB (0xBF)

class Es7210Codec
{
   public:
    static Es7210Codec& GetInstance()
    {
        static Es7210Codec instance;
        return instance;
    }

    Es7210Codec() = default;
    ~Es7210Codec();

    void InitEs7210Codec();
    bool RegisterEs7210Codec();
    static void AudioCaptureTask(void* pvParameters);
    void GetAudios();

   private:
    bool WriteReg(uint8_t reg_addr, uint8_t reg_value);
    bool ReadReg(uint8_t reg_addr, uint8_t* reg_value);
    bool SetI2sFormat(es7210_i2s_fmt_t i2s_format, es7210_i2s_bits_t bit_width, bool tdm_enable);
    bool SetSampleRate(uint32_t sample_rate_hz, uint32_t mclk_ratio);
    bool SetMicGain(es7210_mic_gain_t mic_gain);
    bool SetMicBias(es7210_mic_bias_t mic_bias);
    bool ConfigCodec();
    bool ConfigVolume(int8_t volume_db);

    i2c_master_dev_handle_t dev_handle_ = nullptr;
    i2s_chan_handle_t rx_chan_ = nullptr;
};