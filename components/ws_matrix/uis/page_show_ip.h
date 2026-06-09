#pragma once

#include "page.h"
#include <cstdint>

// IP 显示页面：滚动显示 IP 地址，7 秒后自动结束
class ShowIPPage : public Page
{
   public:
    explicit ShowIPPage(const char* ip);
    ~ShowIPPage() override;
    bool Tick() override;
    Type GetPageType() const override { return Type::ShowIP; }

   private:
    char ip_str_[16] = {};
    int ip_scroll_ = 36;  // MATRIX_WIDTH
    uint32_t enter_ms_ = 0;
};
