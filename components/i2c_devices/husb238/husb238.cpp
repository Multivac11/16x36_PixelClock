#include "husb238.h"

static const char *TAG = "Husb238";

void Husb238::InitHusb238()
{
    if (!RegisterHusb238())
    {
        ESP_LOGE(TAG, "HUSB238 registration failed");
        return;
    }

    GetSourceCapabilities();

    uint16_t voltage;
    float current;
    GetCurrentPDO(&voltage, &current);

    vTaskDelay(pdMS_TO_TICKS(100));
}

bool Husb238::RegisterHusb238()
{
    if (registered_) return true;

    esp_err_t ret = i2c_master_probe(I2CMaster::GetInstance().bus_handle_, HUSB238_ADDR, -1);
    if (ret == ESP_ERR_NOT_FOUND)
    {
        ESP_LOGE(TAG, "Device not found at 0x%02X", HUSB238_ADDR);
        return false;
    }
    else if (ret == ESP_ERR_TIMEOUT)
    {
        ESP_LOGE(TAG, "Probe timeout at 0x%02X", HUSB238_ADDR);
        return false;
    }
    else if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Probe error: %s", esp_err_to_name(ret));
        return false;
    }

    i2c_device_config_t dev_config = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                      .device_address = HUSB238_ADDR,
                                      .scl_speed_hz = I2C_MASTER_FREQ_HZ,
                                      .scl_wait_us = 100,
                                      .flags = {.disable_ack_check = false}};

    ret = i2c_master_bus_add_device(I2CMaster::GetInstance().bus_handle_, &dev_config, &dev_handle_);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add device: %s", esp_err_to_name(ret));
        return false;
    }

    ESP_LOGI(TAG, "HUSB238 registered at 0x%02X", HUSB238_ADDR);
    registered_ = true;
    return true;
}

esp_err_t Husb238::ReadReg(uint8_t reg, uint8_t *data)
{
    if (!registered_ || dev_handle_ == nullptr) return ESP_ERR_INVALID_STATE;

    // 写寄存器地址 + 重复启动 + 读 1 字节
    return i2c_master_transmit_receive(dev_handle_, &reg, 1, data, 1, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
}

esp_err_t Husb238::WriteReg(uint8_t reg, uint8_t data)
{
    if (!registered_ || dev_handle_ == nullptr) return ESP_ERR_INVALID_STATE;

    uint8_t buf[2] = {reg, data};
    return i2c_master_transmit(dev_handle_, buf, 2, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
}

bool Husb238::GetCurrentPDO(uint16_t *voltage, float *current)
{
    if (!voltage || !current) return false;

    uint8_t status0;
    if (ReadReg(Reg_PD_STATUS0, &status0) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read PD_STATUS0");
        return false;
    }

    uint8_t volt_code = (status0 >> 4) & 0x0F;  // D[7:4]
    uint8_t curr_code = status0 & 0x0F;         // D[3:0]

    *voltage = VoltageCodeToVolt(volt_code);
    *current = CurrentCodeToAmp(curr_code);

    ESP_LOGI(TAG, "Current PDO: %uV %.2fA (raw=0x%02X)", *voltage, *current, status0);
    return true;
}

void Husb238::GetSourceCapabilities()
{
    Capability_t caps[6];
    static const uint16_t voltages[6] = {5, 9, 12, 15, 18, 20};
    static const uint8_t regs[6] = {Reg_SRC_PDO_5V,  Reg_SRC_PDO_9V,  Reg_SRC_PDO_12V,
                                    Reg_SRC_PDO_15V, Reg_SRC_PDO_18V, Reg_SRC_PDO_20V};

    for (int i = 0; i < 6; i++)
    {
        uint8_t val = 0;
        if (ReadReg(regs[i], &val) != ESP_OK)
        {
            caps[i] = {false, 0.0f, voltages[i]};
            continue;
        }

        caps[i].detected = (val & 0x80) ? true : false;  // D7 = SRC_DETECTED
        uint8_t curr_code = val & 0x0F;                  // D[3:0] = current code
        caps[i].current = CurrentCodeToAmp(curr_code);
        caps[i].voltage = voltages[i];

        ESP_LOGI(TAG, "PDO %2dV: %s | max %.2fA (raw=0x%02X)", voltages[i], caps[i].detected ? "SUPPORT" : "  --  ",
                 caps[i].current, val);
    }
}

esp_err_t Husb238::RequestVoltage(HUSB238_SELECT_Voltage_e pdo)
{
    if (pdo == PDO_NotSel) return ESP_ERR_INVALID_ARG;

    esp_err_t ret = WriteReg(Reg_SRC_PDO_SEL, (uint8_t)pdo);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to write SRC_PDO_SEL");
        return ret;
    }

    ret = WriteReg(Reg_GO_COMMAND, CMD_Request_PDO);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to write GO_COMMAND");
        return ret;
    }

    ESP_LOGI(TAG, "Requested PDO: %dV", VoltageCodeToVolt((uint8_t)pdo));
    return ESP_OK;
}

esp_err_t Husb238::RefreshSourceCapabilities()
{
    esp_err_t ret = WriteReg(Reg_GO_COMMAND, CMD_Get_SRC_Cap);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Sent Get_SRC_Cap");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to send Get_SRC_Cap");
    }
    return ret;
}

esp_err_t Husb238::HardReset()
{
    esp_err_t ret = WriteReg(Reg_GO_COMMAND, CMD_Hard_Reset);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Sent Hard_Reset");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to send Hard_Reset");
    }
    return ret;
}

bool Husb238::IsAttached()
{
    uint8_t status1 = 0;
    if (ReadReg(Reg_PD_STATUS1, &status1) != ESP_OK) return false;
    return (status1 >> 6) & 0x01;  // D6
}

uint8_t Husb238::GetPDResponse()
{
    uint8_t status1 = 0;
    if (ReadReg(Reg_PD_STATUS1, &status1) != ESP_OK) return 0xFF;  // 错误码
    return (status1 >> 3) & 0x07;                                  // D[5:3]
}

bool Husb238::IsCCDirFlip()
{
    uint8_t status1 = 0;
    if (ReadReg(Reg_PD_STATUS1, &status1) != ESP_OK) return false;
    return (status1 >> 7) & 0x01;  // D7
}

float Husb238::CurrentCodeToAmp(uint8_t code)
{
    switch (code & 0x0F)
    {
        case 0x0:
            return 0.5f;
        case 0x1:
            return 1.0f;
        case 0x2:
            return 1.25f;
        case 0x3:
            return 1.5f;
        case 0x4:
            return 1.75f;
        case 0x5:
            return 2.0f;
        case 0x6:
            return 2.25f;
        case 0x7:
            return 2.5f;
        case 0x8:
            return 2.75f;
        case 0x9:
            return 3.0f;
        case 0xA:
            return 3.25f;
        case 0xB:
            return 3.5f;
        case 0xC:
            return 3.75f;
        case 0xD:
            return 4.0f;
        case 0xE:
            return 4.5f;
        case 0xF:
            return 5.0f;
        default:
            return 0.0f;
    }
}

uint16_t Husb238::VoltageCodeToVolt(uint8_t code)
{
    switch (code & 0x0F)
    {
        case 0x1:
            return 5;
        case 0x2:
            return 9;
        case 0x3:
            return 12;
        case 0x4:
            return 15;
        case 0x5:
            return 18;
        case 0x6:
            return 20;
        default:
            return 0;
    }
}