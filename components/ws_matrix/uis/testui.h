#pragma once

#include "scene_manager.h"

// 测试 UI：左侧火苗动图 + 右侧 3 位秒计数器
class TestUI
{
   public:
    static TestUI& GetInstance()
    {
        static TestUI instance;
        return instance;
    }

    // 每帧调用，内部用自己的频率做节流
    void Test();

   private:
    TestUI() = default;
    uint32_t last_sec_ = 0;
    uint16_t sec_ = 0;
};
