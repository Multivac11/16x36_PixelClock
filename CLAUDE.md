# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 在此仓库中工作时提供指导。

## 项目概述

基于 ESP32-S3 的 16x36 像素时钟显示器，使用 ESP-IDF v5.5.3。功能包括：显示时间、读取环境传感器、在 WS2812 LED 矩阵上播放 GIF 动画，以及通过强制门户提供 WiFi 配网。

## 构建与烧录

```bash
# 首先设置 ESP-IDF 环境
. /home/tianyu/.espressif/v5.5.3/esp-idf/export.sh

# 构建
idf.py build

# 烧录到 /dev/ttyUSB0
idf.py flash

# 构建、烧录并监视串口输出
idf.py flash monitor
```

项目使用 `-ffast-math -O3` 编译，采用 C++17 标准。目标芯片为 `esp32s3`（通过 `IDF_TARGET=esp32s3` 设置）。

## 架构

所有模块均为单例模式，通过 `GetInstance()` 静态方法访问。初始化在 `main/main.cpp:app_main()` 中按顺序执行——顺序很重要，因为后续模块依赖 I2C/SPI/按键等先行就绪。

### 组件关系图

```
app_main
  ├── StatusLed        (GPIO LED + 蜂鸣器，指示设备状态)
  ├── StatusKey        (3 个硬件按键，长短按检测)
  ├── I2CMaster        (单路 I2C 总线，GPIO 47/48，100kHz)
  │   ├── Sht40        (温湿度传感器)
  │   ├── DS3231       (RTC 实时时钟)
  │   ├── Husb238      (USB PD 受电控制器)
  │   ├── Qmi8658      (6 轴 IMU)
  │   ├── Es8311       (音频 DAC)
  │   ├── Qmc5883P     (3 轴磁力计)
  │   └── Es7210Codec  (音频 ADC/编解码器)
  ├── SpiMaster        (SPI 总线，GPIO 11/12/13)
  │   └── SpiSdCard    (FATFS SD 卡，GIF 文件存储在 /sdcard/gif)
  ├── LightSensor      (环境光 ADC 采集，用于自动亮度调节)
  ├── ApWifi           (WiFi AP 模式 + Web 服务器 + WebSocket 配网)
  │   ├── WifiManager  (STA 模式连接/扫描，NVS 凭据存储)
  │   └── WsServer     (HTTP + WebSocket 服务器，提供配网页面)
  └── SceneManager     (管理 LED 矩阵上的精灵/GIF 动画)
      ├── MatrixHal    (WS2812 灯带驱动，通过 RMT，GPIO 16)
      ├── GfxDriver    (36x16 像素帧缓冲区，含绘图基元)
      └── Animator     (逐帧 RGB 位图播放器)
```

### 关键硬件引脚分配

| GPIO | 功能 |
|------|------|
| 47, 48 | I2C SCL/SDA |
| 11, 12, 13 | SPI MOSI/CLK/MISO |
| 16 | WS2812 灯带 (RMT) |
| 41, 40, 39 | 按键 (key1/2/3) |
| 3, 46 | 状态 LED |
| 10 | 蜂鸣器 |
| 1 | 光线传感器 ADC |

### 关键数据流

1. **WiFi 配网**：`ApWifi` 启动 AP 模式 → 通过 HTTP 提供 `apcfg.html` → 用户选择 SSID/密码 → WebSocket 传递凭据 → `WifiManager` 保存到 NVS 并以 STA 模式连接。
2. **显示刷新**：`SceneManager::Tick()` 每帧运行 — 每个 `AnimationSlot` 持有一个 `Animator`，从 SD 卡加载的 RGB 位图缓冲区播放，绘制到 `GfxDriver` 帧缓冲区，然后通过 `MatrixHal::Refresh()` 刷新到 WS2812。
3. **GIF 播放**：`SceneManager::AddAnimation()` 从 SD 卡 (`/sdcard/gif/<filename>`) 加载原始 RGB 帧数据到 32KB RAM 缓冲区，然后 `Animator::Tick()` 按配置的帧间隔逐帧切换。

### 分区布局

- `factory` — 主程序 (4M)
- `html` — SPIFFS 分区 (128K)，存放 AP 配网网页
- `nvs` — WiFi 凭据存储
- `phy_init` — RF 校准数据

## 代码风格

- BasedOnStyle: Google，120 列宽限制，4 空格缩进
- C++17，`.cpp`/`.h` 扩展名
- 所有成员变量带下划线后缀：`bus_handle_`、`status_`
- GPIO 和硬件常量在头文件中使用 `#define` 定义，不在源文件中
