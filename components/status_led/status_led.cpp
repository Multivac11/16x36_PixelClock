#include "status_led.h"

StatusLed::StatusLed(gpio_num_t led_gpio1, gpio_num_t led_gpio2)
{
    led_pin_[1] = led_gpio1;
    led_pin_[0] = led_gpio2;
}

void StatusLed::InitStatusLed()
{
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = (1ULL << led_pin_[0]) | (1ULL << led_pin_[1]),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&gpio_cfg);

    xTaskCreatePinnedToCore(GetStatusTask, "GetStatusTask", 2048, this, 1, nullptr, 1);
    xTaskCreatePinnedToCore(WifiListenerTask, "WifiListenerTask", 2048, this, 1, nullptr, 1);
    xTaskCreatePinnedToCore(WifiStatusTask, "WifiStatusTask", 2048, this, 1, nullptr, 1);
    xTaskCreatePinnedToCore(SetSystemStatusTask, "SetSystemStatusTask", 2048, this, 1, nullptr, 1);
    ESP_LOGI("StatusLed", "InitStatusLed successfull");
}

void StatusLed::WifiListenerTask(void *pvParameters)
{
    static_cast<StatusLed *>(pvParameters)->WifiListener();
}

void StatusLed::WifiStatusTask(void *pvParameters)
{
    static_cast<StatusLed *>(pvParameters)->WifiStatus();
}

void StatusLed::SetSystemStatusTask(void *pvParameters)
{
    static_cast<StatusLed *>(pvParameters)->SystemLedStatus();
}

void StatusLed::GetStatusTask(void *pvParameters)
{
    static_cast<StatusLed *>(pvParameters)->GetStatus();
}

void StatusLed::WifiListener()
{
    QueueHandle_t q = xQueueCreate(1, sizeof(WifiManager::WifiStatus));
    WifiManager::GetInstance().RegisterListener(q);
    while (true)
    {
        if (xQueueReceive(q, &wifi_status_, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGW("StatusLed", "NetworkLedStatus %d", wifi_status_);
        }
    }
}

void StatusLed::SystemLedStatus()
{
    while (true)
    {
        if (system_led_status_ == STATUS_SYSTEM_NORMAL)
        {
            SystemNormal();
        }
        else if (system_led_status_ == STATUS_SYSTEM_ERROR)
        {
            SystemError();
        }
    }
}

void StatusLed::WifiStatus()
{
    while (true)
    {
        if (wifi_status_ == WifiManager::WifiStatus::WIFI_STATUS_DISCONNECTED)
        {
            WifiDisconnected();
        }
        else if (wifi_status_ == WifiManager::WifiStatus::WIFI_STATUS_CONNECTED)
        {
            WifiConnected();
        }
        else if (wifi_status_ == WifiManager::WifiStatus::WIFI_STATUS_APMODE)
        {
            WifiAPmode();
        }
        else if (wifi_status_ == WifiManager::WifiStatus::WIFI_STATUS_SCANNING)
        {
            WifiScanning();
        }
    }
}

void StatusLed::GetStatus()
{
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void StatusLed::SystemNormal()
{
    LedOn(led_pin_[1]);
    vTaskDelay(pdMS_TO_TICKS(200));
    LedOff(led_pin_[1]);
    vTaskDelay(pdMS_TO_TICKS(200));
}

void StatusLed::SystemError()
{
    LedOn(led_pin_[1]);
    vTaskDelay(pdMS_TO_TICKS(50));
    LedOff(led_pin_[1]);
    vTaskDelay(pdMS_TO_TICKS(50));
}

void StatusLed::WifiDisconnected()
{
    LedOn(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(500));
    LedOff(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(500));
}

void StatusLed::WifiAPmode()
{
    LedOn(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(150));
    LedOff(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(150));

    LedOn(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(150));
    LedOff(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(150));

    vTaskDelay(pdMS_TO_TICKS(2000));
}

void StatusLed::WifiConnected()
{
    LedOn(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void StatusLed::WifiScanning()
{
    LedOn(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(50));
    LedOff(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(50));
}

void StatusLed::LedOn(gpio_num_t led_gpio)
{
    gpio_set_level(led_gpio, 1);
}

void StatusLed::LedOff(gpio_num_t led_gpio)
{
    gpio_set_level(led_gpio, 0);
}