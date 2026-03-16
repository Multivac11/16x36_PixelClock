#include "i2c.h"

void I2CMaster::InitI2C()
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {.enable_internal_pullup = false, .allow_pd = false}};

    esp_err_t ret = i2c_new_master_bus(&bus_config, &bus_handle_);
    if (ret != ESP_OK)
    {
        ESP_LOGE("I2CMaster", "I2C bus initialization failed");
        return;
    }
    ESP_LOGI("I2CMaster", "I2C bus initialization successfull!");
}
