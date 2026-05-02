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
#define LED_STRIP_GPIO_PIN 16
#define LED_STRIP_RMT_RES_HZ (10 * 1000 * 1000)

// 查表法：x 坐标到块内基址的映射，消除运行时除法
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
     * @brief 初始化矩阵硬件，包括 RMT、LED 条等
     */
    void MatrixHalInit();

    /**
     * @brief 刷新当前 Gfx 帧缓冲到 WS2812 硬件
     */
    void Refresh();

    /**
     * @brief 刷新指定区域的 Gfx 帧缓冲到 WS2812 硬件
     * @param x 区域左上角 x 坐标
     * @param y 区域左上角 y 坐标
     * @param w 区域宽度
     * @param h 区域高度
     */
    void RefreshArea(int x, int y, int w, int h);

    /**
     * @brief 获取 GfxDriver 实例，用于绘图
     * @return GfxDriver& 引用到 GfxDriver 实例
     */
    GfxDriver& Gfx() { return gfx_; }

    /**
     * @brief 获取 GfxDriver 实例，用于绘图（常量引用）
     * @return const GfxDriver& 引用到 GfxDriver 实例
     */
    const GfxDriver& Gfx() const { return gfx_; }

    /**
     * @brief 显示原始 RGB 数据到矩阵
     * @param rgb_data 指向原始 RGB 数据的指针，每个像素 3 个字节（R、G、B）
     */
    void ShowRaw(const uint8_t* rgb_data);

    /**
     * @brief 设置矩阵亮度
     * @param brightness 亮度值，范围 0~255
     */
    void SetBrightness(uint8_t brightness);

    /**
     * @brief 获取当前矩阵亮度
     * @return uint8_t 当前亮度值，范围 0~255
     */
    uint8_t GetBrightness() const { return brightness_; }

   private:
    MatrixHal() = default;
    ~MatrixHal() = default;

    /**
     * @brief 物理坐标 (x,y) 映射到 LED 串联索引
     * @param x 物理 x 坐标
     * @param y 物理 y 坐标
     * @return int LED 串联索引
     */
    static inline int XYToIndex(int x, int y) { return X_LUT[x] + y * BLOCK_WIDTH; }

   private:
    // LED 串联索引映射表
    int index_map_[MATRIX_HEIGHT][MATRIX_WIDTH];

    // 亮度映射表
    uint8_t brightness_lut_[256];

    // GfxDriver 实例，用于绘图
    GfxDriver gfx_;

    // 当前亮度值
    uint8_t brightness_ = 255;

    // LED 条句柄
    led_strip_handle_t led_strip_ = nullptr;
};