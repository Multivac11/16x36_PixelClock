#pragma once

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_47 /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_48 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0      /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0   /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0   /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

class I2CMaster
{
   public:
    static I2CMaster &GetInstance()
    {
        static I2CMaster instance;
        return instance;
    }

    I2CMaster() = default;
    ~I2CMaster() = default;

    void InitI2C();

   public:
    i2c_master_bus_handle_t bus_handle_;
    i2c_master_dev_handle_t dev_handle_;
};