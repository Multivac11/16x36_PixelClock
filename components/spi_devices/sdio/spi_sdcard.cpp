#include "spi_sdcard.h"

static const char* TAG = "SpiSdCard";

/* ================= 初始化 ================= */

void SpiSdCard::InitSpiSdCard()
{
    if (RegisterSpiSdCard() != ESP_OK)
    {
        ESP_LOGE(TAG, "SD卡挂载失败，检查接线与卡是否插入");
        mounted_ = false;
        return;
    }
    ESP_LOGI(TAG, "SD卡挂载成功");
    mounted_ = true;

    // 打印根目录文件，方便调试
    ListDirectory("/sdcard");
}

esp_err_t SpiSdCard::RegisterSpiSdCard()
{
    const sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.host_id = SPI2_HOST;
    slot_config.gpio_cs = SPI2_SPICS_IO_NUM;

    esp_vfs_fat_mount_config_t mount_config = {};
    mount_config.format_if_mount_failed = false;  // 调试新卡可改为true
    mount_config.max_files = 5;                   // 最多同时打开5个文件
    mount_config.allocation_unit_size = 0;
    mount_config.disk_status_check_enable = false;

    esp_err_t ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &sdcard_);
    return ret;
}

/* ================= 基础文件操作 ================= */

bool SpiSdCard::FileExists(const char* path)
{
    struct stat st;
    return (stat(path, &st) == 0);
}

size_t SpiSdCard::GetFileSize(const char* path)
{
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return st.st_size;
}

bool SpiSdCard::ReadFile(const char* path, uint8_t* buffer, size_t len, size_t offset)
{
    if (!mounted_) return false;
    FILE* f = fopen(path, "rb");
    if (!f)
    {
        ESP_LOGE(TAG, "打开失败: %s", path);
        return false;
    }
    if (offset > 0) fseek(f, offset, SEEK_SET);
    size_t read = fread(buffer, 1, len, f);
    fclose(f);
    if (read != len)
    {
        ESP_LOGW(TAG, "读取不足: %s, 期望%zu, 实际%zu", path, len, read);
        return false;
    }
    return true;
}

bool SpiSdCard::WriteFile(const char* path, const uint8_t* buffer, size_t len)
{
    if (!mounted_) return false;
    FILE* f = fopen(path, "wb");
    if (!f)
    {
        ESP_LOGE(TAG, "创建失败: %s", path);
        return false;
    }
    size_t written = fwrite(buffer, 1, len, f);
    fclose(f);
    return (written == len);
}

bool SpiSdCard::AppendFile(const char* path, const uint8_t* buffer, size_t len)
{
    if (!mounted_) return false;
    FILE* f = fopen(path, "ab");
    if (!f)
    {
        ESP_LOGE(TAG, "追加失败: %s", path);
        return false;
    }
    size_t written = fwrite(buffer, 1, len, f);
    fclose(f);
    return (written == len);
}

void SpiSdCard::ListDirectory(const char* path)
{
    if (!mounted_) return;
    DIR* dir = opendir(path);
    if (!dir)
    {
        ESP_LOGE(TAG, "打开目录失败: %s", path);
        return;
    }
    struct dirent* entry;
    ESP_LOGI(TAG, "目录列表: %s", path);
    while ((entry = readdir(dir)) != NULL)
    {
        ESP_LOGI(TAG, "  %s", entry->d_name);
    }
    closedir(dir);
}

/* ================= 字模读取 ================= */

bool SpiSdCard::ReadFontChar(
    const char* fontPath, char ch, char startChar, uint8_t fontW, uint8_t fontH, uint8_t* outBuf)
{
    if (!mounted_) return false;
    size_t bytesPerChar = (fontW * fontH + 7) / 8;  // 向上取整到字节
    int idx = (int)ch - (int)startChar;
    if (idx < 0) return false;
    size_t offset = (size_t)idx * bytesPerChar;

    FILE* f = fopen(fontPath, "rb");
    if (!f) return false;
    fseek(f, offset, SEEK_SET);
    size_t read = fread(outBuf, 1, bytesPerChar, f);
    fclose(f);
    return (read == bytesPerChar);
}

/* ================= 图片/帧读取 ================= */

bool SpiSdCard::ReadImageFrame(const char* path, uint8_t* outBuf, size_t bufSize)
{
    if (bufSize < FRAME_BUF_SIZE)
    {
        ESP_LOGE(TAG, "Buffer不足，需要%d字节", FRAME_BUF_SIZE);
        return false;
    }
    return ReadFile(path, outBuf, FRAME_BUF_SIZE, 0);
}

/* ================= 动图：合并格式 ================= */

bool SpiSdCard::ReadAnimFramePacked(const char* animPath, int frameIdx, uint8_t* outBuf, size_t bufSize)
{
    if (!mounted_) return false;
    FILE* f = fopen(animPath, "rb");
    if (!f) return false;

    // 读取文件头
    uint32_t frameCount = 0, frameSize = 0;
    fread(&frameCount, 4, 1, f);
    fread(&frameSize, 4, 1, f);
    fseek(f, 8, SEEK_CUR);  // 跳过宽、高、保留（共8字节），头总长16字节

    if (frameIdx >= (int)frameCount || bufSize < frameSize)
    {
        fclose(f);
        return false;
    }

    size_t offset = 16 + (size_t)frameIdx * frameSize;
    fseek(f, offset, SEEK_SET);
    size_t read = fread(outBuf, 1, frameSize, f);
    fclose(f);
    return (read == frameSize);
}

int SpiSdCard::GetAnimFrameCountPacked(const char* animPath)
{
    if (!mounted_) return 0;
    FILE* f = fopen(animPath, "rb");
    if (!f) return 0;
    uint32_t frameCount = 0;
    fread(&frameCount, 4, 1, f);
    fclose(f);
    return (int)frameCount;
}

/* ================= 动图：分散格式 ================= */

bool SpiSdCard::ReadAnimFrameByIndex(const char* folder, int frameIdx, uint8_t* outBuf, size_t bufSize)
{
    if (bufSize < FRAME_BUF_SIZE) return false;
    char path[80];
    snprintf(path, sizeof(path), "%s/%03d.bin", folder, frameIdx);
    return ReadImageFrame(path, outBuf, bufSize);
}
