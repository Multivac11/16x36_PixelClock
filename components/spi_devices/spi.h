#pragma once

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SPI_MISO_GPIO_NUM GPIO_NUM_13
#define SPI_MOSI_GPIO_NUM GPIO_NUM_11
#define SPI_CLK_GPIO_NUM GPIO_NUM_12

class SpiMaster
{
   public:
    static SpiMaster &GetInstance()
    {
        static SpiMaster instance;

        return instance;
    }
    SpiMaster() = default;

    ~SpiMaster() = default;

    void InitSpiMaster();

   public:
};