#pragma once

#include <cstdint>
#include <cstring>

// ================== 颜色结构 ==================
struct Color
{
    uint8_t r, g, b;
    constexpr Color(uint8_t rr = 0, uint8_t gg = 0, uint8_t bb = 0) : r(rr), g(gg), b(bb) {}
    bool operator==(const Color& o) const { return r == o.r && g == o.g && b == o.b; }
    bool operator!=(const Color& o) const { return !(*this == o); }
};

namespace Colors {
constexpr Color BLACK(0, 0, 0);
constexpr Color WHITE(255, 255, 255);
constexpr Color RED(255, 0, 0);
constexpr Color GREEN(0, 255, 0);
constexpr Color BLUE(0, 0, 255);
constexpr Color YELLOW(255, 255, 0);
constexpr Color CYAN(0, 255, 255);
constexpr Color MAGENTA(255, 0, 255);
constexpr Color ORANGE(255, 165, 0);
constexpr Color PURPLE(128, 0, 128);
constexpr Color PINK(255, 192, 203);
}  // namespace Colors

// ================== 2D 软件渲染引擎 ==================
class GfxDriver
{
   public:
    static constexpr int WIDTH = 36;   // 物理宽度
    static constexpr int HEIGHT = 16;  // 物理高度
    static constexpr int PIXELS = WIDTH * HEIGHT;

    GfxDriver();

    // ---------- 缓冲区 ----------
    void clear(const Color& c = Colors::BLACK);
    void fill(const Color& c) { clear(c); }

    // 获取物理坐标像素指针（供硬件刷新使用）x:0~35, y:0~15
    const uint8_t* getPixelPtr(int x, int y) const;
    // 获取整帧缓冲区首地址（RGB 交错，共 PIXELS*3 字节）
    const uint8_t* buffer() const { return &fb_[0][0][0]; }

    // ---------- 基本绘制 ----------
    void drawPixel(int x, int y, const Color& c);
    Color getPixel(int x, int y) const;

    // ---------- 直线 ----------
    void drawLine(int x0, int y0, int x1, int y1, const Color& c);
    void drawFastVLine(int x, int y, int h, const Color& c);
    void drawFastHLine(int x, int y, int w, const Color& c);

    // ---------- 矩形 ----------
    void drawRect(int x, int y, int w, int h, const Color& c);  // 空心
    void fillRect(int x, int y, int w, int h, const Color& c);  // 实心

    // ---------- 圆 ----------
    void drawCircle(int x0, int y0, int r, const Color& c);  // 空心
    void fillCircle(int x0, int y0, int r, const Color& c);  // 实心

    // ---------- 三角形 ----------
    void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, const Color& c);
    void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, const Color& c);

    // ---------- 位图（取模图片） ----------
    /**
     * @brief 1-bit 位图（MSB First，标准 Adafruit 格式）
     * @param bitmap 取模数组，每行向上取整到整字节
     * @param color  前景色
     * @param bg     背景色，传 nullptr 则透明（不覆盖原像素）
     */
    void drawBitmap(int x, int y, const uint8_t* bitmap, int w, int h, const Color& color, const Color* bg = nullptr);

    /**
     * @brief XBM 格式位图（LSB First，PCtoLCD/Image2LCD 常用输出格式）
     */
    void drawXBitmap(int x, int y, const uint8_t* bitmap, int w, int h, const Color& color);

    /**
     * @brief 24-bit RGB 位图（const Color 数组，w*h 大小）
     */
    void drawRGBBitmap(int x, int y, const Color* bitmap, int w, int h);

    // ---------- 文字（内置 5x7 ASCII） ----------
    void drawChar(int x, int y, char c, const Color& color, const Color& bg, int size = 1);
    void drawString(int x, int y, const char* str, const Color& color, const Color& bg, int size = 1);

    // ---------- 旋转 ----------
    void setRotation(uint8_t r);  // 0:正常, 1:90°顺时针, 2:180°, 3:270°顺时针
    uint8_t getRotation() const { return rotation_; }

    // 逻辑尺寸（考虑旋转后的绘图区域）
    int width() const { return (rotation_ % 2 == 0) ? WIDTH : HEIGHT; }
    int height() const { return (rotation_ % 2 == 0) ? HEIGHT : WIDTH; }

   private:
    uint8_t fb_[HEIGHT][WIDTH][3];  // [y][x][0:r,1:g,2:b]
    uint8_t rotation_ = 0;

    void transform(int x, int y, int* out_x, int* out_y) const;
    void writePixelRaw(int x, int y, const Color& c);  // 无越界检查，内部使用
};