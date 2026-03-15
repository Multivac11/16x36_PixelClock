#pragma once

#include <stdio.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

class StatusKey
{
public:
    static StatusKey &GetInstance()
    {
        static StatusKey instance;
        return instance;
    }

    StatusKey(gpio_num_t key_pin1 = GPIO_NUM_41, gpio_num_t key_pin2 = GPIO_NUM_40, gpio_num_t key_pin3 = GPIO_NUM_39, uint32_t longMs = 800);
    ~StatusKey() = default;

    enum KeyStatusEnum
    {
        KEY_NONE = 0,
        KEY_SHORT = 1,
        KEY_LONG = 2,
        KEY_PRESSING = 3,
    };

    struct Event
    {
        KeyStatusEnum key[3];
    };

    void InitKeys();  // 启动任务
    bool Available(); // 是否有新事件
    Event Read();     // 取出事件

private:
    static void GetKeyTask(void *); // 静态入口
    void ScanKeys();
    void KeyStatus();

private:
    gpio_num_t key_pin_[3];
    uint32_t longMs_;
    QueueHandle_t queue_;
    Event ev_; // 缓存
    struct Buf
    {
        uint32_t t;
        bool act;
    };
    Buf buffer_[3] = {};
};