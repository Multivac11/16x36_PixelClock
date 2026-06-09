#include "scene_manager.h"

#include "esp_netif.h"
#include "uis/page_show_ip.h"
#include "uis/page_test.h"
#include "uis/page_wifi_overlay.h"

static const char* TAG = "SceneManager";

void SceneManager::InitSceneManager()
{
    ui_queue_ = xQueueCreate(1, sizeof(WifiManager::WifiStatusInfo));
    MatrixHal::GetInstance().MatrixHalInit();
    xTaskCreatePinnedToCore(RenderTask, "RenderTask", 4096, this, 1, nullptr, 1);
    SplashScreen();
    xTaskCreatePinnedToCore(WIFIStatusListenerTask, "WIFIStatusListenerTask", 4096, this, 1, nullptr, 1);
    xTaskCreatePinnedToCore(UIShowTask, "UISIShowTask", 4096, this, 1, nullptr, 1);
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

void SceneManager::WIFIStatusListenerTask(void* pv)
{
    static_cast<SceneManager*>(pv)->WIFIStatusListenerTaskBody();
}

int SceneManager::PlayWifiAnim(WifiManager::WifiStatus status, uint16_t play_count)
{
    const char* filename;
    int16_t x = 10, y = 0;
    uint8_t fw = 16, fh = 16, fc = 4;
    uint16_t interval = 100;

    switch (status)
    {
        case WifiManager::WIFI_STATUS_APMODE:
            filename = "wifi/ap_mode_anim_3f_22x16.bin";
            fw = 22;
            fh = 16;
            fc = 3;
            interval = 500;
            x = 7;  // (36-22)/2 = 7
            break;
        case WifiManager::WIFI_STATUS_SCAN_FAILED:
            filename = "wifi/no_wifi_anim_7f_16x16.bin";
            fc = 7;
            interval = 100;
            break;
        case WifiManager::WIFI_STATUS_CONNECT_FAILED:
            filename = "wifi/wifi_connect_fail_anim_7f_16x16.bin";
            fc = 7;
            interval = 100;
            break;
        case WifiManager::WIFI_STATUS_CONNECTED:
            filename = "wifi/wifi_connected_anim_6f_16x16.bin";
            fc = 6;
            interval = 100;
            break;
        case WifiManager::WIFI_STATUS_DISCONNECTED:
            filename = "wifi/wifi_disconnected_anim_8f_16x16.bin";
            fc = 8;
            interval = 80;
            break;
        case WifiManager::WIFI_STATUS_SCANNING:
        default:
            filename = "wifi/wifi_scan_anim_4f_16x16.bin";
            fc = 4;
            interval = 400;
            break;
    }
    return AddAnimation(filename, x, y, fw, fh, fc, interval, 60, play_count);
}

void SceneManager::UIShowTaskBody()
{
    // 开机 → WifiOverlayPage
    current_page_ = std::make_unique<WifiOverlayPage>();

    while (true)
    {
        if (current_page_->Tick())
        {
            current_page_ = CreateNextPage();
        }
        CheckWifiStateChange();
    }
}

bool SceneManager::IsAnimPlaying(int idx) const
{
    if (!valid(idx)) return false;
    return slots_[idx].animator.IsPlaying();
}

std::unique_ptr<Page> SceneManager::CreateNextPage()
{
    // 根据上一个页面的类型决定下一页
    Page::Type type = current_page_->GetPageType();
    if (type == Page::Type::WifiOverlay)
    {
        auto* wifiPage = static_cast<WifiOverlayPage*>(current_page_.get());
        strncpy(ip_str_, wifiPage->GetIP(), sizeof(ip_str_) - 1);
        if (wifiPage->GetLastStatus() == WifiManager::WIFI_STATUS_CONNECTED)
        {
            ESP_LOGI(TAG, "Page transition: WifiOverlay → ShowIP");
            return std::make_unique<ShowIPPage>(ip_str_);
        }
        ESP_LOGI(TAG, "Page transition: WifiOverlay → TestPage");
        return std::make_unique<TestPage>();
    }

    if (type == Page::Type::ShowIP)
    {
        ESP_LOGI(TAG, "Page transition: ShowIP → TestPage");
        return std::make_unique<TestPage>();
    }

    // TestPage 不会主动结束，兜底
    return std::make_unique<TestPage>();
}

void SceneManager::CheckWifiStateChange()
{
    // 非 WifiOverlayPage 期间，WiFi 状态异常 → 强制切回覆盖层
    if (current_page_->GetPageType() == Page::Type::WifiOverlay) return;

    WifiManager::WifiStatusInfo info;
    bool has_other = false;
    WifiManager::WifiStatusInfo other_info;

    while (xQueueReceive(ui_queue_, &info, 0) == pdTRUE)
    {
        if (info.status == WifiManager::WIFI_STATUS_DISCONNECTED ||
            info.status == WifiManager::WIFI_STATUS_APMODE)
        {
            ESP_LOGI(TAG, "WiFi state changed, forcing overlay");
            current_page_.reset();
            current_page_ = std::make_unique<WifiOverlayPage>();
            return;
        }
        // 非紧急消息（如 IP 更新）保留最后一条，放回队列给页面消费
        other_info = info;
        has_other = true;
    }

    if (has_other)
    {
        xQueueOverwrite(ui_queue_, &other_info);
    }
}

void SceneManager::WIFIStatusListenerTaskBody()
{
    QueueHandle_t q = xQueueCreate(1, sizeof(WifiManager::WifiStatusInfo));
    WifiManager::GetInstance().RegisterListener(q);
    WifiManager::WifiStatusInfo wifi_info;
    while (true)
    {
        if (xQueueReceive(q, &wifi_info, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGW(TAG, "WifiStatus: %d, IP: %s", wifi_info.status, wifi_info.ip);
            xQueueOverwrite(ui_queue_, &wifi_info);
        }
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
                               uint8_t brightness,
                               uint16_t play_count)
{
    // 已注册？直接返回 slot ID
    for (int i = 0; i < MAX_ANIMATIONS; i++)
    {
        if (slots_[i].active && strcmp(slots_[i].filename, filename) == 0)
        {
            slots_[i].animator.SetPosition(x, y);
            slots_[i].animator.SetBrightness(brightness);
            return i;
        }
    }

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
    slots_[slot].animator.SetPlayCount(play_count);
    slots_[slot].animator.Play();
    strncpy(slots_[slot].filename, filename, sizeof(slots_[slot].filename) - 1);
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
    slots_[idx].filename[0] = '\0';
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