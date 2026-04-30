#pragma once
#include "animator.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "matrix_hal.h"
#include "spi_sdcard.h"

#define ANIM_DIR "/sdcard/gif"
#define MAX_ANIMATIONS 4
#define MAX_ANIM_BYTES_PER_SPRITE (32 * 1024)

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

    int AddAnimation(const char* filename,
                     int16_t x,
                     int16_t y,
                     uint8_t frame_w,
                     uint8_t frame_h,
                     uint8_t frame_count,
                     uint16_t interval_ms,
                     uint8_t brightness = 255);

    void SetAnimPosition(int idx, int16_t x, int16_t y);
    void RemoveAnimation(int idx);
    void PlayAnim(int idx);
    void PauseAnim(int idx);

    static void TestTask(void* pv);
    void TestTaskBody();

   private:
    SceneManager() = default;
    ~SceneManager() = default;
    bool valid(int idx) const;
    AnimationSlot slots_[MAX_ANIMATIONS];
};