#include "scene_manager.h"

#include "esp_netif.h"

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
    auto& gfx = MatrixHal::GetInstance().Gfx();

    bool in_ap_mode = false;
    int anim_id = -1;
    WifiManager::WifiStatus ws = WifiManager::WifiStatus::WIFI_STATUS_SCANNING;
    WifiManager::WifiStatus prev_ws = ws;
    enum Phase
    {
        WIFI_OVERLAY,
        SHOW_IP,
        NORMAL_UI
    };
    Phase phase = WIFI_OVERLAY;
    bool one_shot = false;

    // 开机：先显示扫描动画（loop）
    anim_id = PlayWifiAnim(ws, 0);

    char ip_str[16] = {};
    int ip_scroll = 0;
    uint32_t ip_enter_ms = 0;

    while (true)
    {
        // 从内部队列取 WiFi 状态（WIFIStatusListenerTaskBody 写入）
        WifiManager::WifiStatusInfo info;
        bool has_new = false;
        while (xQueueReceive(ui_queue_, &info, 0) == pdTRUE)
        {
            ws = info.status;
            // 收到 IP 就存下
            if (info.ip[0]) strncpy(ip_str, info.ip, sizeof(ip_str) - 1);
            has_new = true;
        }

        // 状态没变且 (有动画在播 或 不在 overlay) → 跳过重复通知
        bool same_status = (ws == prev_ws);
        if (has_new && same_status && (anim_id >= 0 || phase != WIFI_OVERLAY)) has_new = false;

        // 只在没有一次性动画播放时响应新状态，否则暂存
        if (has_new && !one_shot)
        {
            prev_ws = ws;
            if (ws == WifiManager::WIFI_STATUS_APMODE) in_ap_mode = true;
            if (ws == WifiManager::WIFI_STATUS_CONNECTED) in_ap_mode = false;

            if (anim_id >= 0)
            {
                RemoveAnimation(anim_id);
                anim_id = -1;
            }
            // 防止 NORMAL_UI 留下的动画叠在上面
            for (int i = 0; i < MAX_ANIMATIONS; i++)
            {
                if (slots_[i].active) RemoveAnimation(i);
            }
            gfx.clear(Colors::BLACK);
            InvalidateRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT);

            switch (ws)
            {
                case WifiManager::WIFI_STATUS_APMODE:
                    anim_id = PlayWifiAnim(ws, 0);
                    one_shot = false;
                    phase = WIFI_OVERLAY;
                    break;
                case WifiManager::WIFI_STATUS_SCANNING:
                    if (prev_ws == WifiManager::WIFI_STATUS_CONNECTED)
                    {
                        // 从已连接变成扫描 = 断连了，先播断开动画
                        anim_id = PlayWifiAnim(WifiManager::WIFI_STATUS_DISCONNECTED, 1);
                        one_shot = true;
                    }
                    else
                    {
                        anim_id = PlayWifiAnim(ws, 0);
                        one_shot = false;
                    }
                    phase = WIFI_OVERLAY;
                    break;
                case WifiManager::WIFI_STATUS_CONNECTED:
                case WifiManager::WIFI_STATUS_CONNECT_FAILED:
                case WifiManager::WIFI_STATUS_SCAN_FAILED:
                case WifiManager::WIFI_STATUS_DISCONNECTED:
                    anim_id = PlayWifiAnim(ws, 1);
                    one_shot = true;
                    phase = WIFI_OVERLAY;
                    break;
            }
        }

        // 一次性动画播放完毕 → 决定下一步
        if (one_shot && anim_id >= 0 && !slots_[anim_id].animator.IsPlaying())
        {
            RemoveAnimation(anim_id);
            anim_id = -1;
            one_shot = false;
            gfx.clear(Colors::BLACK);
            InvalidateRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT);

            // 处理期间可能已到达的新状态
            WifiManager::WifiStatusInfo latest = info;
            while (xQueueReceive(ui_queue_, &latest, 0) == pdTRUE)
            {
                ws = latest.status;
                if (latest.ip[0]) strncpy(ip_str, latest.ip, sizeof(ip_str) - 1);
            }

            if (ws == WifiManager::WIFI_STATUS_CONNECT_FAILED && in_ap_mode)
                anim_id = PlayWifiAnim(WifiManager::WIFI_STATUS_APMODE, 0);
            else if (ws == WifiManager::WIFI_STATUS_CONNECTED)
            {
                phase = SHOW_IP;
                ip_enter_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                ip_scroll = MATRIX_WIDTH;
            }
            else if (ws == WifiManager::WIFI_STATUS_SCAN_FAILED || ws == WifiManager::WIFI_STATUS_CONNECT_FAILED ||
                     ws == WifiManager::WIFI_STATUS_DISCONNECTED)
                phase = NORMAL_UI;
            else
            {
                if (ws == WifiManager::WIFI_STATUS_SCANNING || ws == WifiManager::WIFI_STATUS_APMODE)
                {
                    anim_id = PlayWifiAnim(ws, 0);
                    phase = WIFI_OVERLAY;
                }
            }
        }

        // SHOW_IP 阶段
        if (phase == SHOW_IP)
        {
            uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
            if (now - ip_enter_ms > 7000)
            {
                phase = NORMAL_UI;
                gfx.clear(Colors::BLACK);
                InvalidateRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT);
            }
            else
            {
                char disp[32];
                snprintf(disp, sizeof(disp), "IP:%s", ip_str);
                gfx.fillRect(0, 4, MATRIX_WIDTH, 8, Colors::BLACK);
                gfx.drawString(ip_scroll, 4, disp, Colors::WHITE, Colors::BLACK, 1, 60);
                InvalidateRect(0, 4, MATRIX_WIDTH, 8);
                ip_scroll -= 2;
                if (ip_scroll < -(int)strlen(disp) * 6) ip_scroll = MATRIX_WIDTH;
            }
        }

        if (phase == NORMAL_UI)
        {
            static uint32_t last_sec = 0;
            static uint16_t sec = 0;
            uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

            // 火苗动画（AddAnimation 自动检测重复，不会反复加载）
            AddAnimation("fire_anim_5f_16x16.bin", 0, 0, 16, 16, 5, 100, 60);

            if (now - last_sec >= 1000)
            {
                last_sec = now;
                char buf[8];
                snprintf(buf, sizeof(buf), "%03d", (int)(sec % 1000));
                gfx.fillRect(16, 4, 18, 8, Colors::BLACK);
                gfx.drawString(16, 4, buf, Colors::WHITE, Colors::BLACK, 1, 60);
                InvalidateRect(16, 4, 18, 8);
                sec = (sec + 1) % 1000;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
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