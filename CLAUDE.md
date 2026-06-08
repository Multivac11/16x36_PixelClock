# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

基于 ESP32-S3 的 16x36 像素时钟显示器，使用 ESP-IDF v5.5.3。**当前为中期开发阶段**——核心显示框架已就绪，传感器驱动已接入，UI 和功能集成仍在推进中。

## 构建与烧录

```bash
# 首先设置 ESP-IDF 环境
. /home/tianyu/.espressif/v5.5.3/esp-idf/export.sh

# 构建
idf.py build

# 烧录并监视
idf.py flash monitor
```

编译选项：C++17，`-ffast-math -O3 -Wno-error=format=-Wno-format`。目标芯片 `esp32s3`。

## 架构

所有模块均为单例模式，通过 `GetInstance()` 静态方法访问。初始化在 [main/main.cpp](main/main.cpp) 的 `app_main()` 中按顺序执行——顺序很重要，因为后续模块依赖 I2C/SPI/按键等先行就绪。

### 组件关系图

```
app_main
  ├── StatusLed         (GPIO LED + 蜂鸣器，指示设备状态)
  ├── StatusKey         (3 个硬件按键，长短按检测)
  ├── DeviceInit        统一设备初始化
  │     ├── I2CBusManager     I2C 总线（GPIO 47/48，100kHz）
  │     │     ├── DS3231       RTC 实时时钟（0x68）
  │     │     ├── SHT40        温湿度传感器（0x44）
  │     │     ├── QMI8658      6 轴 IMU（0x6A）
  │     │     ├── QMC5883P     3 轴磁力计（0x2C）
  │     │     ├── HUSB238      USB PD 受电控制器（0x08）
  │     │     ├── ES8311       音频 DAC（0x18）
  │     │     └── ES7210       音频 ADC（0x41）
  │     └── SPIBusManager     SPI 总线（GPIO 11/12/13）
  │           └── SDCard       FATFS SD 卡（CS=GPIO 14）
  ├── LightSensor       环境光 ADC 采集（GPIO 1），用于自动亮度调节
  ├── AudioCapture      音频输入（I2S，依赖 ES7210）
  ├── ApWifi            WiFi AP 模式 + Web 服务器 + WebSocket 配网
  │     ├── WifiManager  STA 模式连接/扫描，NVS 凭据存储
  │     └── WsServer     HTTP + WebSocket 服务器，提供配网页面
  └── SceneManager      LED 矩阵显示管理
        ├── MatrixHal    WS2812 灯带驱动（RMT，GPIO 16），含亮度 LUT
        ├── GfxDriver    36x16 像素帧缓冲区，含绘图基元（5x7 字体、几何图形）
        └── Animator     逐帧 RGB 位图播放器
```

### I2C 设备地址速查

| 设备 | 地址 | 功能 |
|------|------|------|
| DS3231 | 0x68 | RTC |
| SHT40 | 0x44 | 温湿度 |
| QMI8658 | 0x6A | 6 轴 IMU |
| QMC5883P | 0x2C | 3 轴磁力计 |
| HUSB238 | 0x08 | USB PD |
| ES8311 | 0x18 | 音频 DAC |
| ES7210 | 0x41 | 音频 ADC |

### 显示渲染架构（重要设计知识）

**上下层完全解耦**，RenderTask 是唯一的硬件刷新者：

```
┌────────────────────────────────────────┐
│ 上层（任意 Task / Core）                │
│                                        │
│  AddAnimation / RemoveAnimation  动图  │
│  gfx.drawXXX() + InvalidateRect() UI   │
│  SetBackground()                  背景 │
│                                        │
│  画完即走，不关心何时刷到硬件            │
└──────────────┬─────────────────────────┘
               │ 写入 gfx 帧缓冲 + 标记脏区
               ▼
┌────────────────────────────────────────┐
│ RenderTask（Core 1，20ms 周期）         │
│                                        │
│  Tick() {                              │
│    收集脏区（动画切帧 + InvalidateRect） │
│    合成帧（背景 → 精灵）               │
│    RefreshArea() 局部刷新到 LED 硬件    │
│  }                                     │
│  唯一调用 led_strip_refresh() 的地方     │
└────────────────────────────────────────┘
```

**关键设计原则：**
- `Refresh()` / `RefreshArea()` 只在 RenderTask 中调用，避免 RMT 硬件并发冲突
- 动画切帧与绘制分离：`AdvanceFrame()` 只计时，`DrawCurrentFrame()` 只绘制
- 脏区自动合并：多个来源（动图、自定义 UI）的脏区取并集后一次性刷新
- 背景色变化触发全屏刷新，普通变化只做局部 `RefreshArea`

### 动画格式

SD 卡存储的 `.bin` 文件为原始 RGB 帧数据拼接，**不是真正的 GIF 文件**：
```
[帧0 RGB] [帧1 RGB] [帧2 RGB] ...
每帧 = width × height × 3 字节
```
存放在 `/sdcard/gif/` 目录下。`Animator` 按配置的帧间隔逐帧切换。

### 关键硬件引脚分配

| GPIO | 功能 |
|------|------|
| 47, 48 | I2C SCL/SDA |
| 11, 12, 13 | SPI MOSI/CLK/MISO |
| 14 | SD 卡 CS |
| 16 | WS2812 灯带 (RMT) |
| 17 | 备用 |
| 41, 40, 39 | 按键 key1/2/3 |
| 3, 46 | 状态 LED |
| 10 | 蜂鸣器 |
| 1 | 光线传感器 ADC |

### 关键数据流

1. **WiFi 配网**：`ApWifi` 启动 AP 模式 → 通过 HTTP 提供 `apcfg.html`（来自 SPIFFS `html` 分区）→ 用户选择 SSID/密码 → WebSocket 传递凭据 → `WifiManager` 保存到 NVS 并以 STA 模式连接。
2. **显示刷新**：`SceneManager::Tick()` 每帧运行 — 每个 `AnimationSlot` 持有一个 `Animator`，从 SD 卡加载的 RGB 位图缓冲区播放，绘制到 `GfxDriver` 帧缓冲区，然后通过 `MatrixHal::Refresh()` 刷新到 WS2812。
3. **动画播放**：`SceneManager::AddAnimation()` 从 SD 卡 (`/sdcard/gif/<filename>`) 加载原始 RGB 帧数据到 32KB RAM 缓冲区，然后 `Animator::Tick()` 按配置的帧间隔逐帧切换。

### 分区布局

| 分区 | 类型 | 大小 | 用途 |
|------|------|------|------|
| `nvs` | data, nvs | 0x6000 | WiFi 凭据存储 |
| `phy_init` | data, phy | 0x1000 | RF 校准数据 |
| `factory` | app, factory | 4M | 主程序 |
| `html` | data, spiffs | 128K | WiFi 配网网页（SPIFFS） |

### 源文件布局

```
main/               — app_main() 入口 + CMakeLists.txt（含 SPIFFS 镜像生成）
components/
  ├── device/       — DeviceInit 统一初始化
  │     ├── i2c_device/  — I2CBusManager + 7 个 I2C 外设驱动
  │     └── spi_device/  — SPIBusManager + SD 卡驱动（FATFS）
  ├── ws_matrix/    — SceneManager / MatrixHal / GfxDriver / Animator
  ├── wifi_manager/ — ApWifi / WifiManager / WsServer
  ├── key/          — StatusKey（3 按键，长短按）
  ├── status_led/   — StatusLed（GPIO LED + 蜂鸣器）
  ├── light_sensor/ — LightSensor（ADC 环境光）
  └── audio_capture/— AudioCapture（I2S 音频输入）
managed_components/ — ESP-IDF 托管组件（仅 led_strip）
```

## 代码风格

- BasedOnStyle: Google，120 列宽限制，4 空格缩进
- C++17，`.cpp`/`.h` 扩展名
- 所有成员变量带下划线后缀：`bus_handle_`、`status_`
- GPIO 和硬件常量在头文件中使用 `#define` 定义，不在源文件中

## 当前状态与待完成功能

已实现：WS2812 驱动、帧缓冲+绘图库、动画播放器、开机动画、渲染任务调度、全部 I2C/SPI 外设驱动、WiFi AP 配网、按键输入、光线传感器 ADC。

待完成：时钟 UI（RTC 时间+界面）、环境数据展示、WiFi STA 模式正常运行、自动亮度调节逻辑、按键交互（模式切换）、NTP 网络校时、天气信息获取、音频功能、低功耗/夜间模式、OTA 升级。
