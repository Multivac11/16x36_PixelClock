#pragma once

// Page 基类：每个页面继承此类，构造=注册动图，析构=清理动图
// Tick() 返回 false 表示继续本页，返回 true 表示本页结束
class Page
{
   public:
    enum class Type
    {
        WifiOverlay,
        ShowIP,
        Test,
    };

    virtual bool Tick() = 0;
    virtual Type GetPageType() const = 0;
    virtual ~Page() = default;
};
