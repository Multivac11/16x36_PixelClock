#include "husb238.h"

static const char *TAG = "HUSB238";

HUSB238::HUSB238(i2c_master_bus_handle_t bus, uint16_t addr) : I2CDevice(bus, addr, TAG)
{
}

bool HUSB238::Init()
{
    if (!I2CDevice::Init())
    {
        return false;
    }

    GetSourceCapabilities();

    ESP_LOGI(TAG, "Init HUSB238 success");
    return true;
}

bool HUSB238::GetCurrentPDO(uint16_t *voltage, float *current)
{
    if (!voltage || !current)
    {
        return false;
    }

    uint8_t status0;
    uint8_t reg_addr = Reg_PD_STATUS0;
    esp_err_t ret = WriteThenRead(&reg_addr, 1, &status0, 1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Read PD_STATUS0 failed: %d", ret);
        return false;
    }

    uint8_t volt_code = (status0 >> 4) & 0x0F;  // D[7:4]
    uint8_t curr_code = status0 & 0x0F;         // D[3:0]

    *voltage = VoltageCodeToVolt(volt_code);
    *current = CurrentCodeToAmp(curr_code);

    ESP_LOGI(TAG, "Current PDO: %uV %.2fA (raw=0x%02X)", *voltage, *current, status0);

    return true;
}

void HUSB238::GetSourceCapabilities()
{
    Capability_t caps[6];
    uint16_t voltages[6] = {5, 9, 12, 15, 18, 20};
    uint8_t regs[6] = {Reg_SRC_PDO_5V,  Reg_SRC_PDO_9V,  Reg_SRC_PDO_12V,
                       Reg_SRC_PDO_15V, Reg_SRC_PDO_18V, Reg_SRC_PDO_20V};

    for (int i = 0; i < 6; i++)
    {
        uint8_t val = 0;
        if (WriteThenRead(&regs[i], 1, &val, 1) != ESP_OK)
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

esp_err_t HUSB238::RequestVoltage(HUSB238_SELECT_Voltage_e pdo)
{
    if (pdo == PDO_NotSel)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buf[2] = {Reg_GO_COMMAND, CMD_Request_PDO};
    esp_err_t ret = Write(buf, 2);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to write GO_COMMAND");
        return ret;
    }

    ESP_LOGI(TAG, "Requested PDO: %dV", VoltageCodeToVolt((uint8_t)pdo));
    return ESP_OK;
}

esp_err_t HUSB238::RefreshSourceCapabilities()
{
    uint8_t buf[2] = {Reg_GO_COMMAND, CMD_Get_SRC_Cap};
    esp_err_t ret = Write(buf, 2);
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

esp_err_t HUSB238::HardReset()
{
    uint8_t buf[2] = {Reg_GO_COMMAND, CMD_Hard_Reset};
    esp_err_t ret = Write(buf, 2);
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

bool HUSB238::IsAttached()
{
    uint8_t status1 = 0;
    uint8_t reg_addr = Reg_PD_STATUS1;

    if (WriteThenRead(&reg_addr, 1, &status1, 1) != ESP_OK)
    {
        return false;
    }
    return (status1 >> 6) & 0x01;  // D6
}

uint8_t HUSB238::GetPDResponse()
{
    uint8_t status1 = 0;
    uint8_t reg_addr = Reg_PD_STATUS1;
    if (WriteThenRead(&reg_addr, 1, &status1, 1) != ESP_OK)
    {
        return 0xFF;  // 错误码
    }
    return (status1 >> 3) & 0x07;  // D[5:3]
}

bool HUSB238::IsCCDirFlip()
{
    uint8_t status1 = 0;
    uint8_t reg_addr = Reg_PD_STATUS1;
    if (WriteThenRead(&reg_addr, 1, &status1, 1) != ESP_OK)
    {
        return false;
    }
    return (status1 >> 7) & 0x01;  // D7
}

float HUSB238::CurrentCodeToAmp(uint8_t code)
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

uint16_t HUSB238::VoltageCodeToVolt(uint8_t code)
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
