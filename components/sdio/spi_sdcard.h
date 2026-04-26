#pragma once

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "spi.h"

#define SPI2_SPICS_IO_NUM GPIO_NUM_14

// 你的16x36屏幕参数
#define MATRIX_WIDTH 36
#define MATRIX_HEIGHT 16
#define MATRIX_PIXELS (MATRIX_WIDTH * MATRIX_HEIGHT)
#define FRAME_BUF_SIZE (MATRIX_PIXELS * 3)  // 每帧 GRB 原始数据字节数

class SpiSdCard
{
   public:
    static SpiSdCard& GetInstance()
    {
        static SpiSdCard instance;
        return instance;
    }

    void InitSpiSdCard();

    // ========== 基础文件操作 ==========
    bool IsMounted() const { return mounted_; }
    bool FileExists(const char* path);
    size_t GetFileSize(const char* path);

    // 从文件offset处读取len字节到buffer
    bool ReadFile(const char* path, uint8_t* buffer, size_t len, size_t offset = 0);
    // 覆盖写入
    bool WriteFile(const char* path, const uint8_t* buffer, size_t len);
    // 追加写入（适合日志）
    bool AppendFile(const char* path, const uint8_t* buffer, size_t len);
    void ListDirectory(const char* path);

    // ========== 字模读取 ==========
    // fontPath: 如 "/sdcard/font/ascii_8x16.bin"
    // ch: 目标字符； startChar: 字模库起始字符（如空格' ' = 0x20）
    // fontW, fontH: 字模宽高（像素）
    // outBuf: 输出缓冲区，大小至少 (fontW * fontH + 7) / 8 字节
    bool ReadFontChar(const char* fontPath, char ch, char startChar, uint8_t fontW, uint8_t fontH, uint8_t* outBuf);

    // ========== 静态图片/单帧读取 ==========
    // 读取一帧原始像素数据（必须已经是物理灯序GRB）
    bool ReadImageFrame(const char* path, uint8_t* outBuf, size_t bufSize);

    // ========== 动图读取：合并文件格式（推荐） ==========
    // 文件头：| 帧数(4B) | 每帧大小(4B) | 宽(2B) | 高(2B) | 保留(4B) | = 16字节
    // 后面紧跟帧数据
    bool ReadAnimFramePacked(const char* animPath, int frameIdx, uint8_t* outBuf, size_t bufSize);
    int GetAnimFrameCountPacked(const char* animPath);

    // ========== 动图读取：分散文件格式 ==========
    // folder: 如 "/sdcard/anim/fire"； frameIdx: 对应 000.bin, 001.bin...
    bool ReadAnimFrameByIndex(const char* folder, int frameIdx, uint8_t* outBuf, size_t bufSize);

   private:
    esp_err_t RegisterSpiSdCard();

    sdmmc_card_t* sdcard_ = nullptr;
    bool mounted_ = false;
};