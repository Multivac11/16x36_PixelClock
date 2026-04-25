#pragma once

#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_manager.h"

class StatusLed
{
   public:
    static StatusLed &GetInstance()
    {
        static StatusLed instance;

        return instance;
    }

    StatusLed(gpio_num_t led_pin1 = GPIO_NUM_19, gpio_num_t led_pin2 = GPIO_NUM_20);

    ~StatusLed() = default;

    enum NetworkLedStatusEnum
    {
        STATUS_NETWOIRK_OFFLINE = 0,
        STATUS_NETWOIRK_ONLINE = 1,
        STATUS_NETWOIRK_SCANING = 2,
    };

    enum SystemLedStatusEnum
    {
        STATUS_SYSTEM_NORMAL = 0,
        STATUS_SYSTEM_ERROR = 1,
    };

    void InitStatusLed();

   private:
    static void WifiListenerTask(void *);

    static void WifiStatusTask(void *);

    static void SetSystemStatusTask(void *);

    static void GetStatusTask(void *);

    void WifiListener();

    void WifiStatus();

    void SystemLedStatus();

    void GetStatus();

    void LedOn(gpio_num_t led_pin);

    void LedOff(gpio_num_t led_pin);

    void WifiDisconnected();

    void WifiAPmode();

    void WifiConnected();

    void WifiScanning();

    void SystemNormal();

    void SystemError();

   private:
    gpio_num_t led_pin_[2];

    NetworkLedStatusEnum network_led_status_ = STATUS_NETWOIRK_SCANING;

    SystemLedStatusEnum system_led_status_ = STATUS_SYSTEM_NORMAL;

    WifiManager::WifiStatus wifi_status_ = WifiManager::WifiStatus::WIFI_STATUS_DISCONNECTED;
};