#include "gfx_driver.h"

// ================== 极简 5x7 ASCII 字体（32~127） ==================
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00},  // 32 ' '
    {0x00, 0x00, 0x5F, 0x00, 0x00},  // 33 '!'
    {0x00, 0x07, 0x00, 0x07, 0x00},  // 34 '"'
    {0x14, 0x7F, 0x14, 0x7F, 0x14},  // 35 '#'
    {0x24, 0x2A, 0x7F, 0x2A, 0x12},  // 36 '$'
    {0x23, 0x13, 0x08, 0x64, 0x62},  // 37 '%'
    {0x36, 0x49, 0x55, 0x22, 0x50},  // 38 '&'
    {0x00, 0x05, 0x03, 0x00, 0x00},  // 39 '''
    {0x00, 0x1C, 0x22, 0x41, 0x00},  // 40 '('
    {0x00, 0x41, 0x22, 0x1C, 0x00},  // 41 ')'
    {0x08, 0x2A, 0x1C, 0x2A, 0x08},  // 42 '*'
    {0x08, 0x08, 0x3E, 0x08, 0x08},  // 43 '+'
    {0x00, 0x50, 0x30, 0x00, 0x00},  // 44 ','
    {0x08, 0x08, 0x08, 0x08, 0x08},  // 45 '-'
    {0x00, 0x60, 0x60, 0x00, 0x00},  // 46 '.'
    {0x20, 0x10, 0x08, 0x04, 0x02},  // 47 '/'
    {0x3E, 0x51, 0x49, 0x45, 0x3E},  // 48 '0'
    {0x00, 0x42, 0x7F, 0x40, 0x00},  // 49 '1'
    {0x42, 0x61, 0x51, 0x49, 0x46},  // 50 '2'
    {0x21, 0x41, 0x45, 0x4B, 0x31},  // 51 '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10},  // 52 '4'
    {0x27, 0x45, 0x45, 0x45, 0x39},  // 53 '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x30},  // 54 '6'
    {0x01, 0x71, 0x09, 0x05, 0x03},  // 55 '7'
    {0x36, 0x49, 0x49, 0x49, 0x36},  // 56 '8'
    {0x06, 0x49, 0x49, 0x29, 0x1E},  // 57 '9'
    {0x00, 0x36, 0x36, 0x00, 0x00},  // 58 ':'
    {0x00, 0x56, 0x36, 0x00, 0x00},  // 59 ';'
    {0x00, 0x08, 0x14, 0x22, 0x41},  // 60 '<'
    {0x14, 0x14, 0x14, 0x14, 0x14},  // 61 '='
    {0x41, 0x22, 0x14, 0x08, 0x00},  // 62 '>'
    {0x02, 0x01, 0x51, 0x09, 0x06},  // 63 '?'
    {0x32, 0x49, 0x79, 0x41, 0x3E},  // 64 '@'
    {0x7E, 0x11, 0x11, 0x11, 0x7E},  // 65 'A'
    {0x7F, 0x49, 0x49, 0x49, 0x36},  // 66 'B'
    {0x3E, 0x41, 0x41, 0x41, 0x22},  // 67 'C'
    {0x7F, 0x41, 0x41, 0x22, 0x1C},  // 68 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41},  // 69 'E'
    {0x7F, 0x09, 0x09, 0x01, 0x01},  // 70 'F'
    {0x3E, 0x41, 0x41, 0x51, 0x32},  // 71 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F},  // 72 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00},  // 73 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01},  // 74 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41},  // 75 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40},  // 76 'L'
    {0x7F, 0x02, 0x04, 0x02, 0x7F},  // 77 'M'
    {0x7F, 0x04, 0x08, 0x10, 0x7F},  // 78 'N'
    {0x3E, 0x41, 0x41, 0x41, 0x3E},  // 79 'O'
    {0x7F, 0x09, 0x09, 0x09, 0x06},  // 80 'P'
    {0x3E, 0x41, 0x51, 0x21, 0x5E},  // 81 'Q'
    {0x7F, 0x09, 0x19, 0x29, 0x46},  // 82 'R'
    {0x46, 0x49, 0x49, 0x49, 0x31},  // 83 'S'
    {0x01, 0x01, 0x7F, 0x01, 0x01},  // 84 'T'
    {0x3F, 0x40, 0x40, 0x40, 0x3F},  // 85 'U'
    {0x1F, 0x20, 0x40, 0x20, 0x1F},  // 86 'V'
    {0x7F, 0x20, 0x18, 0x20, 0x7F},  // 87 'W'
    {0x63, 0x14, 0x08, 0x14, 0x63},  // 88 'X'
    {0x03, 0x04, 0x78, 0x04, 0x03},  // 89 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43},  // 90 'Z'
    {0x00, 0x00, 0x7F, 0x41, 0x00},  // 91 '['
    {0x02, 0x04, 0x08, 0x10, 0x20},  // 92 '\'
    {0x00, 0x41, 0x7F, 0x00, 0x00},  // 93 ']'
    {0x04, 0x02, 0x01, 0x02, 0x04},  // 94 '^'
    {0x40, 0x40, 0x40, 0x40, 0x40},  // 95 '_'
    {0x00, 0x01, 0x02, 0x04, 0x00},  // 96 '`'
    {0x20, 0x54, 0x54, 0x54, 0x78},  // 97 'a'
    {0x7F, 0x48, 0x44, 0x44, 0x38},  // 98 'b'
    {0x38, 0x44, 0x44, 0x44, 0x20},  // 99 'c'
    {0x38, 0x44, 0x44, 0x48, 0x7F},  // 100 'd'
    {0x38, 0x54, 0x54, 0x54, 0x18},  // 101 'e'
    {0x08, 0x7E, 0x09, 0x01, 0x02},  // 102 'f'
    {0x08, 0x14, 0x54, 0x54, 0x3C},  // 103 'g'
    {0x7F, 0x08, 0x04, 0x04, 0x78},  // 104 'h'
    {0x00, 0x44, 0x7D, 0x40, 0x00},  // 105 'i'
    {0x20, 0x40, 0x44, 0x3D, 0x00},  // 106 'j'
    {0x00, 0x7F, 0x10, 0x28, 0x44},  // 107 'k'
    {0x00, 0x41, 0x7F, 0x40, 0x00},  // 108 'l'
    {0x7C, 0x04, 0x18, 0x04, 0x78},  // 109 'm'
    {0x7C, 0x08, 0x04, 0x04, 0x78},  // 110 'n'
    {0x38, 0x44, 0x44, 0x44, 0x38},  // 111 'o'
    {0x7C, 0x14, 0x14, 0x14, 0x08},  // 112 'p'
    {0x08, 0x14, 0x14, 0x18, 0x7C},  // 113 'q'
    {0x7C, 0x08, 0x04, 0x04, 0x08},  // 114 'r'
    {0x48, 0x54, 0x54, 0x54, 0x20},  // 115 's'
    {0x04, 0x3F, 0x44, 0x40, 0x20},  // 116 't'
    {0x3C, 0x40, 0x40, 0x20, 0x7C},  // 117 'u'
    {0x1C, 0x20, 0x40, 0x20, 0x1C},  // 118 'v'
    {0x3C, 0x40, 0x30, 0x40, 0x3C},  // 119 'w'
    {0x44, 0x28, 0x10, 0x28, 0x44},  // 120 'x'
    {0x0C, 0x50, 0x50, 0x50, 0x3C},  // 121 'y'
    {0x44, 0x64, 0x54, 0x4C, 0x44},  // 122 'z'
    {0x00, 0x08, 0x36, 0x41, 0x00},  // 123 '{'
    {0x00, 0x00, 0x7F, 0x00, 0x00},  // 124 '|'
    {0x00, 0x41, 0x36, 0x08, 0x00},  // 125 '}'
    {0x08, 0x08, 0x2A, 0x1C, 0x08},  // 126 '~'
};

// ================== 实现 ==================

GfxDriver::GfxDriver()
{
    clear(Colors::BLACK);
}

void GfxDriver::clear(const Color& c)
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            fb_[y][x][0] = c.r;
            fb_[y][x][1] = c.g;
            fb_[y][x][2] = c.b;
        }
    }
}

const uint8_t* GfxDriver::getPixelPtr(int x, int y) const
{
    return fb_[y][x];
}

void GfxDriver::transform(int x, int y, int* out_x, int* out_y) const
{
    switch (rotation_)
    {
        case 0:
            *out_x = x;
            *out_y = y;
            break;
        case 1:
            *out_x = HEIGHT - 1 - y;
            *out_y = x;
            break;
        case 2:
            *out_x = WIDTH - 1 - x;
            *out_y = HEIGHT - 1 - y;
            break;
        case 3:
            *out_x = y;
            *out_y = WIDTH - 1 - x;
            break;
        default:
            *out_x = x;
            *out_y = y;
            break;
    }
}

void GfxDriver::writePixelRaw(int x, int y, const Color& c)
{
    fb_[y][x][0] = c.r;
    fb_[y][x][1] = c.g;
    fb_[y][x][2] = c.b;
}

void GfxDriver::drawPixel(int x, int y, const Color& c)
{
    if (x < 0 || x >= width() || y < 0 || y >= height()) return;
    int tx, ty;
    transform(x, y, &tx, &ty);
    writePixelRaw(tx, ty, c);
}

Color GfxDriver::getPixel(int x, int y) const
{
    if (x < 0 || x >= width() || y < 0 || y >= height()) return Colors::BLACK;
    int tx, ty;
    transform(x, y, &tx, &ty);
    return Color(fb_[ty][tx][0], fb_[ty][tx][1], fb_[ty][tx][2]);
}

void GfxDriver::drawFastVLine(int x, int y, int h, const Color& c)
{
    if (x < 0 || x >= width()) return;
    if (y < 0)
    {
        h += y;
        y = 0;
    }
    if (y + h > height()) h = height() - y;
    if (h <= 0) return;
    for (int i = y; i < y + h; i++) drawPixel(x, i, c);
}

void GfxDriver::drawFastHLine(int x, int y, int w, const Color& c)
{
    if (y < 0 || y >= height()) return;
    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if (x + w > width()) w = width() - x;
    if (w <= 0) return;
    for (int i = x; i < x + w; i++) drawPixel(i, y, c);
}

void GfxDriver::drawLine(int x0, int y0, int x1, int y1, const Color& c)
{
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1), sx = (x0 < x1) ? 1 : -1;
    int dy = (y1 > y0) ? (y0 - y1) : (y1 - y0), sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;
    while (true)
    {
        drawPixel(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void GfxDriver::drawRect(int x, int y, int w, int h, const Color& c)
{
    drawFastHLine(x, y, w, c);
    drawFastHLine(x, y + h - 1, w, c);
    drawFastVLine(x, y, h, c);
    drawFastVLine(x + w - 1, y, h, c);
}

void GfxDriver::fillRect(int x, int y, int w, int h, const Color& c)
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
    if (x + w > width()) w = width() - x;
    if (y + h > height()) h = height() - y;
    if (w <= 0 || h <= 0) return;
    for (int j = y; j < y + h; j++)
    {
        for (int i = x; i < x + w; i++)
        {
            drawPixel(i, j, c);
        }
    }
}

void GfxDriver::drawCircle(int x0, int y0, int r, const Color& c)
{
    int x = -r, y = 0, err = 2 - 2 * r;
    do
    {
        drawPixel(x0 - x, y0 + y, c);
        drawPixel(x0 - y, y0 - x, c);
        drawPixel(x0 + x, y0 - y, c);
        drawPixel(x0 + y, y0 + x, c);
        int e2 = err;
        if (e2 > x) err += ++x * 2 + 1;
        if (e2 <= y) err += ++y * 2 + 1;
    } while (x < 0);
}

void GfxDriver::fillCircle(int x0, int y0, int r, const Color& c)
{
    drawFastVLine(x0, y0 - r, 2 * r + 1, c);
    int x = -r, y = 0, err = 2 - 2 * r;
    do
    {
        drawFastVLine(x0 - x, y0 - y, 2 * y + 1, c);
        drawFastVLine(x0 + x, y0 - y, 2 * y + 1, c);
        int e2 = err;
        if (e2 > x) err += ++x * 2 + 1;
        if (e2 <= y) err += ++y * 2 + 1;
    } while (x < 0);
}

void GfxDriver::drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, const Color& c)
{
    drawLine(x0, y0, x1, y1, c);
    drawLine(x1, y1, x2, y2, c);
    drawLine(x2, y2, x0, y0, c);
}

void GfxDriver::fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, const Color& c)
{
    int a, b, y, last;
    if (y0 > y1)
    {
        int t;
        t = x0;
        x0 = x1;
        x1 = t;
        t = y0;
        y0 = y1;
        y1 = t;
    }
    if (y1 > y2)
    {
        int t;
        t = x1;
        x1 = x2;
        x2 = t;
        t = y1;
        y1 = y2;
        y2 = t;
    }
    if (y0 > y1)
    {
        int t;
        t = x0;
        x0 = x1;
        x1 = t;
        t = y0;
        y0 = y1;
        y1 = t;
    }

    if (y0 == y2) return;
    int dx01 = x1 - x0, dy01 = y1 - y0;
    int dx02 = x2 - x0, dy02 = y2 - y0;
    int dx12 = x2 - x1, dy12 = y2 - y1;
    int sa = 0, sb = 0;
    if (y1 == y2)
        last = y1;
    else
        last = y1 - 1;

    for (y = y0; y <= last; y++)
    {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        if (a > b)
        {
            int t = a;
            a = b;
            b = t;
        }
        drawFastHLine(a, y, b - a + 1, c);
    }

    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++)
    {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        if (a > b)
        {
            int t = a;
            a = b;
            b = t;
        }
        drawFastHLine(a, y, b - a + 1, c);
    }
}

void GfxDriver::drawBitmap(int x, int y, const uint8_t* bitmap, int w, int h, const Color& color, const Color* bg)
{
    int byteWidth = (w + 7) / 8;
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            int byteIdx = j * byteWidth + i / 8;
            uint8_t bitMask = 0x80 >> (i & 7);  // MSB first
            if (bitmap[byteIdx] & bitMask)
            {
                drawPixel(x + i, y + j, color);
            }
            else if (bg)
            {
                drawPixel(x + i, y + j, *bg);
            }
        }
    }
}

void GfxDriver::drawXBitmap(int x, int y, const uint8_t* bitmap, int w, int h, const Color& color)
{
    int byteWidth = (w + 7) / 8;
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            int byteIdx = j * byteWidth + i / 8;
            uint8_t bitMask = 1 << (i & 7);  // LSB first (XBM)
            if (bitmap[byteIdx] & bitMask)
            {
                drawPixel(x + i, y + j, color);
            }
        }
    }
}

void GfxDriver::drawRGBBitmap(int x, int y, const Color* bitmap, int w, int h)
{
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            drawPixel(x + i, y + j, bitmap[j * w + i]);
        }
    }
}

void GfxDriver::drawChar(int x, int y, char c, const Color& color, const Color& bg, int size)
{
    if (c < 32 || c > 127) c = 32;
    const uint8_t* chr = font5x7[c - 32];
    for (int i = 0; i < 5; i++)
    {
        uint8_t line = chr[i];
        for (int j = 0; j < 7; j++, line >>= 1)
        {
            if (line & 1)
            {
                if (size == 1)
                    drawPixel(x + i, y + j, color);
                else
                    fillRect(x + i * size, y + j * size, size, size, color);
            }
            else if (bg != Colors::BLACK)
            {  // 可选背景填充
                if (size == 1)
                    drawPixel(x + i, y + j, bg);
                else
                    fillRect(x + i * size, y + j * size, size, size, bg);
            }
        }
    }
}

void GfxDriver::drawString(int x, int y, const char* str, const Color& color, const Color& bg, int size)
{
    int xo = x;
    while (*str)
    {
        if (*str == '\n')
        {
            x = xo;
            y += 8 * size;
        }
        else
        {
            drawChar(x, y, *str, color, bg, size);
            x += 6 * size;
        }
        str++;
    }
}

void GfxDriver::setRotation(uint8_t r)
{
    rotation_ = r & 3;
}