#pragma once

#include "animator.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "matrix_hal.h"
#include "spi_bus.h"
#include "wifi_manager.h"

#define ANIM_DIR "/sdcard/gif"
#define IMG_DIR "/sdcard/img"
#define MAX_ANIMATIONS 4
#define MAX_ANIM_BYTES_PER_SPRITE (32 * 1024)
#define SPRITE_SIZE (16 * 16 * 3)

class SceneManager
{
   public:
    static SceneManager& GetInstance()
    {
        static SceneManager instance;
        return instance;
    }

    struct AnimationSlot
    {
        Animator animator;
        AnimationDesc desc{};
        alignas(4) uint8_t buffer[MAX_ANIM_BYTES_PER_SPRITE];
        bool active = false;
    };

    void InitSceneManager();

    void Tick(uint32_t now_ms);

    void SplashScreen();

    int AddAnimation(const char* filename,
                     int16_t x,
                     int16_t y,
                     uint8_t frame_w,
                     uint8_t frame_h,
                     uint8_t frame_count,
                     uint16_t interval_ms,
                     uint8_t brightness = 255,
                     uint16_t play_count = 0);

    void SetAnimPosition(int idx, int16_t x, int16_t y);

    void RemoveAnimation(int idx);

    void PlayAnim(int idx);

    void PauseAnim(int idx);

    void SetBackground(const Color& c)
    {
        background_color_ = c;
        background_dirty_ = true;
    }

    // 上层画完 gfx 后调用，标记需要刷新的区域
    void InvalidateRect(int x, int y, int w, int h);

    static void RenderTask(void* pv);

    static void UIShowTask(void* pv);

    static void WIFIStatusListenerTask(void* pv);

    void RenderTaskBody();

    void UIShowTaskBody();

    void WIFIStatusListenerTaskBody();

   private:
    SceneManager() = default;

    ~SceneManager() = default;

    bool valid(int idx) const;

    AnimationSlot slots_[MAX_ANIMATIONS];

    Color background_color_ = Colors::BLACK;
    bool background_dirty_ = false;

    bool fb_dirty_ = false;
    int dirty_x1_ = 0, dirty_y1_ = 0, dirty_x2_ = 0, dirty_y2_ = 0;
};