#pragma once

#include "i2c_device.h"

class DS3231 : public I2CDevice
{
   public:
    explicit DS3231(i2c_master_bus_handle_t bus, uint16_t addr);

    bool Init() override;

    struct DateTime
    {
        uint16_t year;    // 2000~2099
        uint8_t month;    // 1~12
        uint8_t day;      // 1~31
        uint8_t hour;     // 0~23
        uint8_t minute;   // 0~59
        uint8_t second;   // 0~59
        uint8_t weekday;  // 1~7
    };

    bool GetTime(DateTime& dt);

    bool SetTime(const DateTime& dt);

    float GetTemperature();  // 读取芯片内部温度

    bool Enable32kHzOutput(bool enable);

    bool ClearOSF();  // 清除振荡器停止标志

   private:
    bool WriteReg(uint8_t reg, uint16_t val);

    uint16_t ReadReg(uint8_t reg);

    DateTime dt_;

    uint8_t bcd2bin(uint8_t bcd) { return ((bcd >> 4) * 10) + (bcd & 0x0F); }

    uint8_t bin2bcd(uint8_t bin) { return ((bin / 10) << 4) | (bin % 10); }
};