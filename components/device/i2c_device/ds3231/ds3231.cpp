#include "ds3231.h"

static const char* TAG = "DS3231";

DS3231::DS3231(i2c_master_bus_handle_t bus, uint16_t addr) : I2CDevice(bus, addr, TAG)
{
}

bool DS3231::Init()
{
    if (!I2CDevice::Init())
    {
        return false;
    }
    ClearOSF();               // 清除掉电标志
    Enable32kHzOutput(true);  // 可选：开启 32kHz 方波输出

    ESP_LOGI(TAG, "Init DS3231 success");

    return true;
}

bool DS3231::GetTime(DateTime& dt)
{
    uint8_t reg = 0x00;  // 从秒寄存器开始读
    uint8_t buf[7];

    esp_err_t ret = WriteThenRead(&reg, 1, buf, 7);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Read time failed");
        return false;
    }

    dt.second = bcd2bin(buf[0] & 0x7F);
    dt.minute = bcd2bin(buf[1]);
    dt.hour = bcd2bin(buf[2] & 0x3F);  // 强制按 24h 制解析
    dt.weekday = bcd2bin(buf[3] & 0x07);
    dt.day = bcd2bin(buf[4]);
    dt.month = bcd2bin(buf[5] & 0x1F);
    dt.year = bcd2bin(buf[6]) + 2000;

    return true;
}

bool DS3231::SetTime(const DateTime& dt)
{
    // 一次性写入 00h~06h 共 7 个寄存器
    uint8_t buf[8];
    buf[0] = 0x00;  // 起始寄存器地址
    buf[1] = bin2bcd(dt.second);
    buf[2] = bin2bcd(dt.minute);
    buf[3] = bin2bcd(dt.hour) & 0x3F;  // 24h 模式
    buf[4] = bin2bcd(dt.weekday);
    buf[5] = bin2bcd(dt.day);
    buf[6] = bin2bcd(dt.month) & 0x1F;
    buf[7] = bin2bcd(dt.year - 2000);

    esp_err_t ret = Write(buf, 8);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Set time failed");
        return false;
    }

    return true;
}

float DS3231::GetTemperature()
{
    uint8_t reg = 0x11;  // 温度寄存器高字节
    uint8_t buf[2];

    esp_err_t ret = WriteThenRead(&reg, 1, buf, 2);
    if (ret != ESP_OK)
    {
        ESP_LOGE("DS3231", "Read temperature failed");
        return -999.0f;
    }

    // 10-bit 补码，分辨率 0.25°C
    int16_t temp_raw = ((int16_t)(buf[0]) << 2) | (buf[1] >> 6);
    if (temp_raw & 0x0200)  // 符号位扩展
        temp_raw |= 0xFC00;

    return temp_raw * 0.25f;
}

bool DS3231::Enable32kHzOutput(bool enable)
{
    uint8_t reg = 0x0F;  // 状态寄存器
    uint8_t status;

    if (WriteThenRead(&reg, 1, &status, 1) != ESP_OK)
    {
        return false;
    }

    if (enable)
    {
        status |= 0x08;  // EN32kHz (bit3)
    }
    else
    {
        status &= ~0x08;
    }
    uint8_t buf[2] = {0x0F, status};

    return Write(buf, 2) == ESP_OK;
}

bool DS3231::ClearOSF()
{
    uint8_t reg = 0x0F;
    uint8_t status;
    if (WriteThenRead(&reg, 1, &status, 1) != ESP_OK)
    {
        return false;
    }
    status &= ~0x80;  // 清除 Oscillator Stop Flag (bit7)

    uint8_t buf[2] = {0x0F, status};

    return Write(buf, 2) == ESP_OK;
}
