#include "scene_manager.h"
static const char* TAG = "SceneManager";

void SceneManager::InitSceneManager()
{
    MatrixHal::GetInstance().MatrixHalInit();
    auto& sd = SpiSdCard::GetInstance();
    auto& gfx = MatrixHal::GetInstance().Gfx();

    sd.ListDirectory("/sdcard/img");
    sd.ListDirectory(ANIM_DIR);

    int id0 = AddAnimation("fire_anim_5f_16x16.bin", 0, 0, 16, 16, 5, 100, 25);
    if (id0 >= 0) ESP_LOGI(TAG, "Anim[0] fire @ slot %d", id0);

    int id1 = AddAnimation("cat_anim_6f_16x16.bin", 18, 0, 16, 16, 6, 80, 25);
    if (id1 >= 0) ESP_LOGI(TAG, "Anim[1] cat @ slot %d", id1);

    MatrixHal::GetInstance().Refresh();
    xTaskCreatePinnedToCore(TestTask, "AnimTask", 4096, this, 1, nullptr, 1);
}

void SceneManager::TestTask(void* pv)
{
    static_cast<SceneManager*>(pv)->TestTaskBody();
}
void SceneManager::TestTaskBody()
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
    bool any_dirty = false;
    int min_x = MATRIX_WIDTH, min_y = MATRIX_HEIGHT;
    int max_x = 0, max_y = 0;

    for (int i = 0; i < MAX_ANIMATIONS; i++)
    {
        if (!slots_[i].active) continue;
        if (slots_[i].animator.Tick(now_ms, gfx))
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

    if (any_dirty)
    {
        MatrixHal::GetInstance().RefreshArea(min_x, min_y, max_x - min_x, max_y - min_y);
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
    auto& sd = SpiSdCard::GetInstance();
    if (!sd.IsMounted())
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
    if (!sd.ReadFile(path, slots_[slot].buffer, total))
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