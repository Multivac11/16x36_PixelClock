#include "testui.h"

static const char* TAG = "TestUI";

void TestUI::Test()
{
    auto& gfx = MatrixHal::GetInstance().Gfx();

    int id = SceneManager::GetInstance().AddAnimation("fire_anim_5f_16x16.bin", 0, 0, 16, 16, 5, 100, 60);
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
        SceneManager::GetInstance().InvalidateRect(16, 4, 18, 8);
        vTaskDelay(pdMS_TO_TICKS(1000));
        sec = (sec + 1) % 1000;
    }
}
