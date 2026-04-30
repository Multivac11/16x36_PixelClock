#pragma once
#include <cstdint>

#include "gfx_driver.h"

struct AnimationDesc
{
    uint8_t frame_width;
    uint8_t frame_height;
    uint8_t frame_count;
    uint16_t frame_bytes;  // w*h*3
    uint16_t interval_ms;  // 帧间隔
    const uint8_t* data;   // RAM 缓冲区指针
};

class Animator
{
   public:
    void SetAnimation(const AnimationDesc* desc)
    {
        desc_ = desc;
        current_frame_ = 0;
        last_tick_ms_ = 0;
        playing_ = (desc != nullptr);
    }

    void Play() { playing_ = true; }
    void Pause() { playing_ = false; }
    void Stop()
    {
        playing_ = false;
        current_frame_ = 0;
    }
    int16_t GetX() const { return pos_x_; }
    int16_t GetY() const { return pos_y_; }
    uint8_t GetFrameWidth() const { return desc_ ? desc_->frame_width : 0; }
    uint8_t GetFrameHeight() const { return desc_ ? desc_->frame_height : 0; }
    void SetPosition(int16_t x, int16_t y)
    {
        pos_x_ = x;
        pos_y_ = y;
    }
    void SetBrightness(uint8_t b) { brightness_ = b; }

    // 在主循环中周期性调用，传入当前时间 ms
    bool Tick(uint32_t now_ms, GfxDriver& gfx)
    {
        if (!playing_ || !desc_) return false;
        if (now_ms - last_tick_ms_ >= desc_->interval_ms)
        {
            last_tick_ms_ = now_ms;
            DrawCurrentFrame(gfx);
            current_frame_ = (current_frame_ + 1) % desc_->frame_count;
            return true;  // 帧切换了，需要 Refresh
        }
        return false;
    }

    void DrawFrame(uint8_t frame_idx, GfxDriver& gfx)
    {
        if (!desc_) return;
        current_frame_ = frame_idx % desc_->frame_count;
        DrawCurrentFrame(gfx);
    }

    uint8_t CurrentFrame() const { return current_frame_; }
    bool IsPlaying() const { return playing_; }

   private:
    void DrawCurrentFrame(GfxDriver& gfx)
    {
        uint32_t offset = (uint32_t)current_frame_ * desc_->frame_bytes;
        const Color* bitmap = reinterpret_cast<const Color*>(desc_->data + offset);
        gfx.drawRGBBitmap(pos_x_, pos_y_, bitmap, desc_->frame_width, desc_->frame_height, brightness_);
    }
    const AnimationDesc* desc_ = nullptr;
    uint8_t current_frame_ = 0;
    uint32_t last_tick_ms_ = 0;
    bool playing_ = false;
    int16_t pos_x_ = 0, pos_y_ = 0;
    uint8_t brightness_ = 255;
};