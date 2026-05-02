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

    // 预计算每个 (x,y) 对应的硬件索引
    for (int y = 0; y < MATRIX_HEIGHT; ++y)
    {
        for (int x = 0; x < MATRIX_WIDTH; ++x)
        {
            index_map_[y][x] = XYToIndex(x, y);
        }
    }

    // 初始化亮度 LUT
    SetBrightness(brightness_);

    // 启动后缓慢变亮再变暗，确认硬件通电
    const uint8_t peak = 30;
    const int steps = 50;
    const int delay_ms = 5;

    for (int i = 0; i <= steps; ++i)
    {
        uint8_t v = (uint8_t)(peak * i / steps);
        gfx_.clear(Color(v, v, v));
        Refresh();
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

void MatrixHal::Refresh()
{
    const uint8_t* lut = brightness_lut_;
    for (int y = 0; y < MATRIX_HEIGHT; ++y)
    {
        for (int x = 0; x < MATRIX_WIDTH; ++x)
        {
            const uint8_t* p = gfx_.getPixelPtr(x, y);
            int idx = index_map_[y][x];
            led_strip_set_pixel(led_strip_, idx, lut[p[0]], lut[p[1]], lut[p[2]]);
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

    const uint8_t* lut = brightness_lut_;
    for (int j = y; j < y + h; ++j)
    {
        for (int i = x; i < x + w; ++i)
        {
            const uint8_t* p = gfx_.getPixelPtr(i, j);
            int idx = index_map_[j][i];
            led_strip_set_pixel(led_strip_, idx, lut[p[0]], lut[p[1]], lut[p[2]]);
        }
    }
    ESP_ERROR_CHECK(led_strip_refresh(led_strip_));
}

void MatrixHal::ShowRaw(const uint8_t* rgb_data)
{
    const uint8_t* lut = brightness_lut_;
    for (int i = 0; i < LED_STRIP_LED_COUNT; ++i)
    {
        uint8_t r = lut[rgb_data[i * 3 + 0]];
        uint8_t g = lut[rgb_data[i * 3 + 1]];
        uint8_t b = lut[rgb_data[i * 3 + 2]];
        led_strip_set_pixel(led_strip_, i, r, g, b);
    }
    ESP_ERROR_CHECK(led_strip_refresh(led_strip_));
}

void MatrixHal::SetBrightness(uint8_t brightness)
{
    brightness_ = brightness;
    for (int i = 0; i < 256; ++i)
    {
        brightness_lut_[i] = (uint16_t)i * brightness / 255;
    }
    ESP_LOGI(TAG, "Brightness set to %d/255", brightness_);
}