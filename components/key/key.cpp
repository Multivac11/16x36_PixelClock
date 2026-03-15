#include "key.h"

constexpr uint32_t SCAN_MS = 20;

StatusKey::StatusKey(gpio_num_t p1, gpio_num_t p2, gpio_num_t p3, uint32_t lm) : longMs_(lm), queue_(nullptr)
{
    key_pin_[0] = p3;
    key_pin_[1] = p2;
    key_pin_[2] = p1;
}

void StatusKey::InitKeys()
{
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = (1ULL << key_pin_[0]) | (1ULL << key_pin_[1]) | (1ULL << key_pin_[2]),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&gpio_cfg);

    queue_ = xQueueCreate(1, sizeof(Event));
    xTaskCreatePinnedToCore(GetKeyTask, "GetKeyTask", 2048, this, 2, nullptr, 1);
}

void StatusKey::GetKeyTask(void *pvParameters)
{
    static_cast<StatusKey *>(pvParameters)->KeyStatus();
}

void StatusKey::KeyStatus()
{
    while (true)
    {
        ScanKeys();
    }
}

void StatusKey::ScanKeys()
{
    uint32_t now = esp_timer_get_time() / 1000;

    for (int i = 0; i < 3; ++i)
    {
        bool down = gpio_get_level(key_pin_[i]) == 0;

        if (down)
        { // 检测到按下
            if (!buffer_[i].act)
            { // 如果是刚按下
                buffer_[i].act = true;
                buffer_[i].t = now;
            }
        }
        else
        { // 检测到未按下
            if (buffer_[i].act)
            { // 检测到按下刚松开
                uint32_t dt = now - buffer_[i].t;
                ev_.key[i] = (dt >= longMs_) ? KEY_LONG : KEY_SHORT;
                buffer_[i].act = false;
            }
            else
            {
                ev_.key[i] = KEY_NONE;
                buffer_[i].t = now;
            }
        }
    }

    bool allZero = true; // 判断当按键发生了变化时才写入队列
    for (int i = 0; i < 3; ++i)
    {
        if (ev_.key[i] != KEY_NONE)
        {
            allZero = false;
            ESP_LOGI("Key", "Key %d %s", key_pin_[i], (ev_.key[i] == KEY_LONG) ? "Long" : "Short");
            break;
        }
    }

    if (!allZero)
    {
        xQueueOverwrite(queue_, &ev_);
    }

    vTaskDelay(pdMS_TO_TICKS(SCAN_MS));
}

bool StatusKey::Available()
{
    return xQueueReceive(queue_, &ev_, 0) == pdTRUE;
}

StatusKey::Event StatusKey::Read()
{
    return ev_;
}