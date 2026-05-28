#include "scene_manager.h"

static const char* TAG = "SceneManager";

void SceneManager::InitSceneManager()
{
    MatrixHal::GetInstance().MatrixHalInit();
    xTaskCreatePinnedToCore(RenderTask, "RenderTask", 8192, this, 1, nullptr, 1);
    SplashScreen();
    xTaskCreatePinnedToCore(UIShowTask, "UISIShowTask", 8192, this, 1, nullptr, 1);
}

void SceneManager::SplashScreen()
{
    int id = AddAnimation("cat_anim_6f_16x16.bin", 9, 0, 16, 16, 6, 80, 60);
    if (id < 0)
    {
        ESP_LOGE(TAG, "Failed to load splash animation");
        return;
    }

    const uint8_t peak = 60;
    const int rise_steps = 100;
    const int hold_steps = 50;
    const int fall_steps = 50;
    const int total_steps = rise_steps + hold_steps + fall_steps;
    const int delay_ms = 12;

    for (int i = 0; i <= total_steps; ++i)
    {
        uint8_t v;
        if (i < rise_steps)
            v = (uint8_t)(peak * i / rise_steps);
        else if (i < rise_steps + hold_steps)
            v = peak;
        else
            v = (uint8_t)(peak * (total_steps - i) / fall_steps);

        slots_[id].animator.SetBrightness(v);
        SetBackground(Color(v, v, v));
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }

    RemoveAnimation(id);
    SetBackground(Colors::BLACK);
}

void SceneManager::RenderTask(void* pv)
{
    static_cast<SceneManager*>(pv)->RenderTaskBody();
}

void SceneManager::UIShowTask(void* pv)
{
    static_cast<SceneManager*>(pv)->UIShowTaskBody();
}

void SceneManager::UIShowTaskBody()
{
    auto& gfx = MatrixHal::GetInstance().Gfx();

    int id = AddAnimation("fire_anim_5f_16x16.bin", 0, 0, 16, 16, 5, 100, 60);
    if (id < 0)
    {
        ESP_LOGE(TAG, "Failed to load fire animation");
        return;
    }

    uint16_t sec = 0;
    char buf[4];
    while (true)
    {
        snprintf(buf, sizeof(buf), "%03d", sec);
        gfx.fillRect(16, 4, 18, 8, Colors::BLACK);
        gfx.drawString(16, 4, buf, Colors::WHITE, Colors::BLACK, 1, 60);
        InvalidateRect(16, 4, 18, 8);
        vTaskDelay(pdMS_TO_TICKS(1000));
        sec = (sec + 1) % 1000;
    }
}

void SceneManager::RenderTaskBody()
{
    while (true)
    {
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        Tick(now);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void SceneManager::Tick(uint32_t now_ms)
{
    auto& gfx = MatrixHal::GetInstance().Gfx();

    // 1. 收集所有脏区：手动 invalidate + 动画切帧
    bool bg_changed = background_dirty_;
    bool any_dirty = bg_changed || fb_dirty_;

    int min_x = fb_dirty_ ? dirty_x1_ : MATRIX_WIDTH;
    int min_y = fb_dirty_ ? dirty_y1_ : MATRIX_HEIGHT;
    int max_x = fb_dirty_ ? dirty_x2_ : 0;
    int max_y = fb_dirty_ ? dirty_y2_ : 0;
    fb_dirty_ = false;

    for (int i = 0; i < MAX_ANIMATIONS; i++)
    {
        if (!slots_[i].active) continue;
        if (slots_[i].animator.AdvanceFrame(now_ms))
        {
            any_dirty = true;
            int x = slots_[i].animator.GetX();
            int y = slots_[i].animator.GetY();
            int w = slots_[i].animator.GetFrameWidth();
            int h = slots_[i].animator.GetFrameHeight();
            if (x < min_x) min_x = x;
            if (y < min_y) min_y = y;
            if (x + w > max_x) max_x = x + w;
            if (y + h > max_y) max_y = y + h;
        }
    }

    if (!any_dirty) return;

    // 2. 逐层合成帧
    if (bg_changed)
    {
        gfx.clear(background_color_);
        background_dirty_ = false;
    }

    for (int i = 0; i < MAX_ANIMATIONS; i++)
    {
        if (!slots_[i].active) continue;
        slots_[i].animator.DrawCurrentFrame(gfx);
    }

    // 3. 背景变 → 全刷；仅精灵/手动脏区 → 局部刷
    if (bg_changed)
        MatrixHal::GetInstance().Refresh();
    else
        MatrixHal::GetInstance().RefreshArea(min_x, min_y, max_x - min_x, max_y - min_y);
}

void SceneManager::InvalidateRect(int x, int y, int w, int h)
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
    if (x + w > MATRIX_WIDTH) w = MATRIX_WIDTH - x;
    if (y + h > MATRIX_HEIGHT) h = MATRIX_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    int x2 = x + w, y2 = y + h;
    if (!fb_dirty_)
    {
        fb_dirty_ = true;
        dirty_x1_ = x;
        dirty_y1_ = y;
        dirty_x2_ = x2;
        dirty_y2_ = y2;
    }
    else
    {
        if (x < dirty_x1_) dirty_x1_ = x;
        if (y < dirty_y1_) dirty_y1_ = y;
        if (x2 > dirty_x2_) dirty_x2_ = x2;
        if (y2 > dirty_y2_) dirty_y2_ = y2;
    }
}

int SceneManager::AddAnimation(const char* filename,
                               int16_t x,
                               int16_t y,
                               uint8_t frame_w,
                               uint8_t frame_h,
                               uint8_t frame_count,
                               uint16_t interval_ms,
                               uint8_t brightness)
{
    auto* sd = SPIBusManager::GetInstance().GetDeviceByCSPin<SDCard>(GPIO_NUM_14);
    if (!sd)
    {
        ESP_LOGE(TAG, "SD card not found");
        return -1;
    }
    if (!sd->IsMounted())
    {
        ESP_LOGE(TAG, "SD not mounted");
        return -1;
    }

    int slot = -1;
    for (int i = 0; i < MAX_ANIMATIONS; i++)
    {
        if (!slots_[i].active)
        {
            slot = i;
            break;
        }
    }
    if (slot < 0)
    {
        ESP_LOGE(TAG, "No free slot");
        return -1;
    }

    uint32_t total = (uint32_t)frame_w * frame_h * 3 * frame_count;
    if (total > MAX_ANIM_BYTES_PER_SPRITE)
    {
        ESP_LOGE(TAG, "Anim too large: %lu", total);
        return -1;
    }

    char path[128];
    snprintf(path, sizeof(path), "%s/%s", ANIM_DIR, filename);
    if (!sd->ReadFile(path, slots_[slot].buffer, total))
    {
        ESP_LOGE(TAG, "Read failed: %s", path);
        return -1;
    }

    slots_[slot].desc = {
        frame_w, frame_h, frame_count, (uint16_t)(frame_w * frame_h * 3), interval_ms, slots_[slot].buffer};
    slots_[slot].animator.SetAnimation(&slots_[slot].desc);
    slots_[slot].animator.SetPosition(x, y);
    slots_[slot].animator.SetBrightness(brightness);
    slots_[slot].animator.Play();
    slots_[slot].active = true;

    auto& gfx = MatrixHal::GetInstance().Gfx();
    slots_[slot].animator.DrawFrame(0, gfx);
    InvalidateRect(x, y, frame_w, frame_h);

    ESP_LOGI(TAG, "Anim[%d] %s loaded: %dx%d, %d frames, %lu bytes, pos(%d,%d)", slot, filename, frame_w, frame_h,
             frame_count, total, x, y);
    return slot;
}

void SceneManager::SetAnimPosition(int idx, int16_t x, int16_t y)
{
    if (!valid(idx)) return;
    slots_[idx].animator.SetPosition(x, y);
}

void SceneManager::RemoveAnimation(int idx)
{
    if (!valid(idx)) return;
    slots_[idx].animator.Stop();
    slots_[idx].active = false;
    ESP_LOGI(TAG, "Anim[%d] removed", idx);
}
bool SceneManager::valid(int idx) const
{
    return idx >= 0 && idx < MAX_ANIMATIONS && slots_[idx].active;
}
void SceneManager::PlayAnim(int idx)
{
    if (valid(idx)) slots_[idx].animator.Play();
}
void SceneManager::PauseAnim(int idx)
{
    if (valid(idx)) slots_[idx].animator.Pause();
}