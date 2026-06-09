#include "page_show_ip.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "matrix_hal.h"
#include "scene_manager.h"

static const char* TAG = "ShowIPPage";

ShowIPPage::ShowIPPage(const char* ip)
{
    strncpy(ip_str_, ip, sizeof(ip_str_) - 1);
    ip_str_[sizeof(ip_str_) - 1] = '\0';
    enter_ms_ = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // 清空上一页可能残留的动图
    auto& gfx = MatrixHal::GetInstance().Gfx();
    gfx.clear(Colors::BLACK);
    SceneManager::GetInstance().InvalidateRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT);

    ESP_LOGI(TAG, "ShowIPPage created, IP=%s", ip_str_);
}

ShowIPPage::~ShowIPPage()
{
    auto& gfx = MatrixHal::GetInstance().Gfx();
    gfx.fillRect(0, 4, MATRIX_WIDTH, 8, Colors::BLACK);
    SceneManager::GetInstance().InvalidateRect(0, 4, MATRIX_WIDTH, 8);
    ESP_LOGI(TAG, "ShowIPPage destroyed");
}

bool ShowIPPage::Tick()
{
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // 持续从队列拉取 IP 更新（DHCP 可能比 CONNECTED 事件晚到）
    QueueHandle_t q = SceneManager::GetInstance().GetUIQueue();
    WifiManager::WifiStatusInfo info;
    while (xQueueReceive(q, &info, 0) == pdTRUE)
    {
        if (info.ip[0]) strncpy(ip_str_, info.ip, sizeof(ip_str_) - 1);
    }

    // 7 秒超时 → 结束
    if (now - enter_ms_ > 7000) return true;

    auto& gfx = MatrixHal::GetInstance().Gfx();
    char disp[32];
    snprintf(disp, sizeof(disp), "IP:%s", ip_str_);

    gfx.fillRect(0, 4, MATRIX_WIDTH, 8, Colors::BLACK);
    gfx.drawString(ip_scroll_, 4, disp, Colors::WHITE, Colors::BLACK, 1, 60);
    SceneManager::GetInstance().InvalidateRect(0, 4, MATRIX_WIDTH, 8);

    ip_scroll_ -= 2;
    if (ip_scroll_ < -(int)strlen(disp) * 6) ip_scroll_ = MATRIX_WIDTH;

    vTaskDelay(pdMS_TO_TICKS(50));
    return false;
}
