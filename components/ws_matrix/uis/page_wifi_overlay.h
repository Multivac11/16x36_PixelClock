#pragma once

#include "page.h"
#include "wifi_manager.h"

// WiFi 配网动画覆盖层：响应 WiFi 状态变化，播放对应动图
// 连接成功 → Tick() 返回 true，PageManager 切到 ShowIPPage
// 连接失败/断连 → Tick() 返回 true，PageManager 切到 TestPage
class WifiOverlayPage : public Page
{
   public:
    WifiOverlayPage();
    ~WifiOverlayPage() override;
    bool Tick() override;
    Type GetPageType() const override { return Type::WifiOverlay; }

    // 结束后供 PageManager 读取，决定切到哪个页面
    const char* GetIP() const { return ip_str_; }
    WifiManager::WifiStatus GetLastStatus() const { return ws_; }

   private:
    void HandleStatus(WifiManager::WifiStatus ws);

    int anim_id_ = -1;
    bool one_shot_ = false;
    bool in_ap_mode_ = false;
    WifiManager::WifiStatus ws_ = WifiManager::WIFI_STATUS_SCANNING;
    WifiManager::WifiStatus prev_ws_ = WifiManager::WIFI_STATUS_SCANNING;
    char ip_str_[16] = {};
};
