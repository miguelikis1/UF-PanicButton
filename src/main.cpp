#include "utils.h"
#include "config.h"
#include "audio_control.h"
#include "window_manager.h"
#include "decoy_manager.h"
#include "tray_icon.h"
#include "settings_ui.h"
#include "i18n.h"

#include <objidl.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <shellapi.h>

namespace {

constexpr const wchar_t* kMainWndClass = L"PanicButton_MainWnd";
constexpr const wchar_t* kRawInputClass = L"PanicButton_RawInputSink";
constexpr const wchar_t* kMutexName = L"Local\\PanicButton_SingleInstance_v1";

constexpr int kHotkeyIdPanic = 1;
constexpr int kHotkeyIdSettings = 2;

constexpr UINT WM_APP_OPEN_SETTINGS = WM_APP + 50;
constexpr UINT WM_APP_RESTORE_NOW = WM_APP + 51;

#ifndef MOD_NOREPEAT
#define MOD_NOREPEAT 0x4000
#endif

class AppController {
public:
    AppController(HINSTANCE hInst) : hInst_(hInst) {}

    int Run(bool startMinimized, bool forceSettings) {
        config_.Load();
        i18n::SetConfiguredLanguage(static_cast<i18n::Language>(config_.language));

        if (!CreateMainWindow()) return 1;

        taskbarCreatedMsg_ = RegisterWindowMessageW(L"TaskbarCreated");

        // Recover any windows left hidden by an unexpected crash/kill
        size_t recovered = windowMgr_.RecoverOrphanedWindowsIfAny();

        tray_.Init(mainHwnd_);
        tray_.SetVisible(!config_.stealthTray);
        tray_.UpdateState(false, utils::FormatHotkey(config_.panicModifiers, config_.panicVk));

        // Register global hotkeys and mouse raw input sink
        std::wstring hkErr;
        bool hkOk = RegisterAllHotkeys(hkErr);
        UpdateMouseTriggerRegistration();

        // Pre-warm decoy in background if configured
        decoyMgr_.Prepare(mainHwnd_, hInst_, config_);

        if (recovered > 0) {
            tray_.ShowBalloon(i18n::Get(i18n::StrId::TrayBalloonTitle),
                              std::wstring(i18n::Get(i18n::StrId::TrayBalloonRecovered)) + std::to_wstring(recovered));
        } else if (!hkOk) {
            OpenSettings();
            MessageBoxW(settingsUi_.GetHwnd(),
                        (std::wstring(i18n::Get(i18n::StrId::ToastErrorApplying)) + L":\n" + hkErr +
                         L"\n\n" + i18n::Get(i18n::StrId::PanicHotkeyDesc)).c_str(),
                        i18n::Get(i18n::StrId::AppName), MB_ICONWARNING | MB_OK);
        } else if (forceSettings || (!startMinimized && !ConfigFileExists())) {
            OpenSettings();
        } else if (!startMinimized && config_.showStartupNotification && !config_.stealthTray) {
            tray_.ShowBalloon(i18n::Get(i18n::StrId::TrayBalloonTitle),
                              std::wstring(i18n::Get(i18n::StrId::TrayBalloonStartup)) +
                              utils::FormatHotkey(config_.panicModifiers, config_.panicVk));
        } else if (!startMinimized) {
            OpenSettings();
        }

        // Trim unused pages after startup to minimize RAM footprint
        TrimWorkingSet();

        // Ultra-efficient event-driven message loop (0.0% idle CPU)
        MSG msg{};
        while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
            if (settingsUi_.IsVisible() && !settingsUi_.IsCapturingHotkey() &&
                IsDialogMessageW(settingsUi_.GetHwnd(), &msg)) {
                continue;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        Cleanup();
        return (int)msg.wParam;
    }

private:
    bool ConfigFileExists() const {
        DWORD attr = GetFileAttributesW(AppConfig::GetConfigPath().c_str());
        return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
    }

    void TrimWorkingSet() {
        SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);
    }

    bool CreateMainWindow() {
        // 1. Host window for tray icon and global hotkeys:
        // Use WS_POPUP with WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, 0x0 size, hidden, and explicitly NOTOPMOST.
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = AppController::MainWndProc;
        wc.hInstance = hInst_;
        wc.lpszClassName = kMainWndClass;
        if (!RegisterClassExW(&wc)) return false;

        mainHwnd_ = CreateWindowExW(
            WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
            kMainWndClass, L"PanicButton_Host",
            WS_POPUP, 0, 0, 0, 0,
            nullptr, nullptr, hInst_, this);
        if (!mainHwnd_) return false;

        ShowWindow(mainHwnd_, SW_HIDE);
        SetWindowPos(mainHwnd_, HWND_NOTOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_HIDEWINDOW);

        // 2. Dedicated message-only window (HWND_MESSAGE) for Raw Input (Mouse side buttons).
        // HWND_MESSAGE windows have NO Z-order, do NOT appear in Alt+Tab or DWM window trees,
        // and can never trap or steal foreground focus from other applications.
        WNDCLASSEXW rcwc{};
        rcwc.cbSize = sizeof(rcwc);
        rcwc.lpfnWndProc = AppController::RawInputWndProc;
        rcwc.hInstance = hInst_;
        rcwc.lpszClassName = kRawInputClass;
        RegisterClassExW(&rcwc);

        rawInputHwnd_ = CreateWindowExW(
            0, kRawInputClass, L"",
            0, 0, 0, 0, 0,
            HWND_MESSAGE, nullptr, hInst_, this);

        return true;
    }

    void UnregisterAllHotkeys() {
        if (!mainHwnd_) return;
        UnregisterHotKey(mainHwnd_, kHotkeyIdPanic);
        UnregisterHotKey(mainHwnd_, kHotkeyIdSettings);
    }

    bool RegisterAllHotkeys(std::wstring& errOut) {
        UnregisterAllHotkeys();
        if (!mainHwnd_) return false;

        if (config_.panicVk == 0) {
            errOut = i18n::Get(i18n::StrId::PanicHotkeyDesc);
            return false;
        }

        if (!RegisterHotKey(mainHwnd_, kHotkeyIdPanic, config_.panicModifiers | MOD_NOREPEAT, config_.panicVk)) {
            errOut = std::wstring(i18n::Get(i18n::StrId::PanicHotkeyLabel)) + L" (" +
                     utils::FormatHotkey(config_.panicModifiers, config_.panicVk) + L")";
            return false;
        }

        if (config_.settingsVk != 0 &&
            !(config_.settingsVk == config_.panicVk && config_.settingsModifiers == config_.panicModifiers)) {
            RegisterHotKey(mainHwnd_, kHotkeyIdSettings, config_.settingsModifiers | MOD_NOREPEAT, config_.settingsVk);
        }
        return true;
    }

    void UpdateMouseTriggerRegistration() {
        if (!rawInputHwnd_) return;
        RAWINPUTDEVICE rid[1]{};
        rid[0].usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
        rid[0].usUsage = 0x02;     // HID_USAGE_GENERIC_MOUSE

        if (config_.mouseTrigger != MouseTrigger::Disabled) {
            rid[0].dwFlags = RIDEV_INPUTSINK;
            rid[0].hwndTarget = rawInputHwnd_;
            RegisterRawInputDevices(rid, 1, sizeof(RAWINPUTDEVICE));
        } else {
            rid[0].dwFlags = RIDEV_REMOVE;
            rid[0].hwndTarget = nullptr;
            RegisterRawInputDevices(rid, 1, sizeof(RAWINPUTDEVICE));
        }
    }

    void TogglePanic() {
        ULONGLONG now = GetTickCount64();
        if (now - lastToggleMs_ < 220) return;
        lastToggleMs_ = now;

        if (!panicActive_) {
            ActivatePanic();
        } else {
            DeactivatePanic();
        }
    }

    void ActivatePanic() {
        if (panicActive_) return;

        // 1. Hide our own settings window immediately if open
        settingsWasVisibleBeforePanic_ = settingsUi_.IsVisible();
        if (settingsWasVisibleBeforePanic_) {
            settingsUi_.Hide();
        }

        // 2. Intelligent multimedia pause via WASAPI peak check + master mute
        audio_.OnPanic(config_.muteAudioOnPanic, config_.pauseMediaOnPanic);

        // 3. Collect decoy HWNDs so we don't hide our pre-warmed / captured decoy
        std::unordered_set<HWND> excluded;
        decoyMgr_.CollectExcludedHwnds(excluded);

        // 4. Hide all open windows & taskbar entries asynchronously (<5ms)
        windowMgr_.HideAllWindows(excluded, config_.GetExcludedProcessList());

        // 5. Pop up camouflage instantly (<2ms built-in, <10ms pre-warmed)
        decoyMgr_.Activate(mainHwnd_, hInst_, config_);

        panicActive_ = true;
        tray_.UpdateState(true, utils::FormatHotkey(config_.panicModifiers, config_.panicVk));
    }

    void DeactivatePanic() {
        if (!panicActive_) return;

        // 1. Hide or close camouflage decoy
        decoyMgr_.Deactivate(config_);

        // 2. Restore every hidden window in exact Z-order, state, and focus
        windowMgr_.RestoreAllWindows();

        // 3. Restore master volume & resume media if we paused it
        audio_.OnRestore();

        panicActive_ = false;
        tray_.UpdateState(false, utils::FormatHotkey(config_.panicModifiers, config_.panicVk));

        // 4. Restore settings UI if the user was testing from settings
        if (settingsWasVisibleBeforePanic_) {
            settingsWasVisibleBeforePanic_ = false;
            OpenSettings();
        } else if (settingsUi_.IsVisible()) {
            settingsUi_.RefreshStatus(config_);
        }
    }

    void OpenSettings() {
        SettingsCallbacks cb{};
        cb.onApplyConfig = [this](const AppConfig& newCfg, std::wstring& err) -> bool {
            AppConfig oldCfg = config_;
            bool decoyChanged = (oldCfg.decoyType != newCfg.decoyType) ||
                                (oldCfg.externalPath != newCfg.externalPath) ||
                                (oldCfg.externalArgs != newCfg.externalArgs) ||
                                (oldCfg.urlAddress != newCfg.urlAddress) ||
                                (oldCfg.urlBrowserMode != newCfg.urlBrowserMode);
            bool mouseTriggerChanged = (oldCfg.mouseTrigger != newCfg.mouseTrigger);
            bool langChanged = (oldCfg.language != newCfg.language);

            config_ = newCfg;
            if (langChanged) {
                i18n::SetConfiguredLanguage(static_cast<i18n::Language>(config_.language));
            }

            if (!RegisterAllHotkeys(err)) {
                config_ = oldCfg;
                std::wstring dummy;
                RegisterAllHotkeys(dummy);
                return false;
            }

            if (mouseTriggerChanged) {
                UpdateMouseTriggerRegistration();
            }

            config_.Save();
            SetStartWithWindows(config_.startWithWindows);
            tray_.SetVisible(!config_.stealthTray);
            tray_.UpdateState(panicActive_, utils::FormatHotkey(config_.panicModifiers, config_.panicVk));

            if (decoyChanged) {
                decoyMgr_.ReleasePrewarmedExternal(true);
            }
            decoyMgr_.Prepare(mainHwnd_, hInst_, config_);
            return true;
        };
        cb.onTogglePanic = [this]() { TogglePanic(); };
        cb.onPrepareDecoy = [this]() {
            decoyMgr_.ReleasePrewarmedExternal(true);
            decoyMgr_.Prepare(mainHwnd_, hInst_, config_);
        };
        cb.onCaptureWindow = [this](HWND target, bool hideNow) {
            decoyMgr_.SetCapturedWindow(target, hideNow);
        };
        cb.onReleaseCapturedWindow = [this]() {
            decoyMgr_.ReleaseCapturedWindow(true);
        };
        cb.onBeginHotkeyCapture = [this]() {
            UnregisterAllHotkeys();
        };
        cb.onEndHotkeyCapture = [this]() {
            std::wstring dummy;
            RegisterAllHotkeys(dummy);
        };
        cb.onExitApp = [this]() {
            RequestExit();
        };
        cb.onRelaunchAdmin = [this]() {
            if (panicActive_) DeactivatePanic();
            if (utils::RelaunchAsAdmin()) {
                RequestExit();
            }
        };
        cb.isPanicActive = [this]() { return panicActive_; };
        cb.getDecoyStatus = [this]() { return decoyMgr_.GetStatusText(config_); };
        cb.getCapturedWindowTitle = [this]() { return decoyMgr_.GetCapturedWindowTitle(); };
        cb.getLastHideStats = [this]() { return windowMgr_.LastHideStats(); };
        cb.getLastRestoreStats = [this]() { return windowMgr_.LastRestoreStats(); };
        cb.getHiddenCount = [this]() { return windowMgr_.HiddenCount(); };

        settingsUi_.Show(hInst_, config_, std::move(cb));
    }

    void RequestExit() {
        if (panicActive_) {
            DeactivatePanic();
        }
        PostQuitMessage(0);
    }

    void Cleanup() {
        if (panicActive_) {
            DeactivatePanic();
        }
        UnregisterAllHotkeys();
        config_.mouseTrigger = MouseTrigger::Disabled;
        UpdateMouseTriggerRegistration();
        decoyMgr_.Shutdown();
        settingsUi_.Destroy();
        tray_.Destroy();
        if (rawInputHwnd_) {
            DestroyWindow(rawInputHwnd_);
            rawInputHwnd_ = nullptr;
        }
        if (mainHwnd_) {
            DestroyWindow(mainHwnd_);
            mainHwnd_ = nullptr;
        }
    }

    static LRESULT CALLBACK RawInputWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        AppController* self = nullptr;
        if (msg == WM_NCCREATE) {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            self = reinterpret_cast<AppController*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->rawInputHwnd_ = hwnd;
        } else {
            self = reinterpret_cast<AppController*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (self && msg == WM_INPUT) {
            if (self->config_.mouseTrigger != MouseTrigger::Disabled) {
                UINT dwSize = 0;
                GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
                if (dwSize > 0) {
                    alignas(RAWINPUT) BYTE rawBuf[256];
                    BYTE* pData = (dwSize <= sizeof(rawBuf)) ? rawBuf : new BYTE[dwSize];
                    if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, pData, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize) {
                        RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(pData);
                        if (raw->header.dwType == RIM_TYPEMOUSE) {
                            USHORT btn = raw->data.mouse.usButtonFlags;
                            bool triggered = false;
                            if ((self->config_.mouseTrigger == MouseTrigger::XButton1 || self->config_.mouseTrigger == MouseTrigger::AnyXButton) &&
                                (btn & RI_MOUSE_BUTTON_4_DOWN)) {
                                triggered = true;
                            }
                            if ((self->config_.mouseTrigger == MouseTrigger::XButton2 || self->config_.mouseTrigger == MouseTrigger::AnyXButton) &&
                                (btn & RI_MOUSE_BUTTON_5_DOWN)) {
                                triggered = true;
                            }
                            if (triggered) {
                                self->TogglePanic();
                            }
                        }
                    }
                    if (pData != rawBuf) delete[] pData;
                }
            }
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        AppController* self = nullptr;
        if (msg == WM_NCCREATE) {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            self = reinterpret_cast<AppController*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->mainHwnd_ = hwnd;
        } else {
            self = reinterpret_cast<AppController*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }
        if (self) return self->HandleMessage(msg, wParam, lParam);
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (taskbarCreatedMsg_ != 0 && msg == taskbarCreatedMsg_) {
            tray_.OnTaskbarCreated();
            return 0;
        }

        switch (msg) {
            case WM_HOTKEY:
                if (wParam == kHotkeyIdPanic) {
                    TogglePanic();
                    return 0;
                } else if (wParam == kHotkeyIdSettings) {
                    OpenSettings();
                    return 0;
                }
                break;

            case WM_TIMER:
                if (wParam == DecoyManager::kPrewarmTimerId) {
                    if (decoyMgr_.OnPrewarmTimerTick(mainHwnd_) && settingsUi_.IsVisible()) {
                        settingsUi_.RefreshStatus(config_);
                    }
                    return 0;
                }
                break;

            case TrayIcon::WM_TRAYICON:
                if (lParam == WM_LBUTTONDBLCLK) {
                    OpenSettings();
                    return 0;
                } else if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
                    tray_.ShowContextMenu(panicActive_, utils::FormatHotkey(config_.panicModifiers, config_.panicVk));
                    return 0;
                }
                break;

            case WM_COMMAND:
                switch (LOWORD(wParam)) {
                    case ID_TRAY_TOGGLE_PANIC:
                        TogglePanic();
                        return 0;
                    case ID_TRAY_PREPARE_DECOY:
                        decoyMgr_.ReleasePrewarmedExternal(true);
                        decoyMgr_.Prepare(mainHwnd_, hInst_, config_);
                        return 0;
                    case ID_TRAY_OPEN_SETTINGS:
                        OpenSettings();
                        return 0;
                    case ID_TRAY_EXIT:
                        RequestExit();
                        return 0;
                    default:
                        break;
                }
                break;

            case WM_APP_OPEN_SETTINGS:
                OpenSettings();
                return 0;

            case WM_APP_RESTORE_NOW:
                if (panicActive_) DeactivatePanic();
                return 0;

            case WM_QUERYENDSESSION:
                if (panicActive_) DeactivatePanic();
                return TRUE;

            case WM_ENDSESSION:
                if (wParam && panicActive_) DeactivatePanic();
                return 0;

            case WM_DESTROY:
                return 0;

            default:
                break;
        }
        return DefWindowProcW(mainHwnd_, msg, wParam, lParam);
    }

    HINSTANCE hInst_ = nullptr;
    HWND mainHwnd_ = nullptr;
    HWND rawInputHwnd_ = nullptr;
    UINT taskbarCreatedMsg_ = 0;

    AppConfig config_;
    AudioControl audio_;
    WindowManager windowMgr_;
    DecoyManager decoyMgr_;
    TrayIcon tray_;
    SettingsWindow settingsUi_;

    bool panicActive_ = false;
    bool settingsWasVisibleBeforePanic_ = false;
    ULONGLONG lastToggleMs_ = 0;
};

} // namespace

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR lpCmdLine, int) {
    // Parse CLI arguments
    std::wstring cmd = lpCmdLine ? lpCmdLine : L"";
    bool startMinimized = (cmd.find(L"--minimized") != std::wstring::npos);
    bool forceSettings = (cmd.find(L"--settings") != std::wstring::npos);
    bool forceRestore = (cmd.find(L"--restore") != std::wstring::npos);

    // Single-instance check
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, kMutexName);
    if (hMutex && GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND existing = FindWindowW(kMainWndClass, nullptr);
        if (existing) {
            PostMessageW(existing, forceRestore ? WM_APP_RESTORE_NOW : WM_APP_OPEN_SETTINGS, 0, 0);
        }
        CloseHandle(hMutex);
        return 0;
    }

    // Emergency restore CLI mode when app isn't running
    if (forceRestore) {
        WindowManager wm;
        wm.RecoverOrphanedWindowsIfAny();
        if (hMutex) CloseHandle(hMutex);
        return 0;
    }

    // Initialize COM, Common Controls v6, and GDI+
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_TAB_CLASSES | ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    Gdiplus::GdiplusStartupInput gdiInput;
    ULONG_PTR gdiToken = 0;
    Gdiplus::GdiplusStartup(&gdiToken, &gdiInput, nullptr);

    int exitCode = 0;
    {
        AppController app(hInst);
        exitCode = app.Run(startMinimized, forceSettings);
    }

    Gdiplus::GdiplusShutdown(gdiToken);
    CoUninitialize();
    if (hMutex) CloseHandle(hMutex);
    return exitCode;
}
