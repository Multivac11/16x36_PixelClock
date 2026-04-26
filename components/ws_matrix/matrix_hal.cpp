#include "matrix_hal.h"

static const char* TAG = "WsMatrix";

void MatrixHal::MatrixHalInit()
{
    led_strip_config_t strip_config = {.strip_gpio_num = LED_STRIP_GPIO_PIN,
                                       .max_leds = LED_STRIP_LED_COUNT,
                                       .led_model = LED_MODEL_WS2812,
                                       .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
                                       .flags = {.invert_out = false}};

    led_strip_rmt_config_t rmt_config = {.clk_src = RMT_CLK_SRC_DEFAULT,
                                         .resolution_hz = LED_STRIP_RMT_RES_HZ,
                                         .mem_block_symbols = LED_STRIP_MEMORY_BLOCK_WORDS,
                                         .flags = {.with_dma = LED_STRIP_USE_DMA}};

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_));
    ESP_LOGI(TAG, "Matrix init: %dx%d (%d LEDs)", MATRIX_WIDTH, MATRIX_HEIGHT, LED_STRIP_LED_COUNT);

    // 启动后先全屏暗白色，确认硬件通电
    gfx_.clear(Color(5, 5, 5));
    Refresh();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gfx_.clear(Color(0, 0, 0));
    Refresh();
}

void MatrixHal::Refresh()
{
    // 遍历物理坐标，通过 LUT 映射到 LED 索引，发送整帧
    for (int y = 0; y < MATRIX_HEIGHT; y++)
    {
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            const uint8_t* p = gfx_.getPixelPtr(x, y);  // RGB 指针
            int idx = XYToIndex(x, y);
            // led_strip 内部根据 GRB 格式自动重排，这里直接传 RGB
            led_strip_set_pixel(led_strip_, idx, p[0], p[1], p[2]);
        }
    }
    ESP_ERROR_CHECK(led_strip_refresh(led_strip_));
}
