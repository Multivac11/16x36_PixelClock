#pragma once

#include <cstdint>
#include <cstring>

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

class GfxDriver
{
   public:
    static constexpr int WIDTH = 36;
    static constexpr int HEIGHT = 16;
    static constexpr int PIXELS = WIDTH * HEIGHT;

    /**
     * @brief 构造函数
     */
    GfxDriver();

    /**
     * @brief 缩放颜色亮度
     * @param c 输入颜色
     * @param brightness 亮度值，范围 0-255
     * @return Color 缩放后的颜色
     */
    static Color ScaleColor(const Color& c, uint8_t brightness);

    /**
     * @brief 清除缓冲区，填充指定颜色
     * @param c 填充颜色
     */
    void clear(const Color& c = Colors::BLACK);

    /**
     * @brief 填充缓冲区，填充指定颜色
     * @param c 填充颜色
     */
    void fill(const Color& c) { clear(c); }

    /**
     * @brief 获取指定像素的指针
     * @param x 像素 x 坐标
     * @param y 像素 y 坐标
     * @return const uint8_t* 惏素指针
     */
    const uint8_t* getPixelPtr(int x, int y) const;

    /**
     * @brief 获取缓冲区指针
     * @return const uint8_t* 缓冲区指针
     */
    const uint8_t* buffer() const { return &fb_[0][0][0]; }

    /**
     * @brief 绘制像素
     * @param x 像素 x 坐标
     * @param y 像素 y 坐标
     * @param c 像素颜色
     * @param brightness 亮度值，范围 0-255
     */
    inline void drawPixel(int x, int y, const Color& c, uint8_t brightness = 255)
    {
        if ((uint16_t)x >= WIDTH || (uint16_t)y >= HEIGHT) return;
        uint8_t* p = fb_[y][x];
        if (brightness == 255)
        {
            p[0] = c.r;
            p[1] = c.g;
            p[2] = c.b;
        }
        else
        {
            p[0] = (uint16_t)c.r * brightness / 255;
            p[1] = (uint16_t)c.g * brightness / 255;
            p[2] = (uint16_t)c.b * brightness / 255;
        }
    }

    /**
     * @brief 绘制直线
     * @param x0 起始点 x 坐标
     * @param y0 起始点 y 坐标
     * @param x1 终点 x 坐标
     * @param y1 终点 y 坐标
     * @param c 盺线颜色
     * @param brightness 亮度值，范围 0-255
     */
    void drawLine(int x0, int y0, int x1, int y1, const Color& c, uint8_t brightness = 255);

    /**
     * @brief 绘制垂直线
     * @param x 垂直线 x 坐标
     * @param y 垂直线 y 坐标
     * @param h 垂直线高度
     * @param c 垂直线颜色
     * @param brightness 亮度值，范围 0-255
     */
    void drawFastVLine(int x, int y, int h, const Color& c, uint8_t brightness = 255);

    /**
     * @brief 绘制水平线
     * @param x 水平线 x 坐标
     * @param y 水平线 y 坐标
     * @param w 水平线宽度
     * @param c 水平线颜色
     * @param brightness 亮度值，范围 0-255
     */
    void drawFastHLine(int x, int y, int w, const Color& c, uint8_t brightness = 255);

    /**
     * @brief 绘制矩形
     * @param x 矩形 x 坐标
     * @param y 矩形 y 坐标
     * @param w 矩形宽度
     * @param h 矩形高度
     * @param c 矩形颜色
     * @param brightness 亮度值，范围 0-255
     */
    void drawRect(int x, int y, int w, int h, const Color& c, uint8_t brightness = 255);

    /**
     * @brief 绘制填充矩形
     * @param x 矩形 x 坐标
     * @param y 矩形 y 坐标
     * @param w 矩形宽度
     * @param h 矩形高度
     * @param c 矩形颜色
     * @param brightness 亮度值，范围 0-255
     */
    void fillRect(int x, int y, int w, int h, const Color& c, uint8_t brightness = 255);

    /**
     * @brief 绘制圆
     * @param x0 圆心 x 坐标
     * @param y0 圆心 y 坐标
     * @param r 圆半径
     * @param c 圆颜色
     * @param brightness 亮度值，范围 0-255
     */
    void drawCircle(int x0, int y0, int r, const Color& c, uint8_t brightness = 255);

    /**
     * @brief 绘制填充圆
     * @param x0 圆心 x 坐标
     * @param y0 圆心 y 坐标
     * @param r 圆半径
     * @param c 圆颜色
     * @param brightness 亮度值，范围 0-255
     */
    void fillCircle(int x0, int y0, int r, const Color& c, uint8_t brightness = 255);

    /**
     * @brief 绘制三角形
     * @param x0 三角形第一个点 x 坐标
     * @param y0 三角形第一个点 y 坐标
     * @param x1 三角形第二个点 x 坐标
     * @param y1 三角形第二个点 y 坐标
     * @param x2 三角形第三个点 x 坐标
     * @param y2 三角形第三个点 y 坐标
     * @param c 三角形颜色
     * @param brightness 亮度值，范围 0-255
     */
    void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, const Color& c, uint8_t brightness = 255);

    /**
     * @brief 绘制填充三角形
     * @param x0 三角形第一个点 x 坐标
     * @param y0 三角形第一个点 y 坐标
     * @param x1 三角形第二个点 x 坐标
     * @param y1 三角形第二个点 y 坐标
     * @param x2 三角形第三个点 x 坐标
     * @param y2 三角形第三个点 y 坐标
     * @param c 三角形颜色
     * @param brightness 亮度值，范围 0-255
     */
    void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, const Color& c, uint8_t brightness = 255);

    /**
     * @brief 绘制位图
     * @param x 位图 x 坐标
     * @param y 位图 y 坐标
     * @param bitmap 位图数据指针
     * @param w 位图宽度
     * @param h 位图高度
     * @param color 位图颜色
     * @param bg 背景颜色指针
     * @param brightness 亮度值，范围 0-255
     */
    void drawBitmap(int x,
                    int y,
                    const uint8_t* bitmap,
                    int w,
                    int h,
                    const Color& color,
                    const Color* bg = nullptr,
                    uint8_t brightness = 255);

    /**
     * @brief 绘制 X 位图
     * @param x 位图 x 坐标
     * @param y 位图 y 坐标
     * @param bitmap 位图数据指针
     * @param w 位图宽度
     * @param h 位图高度
     * @param color 位图颜色
     * @param brightness 亮度值，范围 0-255
     */
    void drawXBitmap(int x, int y, const uint8_t* bitmap, int w, int h, const Color& color, uint8_t brightness = 255);

    /**
     * @brief 绘制 RGB 位图
     * @param x 位图 x 坐标
     * @param y 位图 y 坐标
     * @param bitmap 位图数据指针
     * @param w 位图宽度
     * @param h 位图高度
     * @param brightness 亮度值，范围 0-255
     */
    void drawRGBBitmap(int x, int y, const Color* bitmap, int w, int h, uint8_t brightness = 255);

    /**
     * @brief 绘制字符
     * @param x 字符 x 坐标
     * @param y 字符 y 坐标
     * @param c 字符
     * @param color 字符颜色
     * @param bg 背景颜色
     * @param size 字符大小
     * @param brightness 亮度值，范围 0-255
     */
    void drawChar(int x, int y, char c, const Color& color, const Color& bg, int size = 1, uint8_t brightness = 255);

    /**
     * @brief 绘制字符串
     * @param x 字符串 x 坐标
     * @param y 字符串 y 坐标
     * @param str 字符串指针
     * @param color 字符颜色
     * @param bg 背景颜色
     * @param size 字符大小
     * @param brightness 亮度值，范围 0-255
     */
    void drawString(
        int x, int y, const char* str, const Color& color, const Color& bg, int size = 1, uint8_t brightness = 255);

    /**
     * @brief 获取矩阵宽度
     * @return 矩阵宽度
     */
    static constexpr int width() { return WIDTH; }

    /**
     * @brief 获取矩阵高度
     * @return 矩阵高度
     */
    static constexpr int height() { return HEIGHT; }

    /**
     * @brief 获取像素颜色
     * @param x 像素 x 坐标
     * @param y 像素 y 坐标
     * @return 像素颜色
     */
    Color getPixel(int x, int y) const;

   private:
    /**
     * @brief 像素缓冲区
     */
    uint8_t fb_[HEIGHT][WIDTH][3];

    /**
     * @brief 写入像素颜色
     * @param x 像素 x 坐标
     * @param y 像素 y 坐标
     * @param c 像素颜色
     */
    inline void writePixelRaw(int x, int y, const Color& c)
    {
        uint8_t* p = fb_[y][x];
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
    }
};