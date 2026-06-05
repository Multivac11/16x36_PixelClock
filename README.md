# 16×36 像素时钟

基于 ESP32-S3 的桌面像素时钟，36×16 WS2812 LED 矩阵显示器。支持显示时间、环境传感器数据、播放 RGB 动图（GIF），以及通过强制门户（Captive Portal）进行 WiFi 配网。

> **当前状态：项目中期开发阶段，核心显示框架已就绪，传感器驱动已接入，UI 和功能集成仍在推进中。**

## 硬件

| 组件 | 型号 / 规格 |
|------|------------|
| 主控 | ESP32-S3 |
| 显示 | 36×16 WS2812 LED 矩阵（576 灯，4 组 9×16 serpentine 走线） |
| 时钟 | DS3231 RTC（I2C 0x68） |
| 温湿度 | SHT40（I2C 0x44） |
| 6 轴 IMU | QMI8658（I2C 0x6A） |
| 3 轴磁力计 | QMC5883P（I2C 0x2C） |
| USB PD | HUSB238（I2C 0x08） |
| 音频 DAC | ES8311（I2C 0x18） |
| 音频 ADC | ES7210（I2C 0x41） |
| 存储 | SPI SD 卡（FATFS，CS=GPIO 14） |
| 光线传感器 | 模拟 ADC（GPIO 1） |
| 按键 | 3 个（GPIO 41/40/39，支持长短按） |
| 状态指示 | 2 个 LED（GPIO 3/46）+ 蜂鸣器（GPIO 10） |

### 引脚分配

| GPIO | 功能 |
|------|------|
| 47, 48 | I2C SCL / SDA |
| 11, 12, 13 | SPI MOSI / CLK / MISO |
| 16 | WS2812 灯带（RMT） |
| 17 | 备用 |
| 14 | SD 卡 CS |
| 41, 40, 39 | 按键 1 / 2 / 3 |
| 3, 46 | 状态 LED |
| 10 | 蜂鸣器 |
| 1 | 光线传感器 ADC |

## 构建

```bash
# 设置 ESP-IDF v5.5.3 环境
. /home/tianyu/.espressif/v5.5.3/esp-idf/export.sh

# 构建
idf.py build

# 烧录并监视
idf.py flash monitor
```

- 编译标准：C++17，`-ffast-math -O3`
- 目标芯片：`ESP32-S3`

## 分区布局

| 分区 | 大小 | 用途 |
|------|------|------|
| `factory` | 4M | 主程序 |
| `html` | 128K | SPIFFS，存放 WiFi 配网网页 |
| `nvs` | — | WiFi 凭据存储 |
| `phy_init` | — | RF 校准数据 |

## 架构

所有模块采用单例模式（`GetInstance()` 静态方法）。初始化在 `main/main.cpp:app_main()` 中按依赖顺序执行。

### 核心组件

```
app_main
  ├── StatusLed         GPIO LED + 蜂鸣器
  ├── StatusKey         3 按键输入（长短按）
  ├── DeviceInit        统一设备初始化
  │     ├── I2CBusManager     I2C 总线（单路，100kHz）
  │     │     ├── DS3231       RTC
  │     │     ├── SHT40        温湿度
  │     │     ├── QMI8658      6 轴 IMU
  │     │     ├── QMC5883P     3 轴磁力计
  │     │     ├── HUSB238      USB PD
  │     │     ├── ES8311       音频 DAC
  │     │     └── ES7210       音频 ADC
  │     └── SPIBusManager     SPI 总线
  │           └── SDCard       FATFS SD 卡
  ├── LightSensor       环境光 ADC
  ├── AudioCapture      音频输入
  ├── ApWifi            WiFi AP + WebSocket 配网
  │     ├── WifiManager  STA 连接 / NVS 凭据
  │     └── WsServer     HTTP + WebSocket
  └── SceneManager      LED 矩阵显示管理
        ├── MatrixHal   WS2812 RMT 驱动 + 亮度 LUT
        ├── GfxDriver   36×16 帧缓冲 + 绘图基元（5×7 字体、几何图形）
        └── Animator    逐帧 RGB 位图播放器
```

### 显示渲染架构

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

SD 卡存储的 `.bin` 文件为原始 RGB 帧数据拼接：
```
[帧0 RGB] [帧1 RGB] [帧2 RGB] ...
每帧 = width × height × 3 字节
```

存放在 `/sdcard/gif/` 目录下。目前素材：
- `cat_anim_6f_16x16.bin` — 小猫奔跑，6 帧
- `fire_anim_5f_16x16.bin` — 火苗，5 帧
- `coin_anim_11f_16x16.bin` — 金币旋转，11 帧

## 已实现功能

- [x] WS2812 LED 矩阵驱动（RMT DMA）
- [x] 36×16 帧缓冲区 + 绘图库（像素、直线、矩形、圆、三角形、5×7 字体）
- [x] RGB 动图播放器（帧循环、可配置帧间隔和亮度）
- [x] 开机动画（渐显→保持→渐隐，动图+白色背景同步渐变）
- [x] 渲染任务统一调度（动画 + 自定义 UI 混合渲染，局部刷新）
- [x] I2C 总线 + 全部 7 个外设驱动及探测
- [x] SPI 总线 + SD 卡挂载（FATFS）
- [x] DS3231 RTC 时钟芯片
- [x] SHT40 温湿度传感器
- [x] HUSB238 USB PD 受电控制器
- [x] QMI8658 6 轴 IMU + QMC5883P 磁力计
- [x] ES8311 DAC + ES7210 ADC（音频通路就绪，上层功能待开发）
- [x] WiFi AP 模式 + Captive Portal 配网页面
- [x] WebSocket 配网流程（扫描→选择→连接→保存 NVS）
- [x] 光线传感器 ADC（自动亮度调节硬件就绪）
- [x] 3 按键输入（长短按检测）

## 待完成

- [ ] 时钟 UI（时间/日期显示，RTC 时间读取 + 界面）
- [ ] 环境数据展示（温湿度、气压等）
- [ ] WiFi 配网后的 STA 模式正常运行
- [ ] 自动亮度调节逻辑
- [ ] 按键交互（切换显示模式、配网触发等）
- [ ] NTP 网络校时
- [ ] 天气信息获取与显示
- [ ] 音频相关功能
- [ ] 低功耗 / 夜间模式
- [ ] OTA 固件升级
