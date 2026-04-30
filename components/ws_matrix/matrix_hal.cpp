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
    ESP_LOGI(TAG, "Matrix init: %dx%d (%d LEDs), brightness=%d/255", MATRIX_WIDTH, MATRIX_HEIGHT, LED_STRIP_LED_COUNT,
             brightness_);

    // 启动后先全屏暗白色，确认硬件通电
    gfx_.clear(Color(5, 5, 5));
    Refresh();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gfx_.clear(Color(0, 0, 0));
    Refresh();
}

void MatrixHal::Refresh()
{
    for (int y = 0; y < MATRIX_HEIGHT; y++)
    {
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            const uint8_t* p = gfx_.getPixelPtr(x, y);
            int idx = XYToIndex(x, y);

            // 应用全局亮度
            uint8_t r = ScaleBrightness(p[0], brightness_);
            uint8_t g = ScaleBrightness(p[1], brightness_);
            uint8_t b = ScaleBrightness(p[2], brightness_);

            led_strip_set_pixel(led_strip_, idx, r, g, b);
        }
    }
    ESP_ERROR_CHECK(led_strip_refresh(led_strip_));
}

void MatrixHal::RefreshArea(int x, int y, int w, int h)
{
    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if (y < 0)
    {
        h += y;
        y = 0;
    }
    if (x + w > MATRIX_WIDTH) w = MATRIX_WIDTH - x;
    if (y + h > MATRIX_HEIGHT) h = MATRIX_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    for (int j = y; j < y + h; j++)
    {
        for (int i = x; i < x + w; i++)
        {
            const uint8_t* p = gfx_.getPixelPtr(i, j);
            int idx = XYToIndex(i, j);
            uint8_t r = ScaleBrightness(p[0], brightness_);
            uint8_t g = ScaleBrightness(p[1], brightness_);
            uint8_t b = ScaleBrightness(p[2], brightness_);
            led_strip_set_pixel(led_strip_, idx, r, g, b);
        }
    }
    ESP_ERROR_CHECK(led_strip_refresh(led_strip_));
}

void MatrixHal::ShowRaw(const uint8_t* rgb_data)
{
    for (int i = 0; i < LED_STRIP_LED_COUNT; i++)
    {
        uint8_t r = ScaleBrightness(rgb_data[i * 3 + 0], brightness_);
        uint8_t g = ScaleBrightness(rgb_data[i * 3 + 1], brightness_);
        uint8_t b = ScaleBrightness(rgb_data[i * 3 + 2], brightness_);
        led_strip_set_pixel(led_strip_, i, r, g, b);
    }
    ESP_ERROR_CHECK(led_strip_refresh(led_strip_));
}

void MatrixHal::SetBrightness(uint8_t brightness)
{
    brightness_ = brightness;
    ESP_LOGI(TAG, "Brightness set to %d/255", brightness_);
}