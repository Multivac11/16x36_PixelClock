#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "matrix_hal.h"
#include "spi_sdcard.h"

#define SPRITE_SIZE (16 * 16 * 3)

class SceneManager
{
   public:
    static SceneManager& GetInstance()
    {
        static SceneManager instance;
        return instance;
    }
    void InitSceneManager();

   private:
    SceneManager() = default;

    ~SceneManager() = default;

   private:
};
