#pragma once

#include "page.h"
#include <cstdint>

// 测试页面：左侧火苗动图 + 右侧 3 位秒计数器
class TestPage : public Page
{
   public:
    TestPage();
    ~TestPage() override;
    bool Tick() override;
    Type GetPageType() const override { return Type::Test; }

   private:
    int anim_id_ = -1;
    uint32_t last_ms_ = 0;
    uint16_t sec_ = 0;
};
