#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c.h"

#define HUSB238_ADDR 0x08

class Husb238
{
   public:
    static Husb238 &GetInstance()
    {
        static Husb238 instance;
        return instance;
    }

    /// @brief 寄存器地址
    enum HUSB238_reg_addr
    {
        Reg_PD_STATUS0 = 0x00,
        Reg_PD_STATUS1,
        Reg_SRC_PDO_5V,
        Reg_SRC_PDO_9V,
        Reg_SRC_PDO_12V,
        Reg_SRC_PDO_15V,
        Reg_SRC_PDO_18V,
        Reg_SRC_PDO_20V,
        Reg_SRC_PDO_SEL,
        Reg_GO_COMMAND,
    };

    /// @brief 写入 0x08 选择目标电压（手册编码）
    enum HUSB238_SELECT_Voltage_e
    {
        PDO_NotSel = 0x0,
        PDO_5V = 0x1,
        PDO_9V = 0x2,
        PDO_12V = 0x3,
        PDO_15V = 0x4,
        PDO_18V = 0x5,
        PDO_20V = 0x6,
    };

    /// @brief 读取 0x00 得到的电压编码
    enum HUSB238_Voltage_e
    {
        Voltage_Unknown = 0x0,
        Voltage_5V = 0x1,
        Voltage_9V = 0x2,
        Voltage_12V = 0x3,
        Voltage_15V = 0x4,
        Voltage_18V = 0x5,
        Voltage_20V = 0x6,
    };

    /// @brief GO_COMMAND 命令码（写入 0x09）
    enum HUSB238_CMD
    {
        CMD_Request_PDO = 0x01,
        CMD_Get_SRC_Cap = 0x02,
        CMD_Hard_Reset = 0x03,
    };

    /// @brief 源端能力条目
    typedef struct
    {
        bool detected;     // 源端是否支持该电压
        float current;     // 该档位最大电流 (A)
        uint16_t voltage;  // 电压值 (V)
    } Capability_t;

    Husb238() = default;

    ~Husb238() = default;

    void InitHusb238();

    bool RegisterHusb238();

    // 底层寄存器读写（已封装重复启动）
    esp_err_t ReadReg(uint8_t reg, uint8_t *data);

    esp_err_t WriteReg(uint8_t reg, uint8_t data);

    // 获取当前实际协商到的电压/电流（读 0x00）
    bool GetCurrentPDO(uint16_t *voltage, float *current);

    // 获取源端所有 PDO 能力（读 0x02~0x07，结果写入 caps[6]）
    void GetSourceCapabilities();

    // 请求指定电压（写 0x08 + 0x09），请求后需等待协商
    esp_err_t RequestVoltage(HUSB238_SELECT_Voltage_e pdo);

    // 重新获取源端能力（写 0x09 = 0x02），刷新 0x02~0x07
    esp_err_t RefreshSourceCapabilities();

    // 发送硬复位（写 0x09 = 0x03）
    esp_err_t HardReset();

    // 状态查询（读 0x01）
    bool IsAttached();        // D6: ATTACH
    uint8_t GetPDResponse();  // D[5:3]: PD_RESPONSE (0=无响应 1=成功 ...)
    bool IsCCDirFlip();       // D7: CC_DIR

    // 编码转换工具
    static float CurrentCodeToAmp(uint8_t code);  // 0x0~0xF -> A

    static uint16_t VoltageCodeToVolt(uint8_t code);  // 0x0~0x6 -> V

   private:
    i2c_master_dev_handle_t dev_handle_ = nullptr;

    bool registered_ = false;
};