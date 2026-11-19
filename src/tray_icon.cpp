#include "tray_icon.h"
#include "i18n.h"

TrayIcon::~TrayIcon() {
    Destroy();
}

void TrayIcon::Init(HWND ownerHwnd) {
    ownerHwnd_ = ownerHwnd;
    iconIdle_ = utils::CreateAppIcon(32, false);
    iconPanic_ = utils::CreateAppIcon(32, true);

    nid_.cbSize = sizeof(nid_);
    nid_.hWnd = ownerHwnd_;
    nid_.uID = 101;
    nid_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid_.uCallbackMessage = WM_TRAYICON;
    nid_.hIcon = iconIdle_;
    wcsncpy_s(nid_.szTip, i18n::Get(i18n::StrId::AppName), _TRUNCATE);
}

void TrayIcon::SetVisible(bool visible) {
    wantVisible_ = visible;
    if (!ownerHwnd_) return;

    if (visible && !added_) {
        nid_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        nid_.hIcon = panicActive_ ? iconPanic_ : iconIdle_;
        if (Shell_NotifyIconW(NIM_ADD, &nid_)) {
            added_ = true;
        }
    } else if (!visible && added_) {
        Shell_NotifyIconW(NIM_DELETE, &nid_);
        added_ = false;
    }
}

void TrayIcon::UpdateState(bool panicActive, const std::wstring& hotkeyLabel) {
    panicActive_ = panicActive;
    hotkeyLabel_ = hotkeyLabel;
    if (!ownerHwnd_) return;

    nid_.hIcon = panicActive_ ? iconPanic_ : iconIdle_;
    std::wstring tip = panicActive_
        ? (std::wstring(i18n::Get(i18n::StrId::TrayTipActive)) + hotkeyLabel_ + L")")
        : (std::wstring(i18n::Get(i18n::StrId::TrayTipIdle)) + hotkeyLabel_);
    wcsncpy_s(nid_.szTip, tip.c_str(), _TRUNCATE);

    if (added_) {
        nid_.uFlags = NIF_ICON | NIF_TIP;
        Shell_NotifyIconW(NIM_MODIFY, &nid_);
    }
}

void TrayIcon::ShowBalloon(const std::wstring& title, const std::wstring& text) {
    if (!added_) return;
    NOTIFYICONDATAW b = nid_;
    b.uFlags = NIF_INFO;
    b.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
    wcsncpy_s(b.szInfoTitle, title.c_str(), _TRUNCATE);
    wcsncpy_s(b.szInfo, text.c_str(), _TRUNCATE);
    Shell_NotifyIconW(NIM_MODIFY, &b);
}

void TrayIcon::ShowContextMenu(bool panicActive, const std::wstring& hotkeyLabel) {
    if (!ownerHwnd_) return;

    HMENU menu = CreatePopupMenu();
    if (!menu) return;

    std::wstring toggleText = panicActive
        ? (std::wstring(i18n::Get(i18n::StrId::TrayRestoreAll)) + L" (" + hotkeyLabel + L")")
        : (std::wstring(i18n::Get(i18n::StrId::TrayTogglePanic)) + L" (" + hotkeyLabel + L")");

    AppendMenuW(menu, MF_STRING, ID_TRAY_TOGGLE_PANIC, toggleText.c_str());
    AppendMenuW(menu, MF_STRING, ID_TRAY_PREPARE_DECOY, i18n::Get(i18n::StrId::TrayPrepareDecoy));
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, ID_TRAY_OPEN_SETTINGS, i18n::Get(i18n::StrId::TraySettings));
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, ID_TRAY_EXIT, i18n::Get(i18n::StrId::TrayExit));

    SetMenuDefaultItem(menu, ID_TRAY_OPEN_SETTINGS, FALSE);

    POINT pt{};
    GetCursorPos(&pt);
    // Use TPM_RETURNCMD to handle tray selection without forcing ownerHwnd_ to steal foreground
    int cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                             pt.x, pt.y, 0, ownerHwnd_, nullptr);
    DestroyMenu(menu);

    if (cmd > 0) {
        PostMessageW(ownerHwnd_, WM_COMMAND, MAKEWPARAM(cmd, 0), 0);
    }
}

void TrayIcon::OnTaskbarCreated() {
    added_ = false;
    if (wantVisible_) {
        SetVisible(true);
        UpdateState(panicActive_, hotkeyLabel_);
    }
}

void TrayIcon::Destroy() {
    if (added_) {
        Shell_NotifyIconW(NIM_DELETE, &nid_);
        added_ = false;
    }
    if (iconIdle_) { DestroyIcon(iconIdle_); iconIdle_ = nullptr; }
    if (iconPanic_) { DestroyIcon(iconPanic_); iconPanic_ = nullptr; }
}
