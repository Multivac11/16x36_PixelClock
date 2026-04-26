#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c.h"

#define DS3231_ADDR 0x68

class DS3231
{
   public:
    static DS3231& GetInstance()
    {
        static DS3231 instance;

        return instance;
    }

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

    DS3231() = default;

    ~DS3231() = default;

    void InitDS3231();

    bool RegisterDS3231();

    bool GetTime(DateTime& dt);

    bool SetTime(const DateTime& dt);

    float GetTemperature();  // 读取芯片内部温度

    bool Enable32kHzOutput(bool enable);

    bool ClearOSF();  // 清除振荡器停止标志

   private:
    static void Ds3231TimeTask(void*);

    void Ds3231Time();

   private:
    i2c_master_dev_handle_t dev_handle_ = nullptr;

    DateTime dt_;

    uint8_t bcd2bin(uint8_t bcd) { return ((bcd >> 4) * 10) + (bcd & 0x0F); }

    uint8_t bin2bcd(uint8_t bin) { return ((bin / 10) << 4) | (bin % 10); }
};
