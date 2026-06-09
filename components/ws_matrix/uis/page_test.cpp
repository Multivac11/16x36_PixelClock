#include "page_test.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "matrix_hal.h"
#include "scene_manager.h"

static const char* TAG = "TestPage";

TestPage::TestPage()
{
    auto& gfx = MatrixHal::GetInstance().Gfx();
    gfx.clear(Colors::BLACK);
    SceneManager::GetInstance().InvalidateRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT);

    anim_id_ = SceneManager::GetInstance().AddAnimation("fire_anim_5f_16x16.bin", 0, 0, 16, 16, 5, 100, 60);
    if (anim_id_ < 0)
    {
        ESP_LOGE(TAG, "Failed to load fire animation");
    }
    else
    {
        ESP_LOGI(TAG, "TestPage created, anim_id=%d", anim_id_);
    }
}

TestPage::~TestPage()
{
    if (anim_id_ >= 0)
    {
        SceneManager::GetInstance().RemoveAnimation(anim_id_);
    }
    auto& gfx = MatrixHal::GetInstance().Gfx();
    gfx.clear(Colors::BLACK);
    SceneManager::GetInstance().InvalidateRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT);
    ESP_LOGI(TAG, "TestPage destroyed");
}

bool TestPage::Tick()
{
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (now - last_ms_ < 1000)
    {
        vTaskDelay(pdMS_TO_TICKS(50));
        return false;
    }
    last_ms_ = now;

    auto& gfx = MatrixHal::GetInstance().Gfx();
    char buf[6];
    snprintf(buf, sizeof(buf), "%03d", sec_);
    gfx.fillRect(16, 4, 18, 8, Colors::BLACK);
    gfx.drawString(16, 4, buf, Colors::WHITE, Colors::BLACK, 1, 60);
    SceneManager::GetInstance().InvalidateRect(16, 4, 18, 8);

    sec_ = (sec_ + 1) % 1000;
    vTaskDelay(pdMS_TO_TICKS(50));
    return false;
}
