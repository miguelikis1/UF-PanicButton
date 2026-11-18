#pragma once

#include "utils.h"
#include <shellapi.h>
#include <string>

enum TrayCommand : UINT {
    ID_TRAY_TOGGLE_PANIC = 3001,
    ID_TRAY_PREPARE_DECOY = 3002,
    ID_TRAY_OPEN_SETTINGS = 3003,
    ID_TRAY_EXIT = 3004
};

class TrayIcon {
public:
    static constexpr UINT WM_TRAYICON = WM_APP + 10;

    TrayIcon() = default;
    ~TrayIcon();

    void Init(HWND ownerHwnd);
    void SetVisible(bool visible);
    void UpdateState(bool panicActive, const std::wstring& hotkeyLabel);
    void ShowBalloon(const std::wstring& title, const std::wstring& text);
    void ShowContextMenu(bool panicActive, const std::wstring& hotkeyLabel);
    void OnTaskbarCreated();
    void Destroy();

private:
    HWND ownerHwnd_ = nullptr;
    NOTIFYICONDATAW nid_{};
    HICON iconIdle_ = nullptr;
    HICON iconPanic_ = nullptr;
    bool added_ = false;
    bool wantVisible_ = true;
    bool panicActive_ = false;
    std::wstring hotkeyLabel_;
};
