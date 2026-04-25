#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class WsMatrix
{
   public:
    static WsMatrix& GetInstance()
    {
        static WsMatrix instance;
        return instance;
    }

    WsMatrix() = default;

    ~WsMatrix() = default;

    void InitWsMatrix();

   private:
    static void WsMatrixTask(void*);

    void ShowMatrix();

    void SetMatrix();

   private:
    QueueHandle_t queue_;
};