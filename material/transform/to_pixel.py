#!/usr/bin/env python3
"""
16x16 像素素材取模脚本 —— 直接映射，零缩放、零插值
输出与 ESP32 MatrixHal 驱动完全兼容的 .bin 文件

支持输入：
  - .png / .jpg / .bmp（必须是 16×16 或会被严格裁剪为 16×16）
  - .svg（自动光栅化为 16×16 像素完美模式）

输出模式：
  - sprite : 768 字节 RGB，配合 drawRGBBitmap(x, y, bitmap, 16, 16) 使用
  - frame  : 1728 字节 RGB，按物理灯序排列，配合 ShowRaw() 使用

依赖：
  pip install Pillow numpy
  如需 SVG 支持：pip install cairosvg
"""

import sys
import io
import argparse
from pathlib import Path
from PIL import Image
import numpy as np

# ================== 与你的 ESP32 硬件完全一致的映射 ==================
MATRIX_W, MATRIX_H = 36, 16
PANEL_W = 9
LED_COUNT = MATRIX_W * MATRIX_H

# XYToIndex LUT，来自 matrix_hal.h
X_LUT = [
    0,   1,   2,   3,   4,   5,   6,   7,   8,
    144, 145, 146, 147, 148, 149, 150, 151, 152,
    288, 289, 290, 291, 292, 293, 294, 295, 296,
    432, 433, 434, 435, 436, 437, 438, 439, 440,
]

def xy_to_index(x: int, y: int) -> int:
    """物理坐标 (x,y) -> LED 串联索引 0~575"""
    return X_LUT[x] + y * PANEL_W

# ================== 核心转换 ==================

def convert_to_sprite(img: Image.Image, transparent_rgb=(0, 0, 0)) -> bytes:
    """
    生成 16×16 精灵图 bin：
    768 字节，行优先 RGB，与 GfxDriver::drawRGBBitmap() 完全对应。
    """
    img = img.convert("RGBA")
    # 严格裁剪为 16×16（从左上角开始，不缩放）
    img = img.crop((0, 0, 16, 16))
    pixels = np.array(img)  # [y][x][r,g,b,a]

    buf = bytearray(16 * 16 * 3)
    for y in range(16):
        for x in range(16):
            r, g, b, a = pixels[y, x]
            if a < 128:
                r, g, b = transparent_rgb
            offset = (y * 16 + x) * 3
            buf[offset + 0] = int(r)
            buf[offset + 1] = int(g)
            buf[offset + 2] = int(b)
    return bytes(buf)


def convert_to_frame(img: Image.Image, offset_x=10, offset_y=0,
                     transparent_rgb=(0, 0, 0)) -> bytes:
    """
    生成 36×16 全屏帧 bin：
    1728 字节，按物理灯序 RGB，与 MatrixHal::ShowRaw() 完全对应。
    16×16 图像默认居中（offset_x=10, offset_y=0），其余区域填黑。
    """
    img = img.convert("RGBA")
    img = img.crop((0, 0, 16, 16))
    pixels = np.array(img)

    buf = bytearray(LED_COUNT * 3)  # 全黑背景

    for y in range(16):
        for x in range(16):
            r, g, b, a = pixels[y, x]
            if a < 128:
                r, g, b = transparent_rgb

            sx = offset_x + x
            sy = offset_y + y
            if 0 <= sx < MATRIX_W and 0 <= sy < MATRIX_H:
                idx = xy_to_index(sx, sy)
                buf[idx * 3 + 0] = int(r)
                buf[idx * 3 + 1] = int(g)
                buf[idx * 3 + 2] = int(b)
    return bytes(buf)


def load_image(path: Path) -> Image.Image:
    """加载图片，如果是 SVG 则先用 cairosvg 转为 PNG。"""
    suffix = path.suffix.lower()
    if suffix == ".svg":
        try:
            import cairosvg
        except ImportError:
            print("错误：需要 cairosvg 来处理 SVG。请执行 pip install cairosvg")
            sys.exit(1)
        # 渲染为 16×16，极小尺寸下默认就是像素化的
        png_data = cairosvg.svg2png(
            url=str(path),
            output_width=16,
            output_height=16,
            scale=1.0,
        )
        return Image.open(io.BytesIO(png_data))
    else:
        return Image.open(path)


# ================== 批量处理 ==================

def process_one(input_path: Path, out_dir: Path, mode: str, transparent_rgb):
    img = load_image(input_path)

    # 如果图片大于 16×16，严格裁剪左上角 16×16；如果更小则报错
    w, h = img.size
    if w < 16 or h < 16:
        print(f"跳过 {input_path}: 尺寸 {w}×{h} 小于 16×16")
        return

    if mode == "sprite":
        data = convert_to_sprite(img, transparent_rgb)
        suffix = "_16x16.bin"
    else:
        data = convert_to_frame(img, offset_x=10, offset_y=0,
                                transparent_rgb=transparent_rgb)
        suffix = "_36x16.bin"

    out_name = input_path.stem + suffix
    out_path = out_dir / out_name
    out_path.write_bytes(data)
    print(f"[OK] {input_path.name} -> {out_path} ({len(data)} 字节)")


def main():
    parser = argparse.ArgumentParser(
        description="16×16 像素素材取模 -> ESP32 .bin"
    )
    parser.add_argument("input", help="输入文件或文件夹")
    parser.add_argument("-o", "--output", default="./pixel_bins",
                        help="输出目录 (默认 ./pixel_bins)")
    parser.add_argument("-m", "--mode", choices=["sprite", "frame"],
                        default="sprite",
                        help="sprite=768B精灵图, frame=1728B全屏帧")
    parser.add_argument("--transparent", nargs=3, type=int,
                        default=[0, 0, 0], metavar=("R", "G", "B"),
                        help="透明像素替换色 (默认 0 0 0 纯黑)")
    args = parser.parse_args()

    in_path = Path(args.input)
    out_dir = Path(args.output)
    out_dir.mkdir(parents=True, exist_ok=True)

    transparent_rgb = tuple(args.transparent)

    if in_path.is_dir():
        files = sorted(in_path.iterdir())
        for f in files:
            if f.suffix.lower() in (".png", ".jpg", ".jpeg", ".bmp", ".svg"):
                process_one(f, out_dir, args.mode, transparent_rgb)
    else:
        process_one(in_path, out_dir, args.mode, transparent_rgb)

    print(f"\n全部完成，输出目录: {out_dir.absolute()}")


if __name__ == "__main__":
    main()
