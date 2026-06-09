#include "page_wifi_overlay.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "matrix_hal.h"
#include "scene_manager.h"

static const char* TAG = "WifiOverlayPage";

WifiOverlayPage::WifiOverlayPage()
{
    auto& gfx = MatrixHal::GetInstance().Gfx();
    gfx.clear(Colors::BLACK);
    SceneManager::GetInstance().InvalidateRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT);

    // 开机播放扫描动画（loop）
    anim_id_ = SceneManager::GetInstance().PlayWifiAnim(WifiManager::WIFI_STATUS_SCANNING, 0);
    ESP_LOGI(TAG, "WifiOverlayPage created, scanning anim_id=%d", anim_id_);
}

WifiOverlayPage::~WifiOverlayPage()
{
    if (anim_id_ >= 0)
    {
        SceneManager::GetInstance().RemoveAnimation(anim_id_);
    }
    // 清理所有可能残留的动图
    for (int i = 0; i < 4; i++)
    {
        // 不能直接访问 private slots_，但 SceneManager 没有 RemoveAll
        // 通过 SceneManager 的公开接口逐个清理 — 暂时依赖析构时 anim_id_ 的清理
    }
    auto& gfx = MatrixHal::GetInstance().Gfx();
    gfx.clear(Colors::BLACK);
    SceneManager::GetInstance().InvalidateRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT);
    ESP_LOGI(TAG, "WifiOverlayPage destroyed");
}

void WifiOverlayPage::HandleStatus(WifiManager::WifiStatus ws)
{
    if (ws == WifiManager::WIFI_STATUS_APMODE) in_ap_mode_ = true;
    if (ws == WifiManager::WIFI_STATUS_CONNECTED) in_ap_mode_ = false;

    // 先清掉当前动图
    if (anim_id_ >= 0)
    {
        SceneManager::GetInstance().RemoveAnimation(anim_id_);
        anim_id_ = -1;
    }

    auto& gfx = MatrixHal::GetInstance().Gfx();
    gfx.clear(Colors::BLACK);
    SceneManager::GetInstance().InvalidateRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT);

    switch (ws)
    {
        case WifiManager::WIFI_STATUS_APMODE:
            anim_id_ = SceneManager::GetInstance().PlayWifiAnim(ws, 0);
            one_shot_ = false;
            break;
        case WifiManager::WIFI_STATUS_SCANNING:
            if (prev_ws_ == WifiManager::WIFI_STATUS_CONNECTED)
            {
                // 断连 → 先播断开动画
                anim_id_ = SceneManager::GetInstance().PlayWifiAnim(WifiManager::WIFI_STATUS_DISCONNECTED, 1);
                one_shot_ = true;
            }
            else
            {
                anim_id_ = SceneManager::GetInstance().PlayWifiAnim(ws, 0);
                one_shot_ = false;
            }
            break;
        case WifiManager::WIFI_STATUS_CONNECTED:
        case WifiManager::WIFI_STATUS_CONNECT_FAILED:
        case WifiManager::WIFI_STATUS_SCAN_FAILED:
        case WifiManager::WIFI_STATUS_DISCONNECTED:
            anim_id_ = SceneManager::GetInstance().PlayWifiAnim(ws, 1);
            one_shot_ = true;
            break;
    }
}

bool WifiOverlayPage::Tick()
{
    bool done = false;

    auto& sm = SceneManager::GetInstance();

    // 1. 从队列拉取最新 WiFi 状态
    QueueHandle_t q = sm.GetUIQueue();
    WifiManager::WifiStatusInfo info;
    bool has_new = false;
    while (xQueueReceive(q, &info, 0) == pdTRUE)
    {
        ws_ = info.status;
        if (info.ip[0]) strncpy(ip_str_, info.ip, sizeof(ip_str_) - 1);
        has_new = true;
    }

    // 状态没变 + 动画在播 → 忽略
    bool same_status = (ws_ == prev_ws_);
    if (has_new && same_status && anim_id_ >= 0) has_new = false;

    // 2. 不在一次性动画中才响应新状态
    if (has_new && !one_shot_)
    {
        prev_ws_ = ws_;
        HandleStatus(ws_);
    }

    // 3. 一次性动画播放完毕 → 决定下一步
    if (one_shot_ && anim_id_ >= 0)
    {
        if (!sm.IsAnimPlaying(anim_id_))
        {
            sm.RemoveAnimation(anim_id_);
            anim_id_ = -1;
            one_shot_ = false;

            // 处理期间可能已到达的新状态
            WifiManager::WifiStatusInfo latest;
            while (xQueueReceive(q, &latest, 0) == pdTRUE)
            {
                ws_ = latest.status;
                if (latest.ip[0]) strncpy(ip_str_, latest.ip, sizeof(ip_str_) - 1);
            }

            // 根据最新状态决定去向
            switch (ws_)
            {
                case WifiManager::WIFI_STATUS_CONNECT_FAILED:
                    if (in_ap_mode_)
                    {
                        anim_id_ = sm.PlayWifiAnim(WifiManager::WIFI_STATUS_APMODE, 0);
                        break;
                    }
                    done = true;
                    break;

                case WifiManager::WIFI_STATUS_CONNECTED:
                    done = true;
                    break;

                case WifiManager::WIFI_STATUS_SCAN_FAILED:
                case WifiManager::WIFI_STATUS_DISCONNECTED:
                    done = true;
                    break;

                case WifiManager::WIFI_STATUS_SCANNING:
                case WifiManager::WIFI_STATUS_APMODE:
                default:
                    anim_id_ = sm.PlayWifiAnim(ws_, 0);
                    break;
            }
        }
    }

    vTaskDelay(pdMS_TO_TICKS(20));
    return done;
}
