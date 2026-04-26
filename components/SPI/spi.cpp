#include "spi.h"

void SpiMaster::InitSpiMaster()
{
    spi_bus_config_t bus_config = {};
    bus_config.flags = SPICOMMON_BUSFLAG_MASTER;
    bus_config.isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO;
    bus_config.max_transfer_sz = 240 * 240 * 2;
    bus_config.miso_io_num = SPI_MISO_GPIO_NUM;
    bus_config.mosi_io_num = SPI_MOSI_GPIO_NUM;
    bus_config.sclk_io_num = SPI_CLK_GPIO_NUM;
    bus_config.quadhd_io_num = -1;
    bus_config.quadwp_io_num = -1;

    spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO);
}
