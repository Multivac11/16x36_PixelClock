#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gfx_driver.h"
#include "led_strip.h"

// ================== 硬件参数 ==================
#define MATRIX_WIDTH 36
#define MATRIX_HEIGHT 16
#define BLOCK_WIDTH 9
#define BLOCK_HEIGHT 16
#define LEDS_PER_BLOCK (BLOCK_WIDTH * BLOCK_HEIGHT)
#define LED_STRIP_LED_COUNT (MATRIX_WIDTH * MATRIX_HEIGHT)
#define LED_STRIP_USE_DMA 1
#define LED_STRIP_MEMORY_BLOCK_WORDS 1024
#define LED_STRIP_GPIO_PIN 17
#define LED_STRIP_RMT_RES_HZ (10 * 1000 * 1000)

// 查表法：x 坐标到块内基址的映射
namespace {
constexpr uint16_t X_LUT[MATRIX_WIDTH] = {0,   1,   2,   3,   4,   5,   6,   7,   8,   144, 145, 146,
                                          147, 148, 149, 150, 151, 152, 288, 289, 290, 291, 292, 293,
                                          294, 295, 296, 432, 433, 434, 435, 436, 437, 438, 439, 440};
}

class MatrixHal
{
   public:
    static MatrixHal& GetInstance()
    {
        static MatrixHal instance;
        return instance;
    }

    /**
     * @brief 初始化矩阵硬件，包括 RMT、LED 条、坐标映射表、亮度 LUT
     */
    void MatrixHalInit();

    /**
     * @brief 刷新当前 Gfx 帧缓冲到 WS2812 硬件
     */
    void Refresh();

    /**
     * @brief 刷新指定区域
     */
    void RefreshArea(int x, int y, int w, int h);

    GfxDriver& Gfx() { return gfx_; }
    const GfxDriver& Gfx() const { return gfx_; }

    void ShowRaw(const uint8_t* rgb_data);
    void SetBrightness(uint8_t brightness);
    uint8_t GetBrightness() const { return brightness_; }

   private:
    MatrixHal() = default;
    ~MatrixHal() = default;

    static inline int XYToIndex(int x, int y) { return X_LUT[x] + y * BLOCK_WIDTH; }

    int index_map_[MATRIX_HEIGHT][MATRIX_WIDTH];
    uint8_t brightness_lut_[256];
    GfxDriver gfx_;
    uint8_t brightness_ = 255;
    led_strip_handle_t led_strip_ = nullptr;
};