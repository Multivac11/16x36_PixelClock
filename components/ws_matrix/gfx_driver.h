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
    static constexpr int WIDTH = 36;
    static constexpr int HEIGHT = 16;
    static constexpr int PIXELS = WIDTH * HEIGHT;

    GfxDriver();

    // ---------- 颜色工具 ----------
    static Color ScaleColor(const Color& c, uint8_t brightness);

    // ---------- 缓冲区 ----------
    void clear(const Color& c = Colors::BLACK);
    void fill(const Color& c) { clear(c); }

    const uint8_t* getPixelPtr(int x, int y) const;
    const uint8_t* buffer() const { return &fb_[0][0][0]; }

    // ---------- 基本绘制（全部支持 brightness，默认 255 即不缩放） ----------
    void drawPixel(int x, int y, const Color& c, uint8_t brightness = 255);

    // ---------- 直线 ----------
    void drawLine(int x0, int y0, int x1, int y1, const Color& c, uint8_t brightness = 255);
    void drawFastVLine(int x, int y, int h, const Color& c, uint8_t brightness = 255);
    void drawFastHLine(int x, int y, int w, const Color& c, uint8_t brightness = 255);

    // ---------- 矩形 ----------
    void drawRect(int x, int y, int w, int h, const Color& c, uint8_t brightness = 255);
    void fillRect(int x, int y, int w, int h, const Color& c, uint8_t brightness = 255);

    // ---------- 圆 ----------
    void drawCircle(int x0, int y0, int r, const Color& c, uint8_t brightness = 255);
    void fillCircle(int x0, int y0, int r, const Color& c, uint8_t brightness = 255);

    // ---------- 三角形 ----------
    void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, const Color& c, uint8_t brightness = 255);
    void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, const Color& c, uint8_t brightness = 255);

    // ---------- 位图 ----------
    void drawBitmap(int x,
                    int y,
                    const uint8_t* bitmap,
                    int w,
                    int h,
                    const Color& color,
                    const Color* bg = nullptr,
                    uint8_t brightness = 255);
    void drawXBitmap(int x, int y, const uint8_t* bitmap, int w, int h, const Color& color, uint8_t brightness = 255);
    void drawRGBBitmap(int x, int y, const Color* bitmap, int w, int h, uint8_t brightness = 255);

    // ---------- 文字 ----------
    void drawChar(int x, int y, char c, const Color& color, const Color& bg, int size = 1, uint8_t brightness = 255);
    void drawString(
        int x, int y, const char* str, const Color& color, const Color& bg, int size = 1, uint8_t brightness = 255);

    // ---------- 旋转 ----------
    void setRotation(uint8_t r);
    uint8_t getRotation() const { return rotation_; }

    int width() const { return (rotation_ % 2 == 0) ? WIDTH : HEIGHT; }
    int height() const { return (rotation_ % 2 == 0) ? HEIGHT : WIDTH; }

    Color getPixel(int x, int y) const;

   private:
    uint8_t fb_[HEIGHT][WIDTH][3];
    uint8_t rotation_ = 0;

    void transform(int x, int y, int* out_x, int* out_y) const;
    void writePixelRaw(int x, int y, const Color& c);
};