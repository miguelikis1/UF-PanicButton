#include "decoy_manager.h"
#include "window_manager.h"
#include "i18n.h"

#include <shellapi.h>

namespace {

std::unordered_set<HWND> SnapshotVisibleUserWindows() {
    std::unordered_set<HWND> snap;
    auto list = WindowManager::EnumerateUserWindows();
    for (const auto& info : list) {
        snap.insert(info.hwnd);
    }
    return snap;
}

} // namespace

DecoyManager::~DecoyManager() {
    Shutdown();
}

void DecoyManager::Shutdown() {
    // Always restore a user-captured window if we hid it in standby!
    ReleaseCapturedWindow(true);
    // Close pre-warmed hidden external window we spawned so no orphan hidden process remains
    if (externalHwnd_ && IsWindow(externalHwnd_) && !IsWindowVisible(externalHwnd_)) {
        PostMessageW(externalHwnd_, WM_CLOSE, 0, 0);
    }
    externalHwnd_ = nullptr;
    externalPid_ = 0;
    builtin_.Destroy();
}

void DecoyManager::CollectExcludedHwnds(std::unordered_set<HWND>& out) const {
    if (builtin_.GetHwnd() && IsWindow(builtin_.GetHwnd())) {
        out.insert(builtin_.GetHwnd());
    }
    if (externalHwnd_ && IsWindow(externalHwnd_)) {
        out.insert(externalHwnd_);
    }
    if (capturedHwnd_ && IsWindow(capturedHwnd_)) {
        out.insert(capturedHwnd_);
    }
}

void DecoyManager::SetCapturedWindow(HWND hwnd, bool hideIntoStandby) {
    // First release any previous captured window cleanly
    if (capturedHwnd_ && capturedHwnd_ != hwnd) {
        ReleaseCapturedWindow(true);
    }
    capturedHwnd_ = (hwnd && IsWindow(hwnd)) ? hwnd : nullptr;
    capturedWasHiddenByUs_ = false;
    if (capturedHwnd_ && hideIntoStandby) {
        ShowWindowAsync(capturedHwnd_, SW_HIDE);
        capturedWasHiddenByUs_ = true;
    }
}

void DecoyManager::ReleaseCapturedWindow(bool restoreIfHidden) {
    if (capturedHwnd_ && IsWindow(capturedHwnd_)) {
        if (restoreIfHidden && capturedWasHiddenByUs_ && !IsWindowVisible(capturedHwnd_)) {
            ShowWindowAsync(capturedHwnd_, SW_SHOWNA);
        }
    }
    capturedHwnd_ = nullptr;
    capturedWasHiddenByUs_ = false;
}

HWND DecoyManager::GetCapturedWindow() const {
    return (capturedHwnd_ && IsWindow(capturedHwnd_)) ? capturedHwnd_ : nullptr;
}

std::wstring DecoyManager::GetCapturedWindowTitle() const {
    HWND h = GetCapturedWindow();
    if (!h) return L"(Ninguna ventana capturada)";
    int len = GetWindowTextLengthW(h);
    if (len <= 0) return L"(Ventana sin título)";
    std::wstring t(len, L'\0');
    GetWindowTextW(h, t.data(), len + 1);
    return t;
}

void DecoyManager::ReleasePrewarmedExternal(bool closeWindow) {
    if (externalHwnd_ && IsWindow(externalHwnd_)) {
        if (closeWindow) {
            PostMessageW(externalHwnd_, WM_CLOSE, 0, 0);
        } else if (!IsWindowVisible(externalHwnd_)) {
            ShowWindowAsync(externalHwnd_, SW_SHOWNA);
        }
    }
    externalHwnd_ = nullptr;
    externalPid_ = 0;
}

bool DecoyManager::LaunchExternalTarget(const AppConfig& cfg, bool forPrewarm, HANDLE* outProcess) {
    if (outProcess) *outProcess = nullptr;

    std::wstring file;
    std::wstring params;

    if (cfg.decoyType == DecoyType::ExternalAppOrFile) {
        file = utils::Trim(cfg.externalPath);
        params = cfg.externalArgs;
        if (file.empty()) return false;
    } else if (cfg.decoyType == DecoyType::ExternalUrl) {
        std::wstring url = utils::Trim(cfg.urlAddress);
        if (url.empty()) return false;
        if (url.find(L"://") == std::wstring::npos) {
            url = L"https://" + url;
        }
        if (cfg.urlBrowserMode == UrlBrowserMode::DefaultBrowser) {
            file = url;
        } else {
            std::wstring browser = utils::FindChromiumBrowserPath(false);
            if (!browser.empty()) {
                file = browser;
                if (cfg.urlBrowserMode == UrlBrowserMode::AppWindow) {
                    params = L"--app=\"" + url + L"\"";
                } else {
                    params = L"--new-window \"" + url + L"\"";
                }
            } else {
                file = url;
            }
        }
    } else {
        return false;
    }

    SHELLEXECUTEINFOW sei{};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    sei.lpVerb = L"open";
    sei.lpFile = file.c_str();
    sei.lpParameters = params.empty() ? nullptr : params.c_str();
    if (forPrewarm) {
        sei.nShow = SW_SHOWMINNOACTIVE;
    } else {
        sei.nShow = cfg.externalMaximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL;
    }

    if (!ShellExecuteExW(&sei)) {
        return false;
    }
    if (outProcess) {
        *outProcess = sei.hProcess;
    } else if (sei.hProcess) {
        CloseHandle(sei.hProcess);
    }
    return true;
}

void DecoyManager::StartWatchingNewWindow(HWND ownerHwnd, DWORD launchedPid, bool hideWhenFound) {
    watchingNewWindow_ = true;
    watchHideWhenFound_ = hideWhenFound;
    watchLaunchedPid_ = launchedPid;
    watchTicksRemaining_ = 40; // 40 * 200ms = 8 seconds max (near-zero CPU)
    if (ownerHwnd) {
        SetTimer(ownerHwnd, kPrewarmTimerId, 200, nullptr);
    }
}

void DecoyManager::StopWatchingNewWindow(HWND ownerHwnd) {
    watchingNewWindow_ = false;
    watchTicksRemaining_ = 0;
    preLaunchSnapshot_.clear();
    if (ownerHwnd) {
        KillTimer(ownerHwnd, kPrewarmTimerId);
    }
}

bool DecoyManager::OnPrewarmTimerTick(HWND ownerHwnd) {
    if (!watchingNewWindow_) {
        KillTimer(ownerHwnd, kPrewarmTimerId);
        return false;
    }

    --watchTicksRemaining_;
    if (watchTicksRemaining_ <= 0) {
        StopWatchingNewWindow(ownerHwnd);
        return true;
    }

    // Pass preLaunchSnapshot_ as excluded so existing windows are skipped in O(1) without OpenProcess
    auto current = WindowManager::EnumerateUserWindows(preLaunchSnapshot_);
    HWND foundByPid = nullptr;
    HWND foundNew = nullptr;

    for (const auto& info : current) {
        DWORD pid = 0;
        GetWindowThreadProcessId(info.hwnd, &pid);
        if (watchLaunchedPid_ != 0 && pid == watchLaunchedPid_) {
            foundByPid = info.hwnd;
            break;
        }
        if (!foundNew) {
            foundNew = info.hwnd;
        }
    }

    HWND target = foundByPid ? foundByPid : foundNew;
    if (target && IsWindow(target)) {
        externalHwnd_ = target;
        GetWindowThreadProcessId(target, &externalPid_);
        if (watchHideWhenFound_) {
            ShowWindowAsync(target, SW_HIDE);
        }
        StopWatchingNewWindow(ownerHwnd);
        return true;
    }
    return false;
}

void DecoyManager::Prepare(HWND ownerHwnd, HINSTANCE hInst, const AppConfig& cfg) {
    // Built-in decoys: pre-create hidden window & pre-decode image into RAM (<1ms show)
    if (cfg.decoyType == DecoyType::BuiltinNotes ||
        cfg.decoyType == DecoyType::BuiltinTerminal ||
        cfg.decoyType == DecoyType::BuiltinSpreadsheet ||
        cfg.decoyType == DecoyType::BuiltinImage) {
        builtin_.Prepare(hInst, cfg);
        return;
    }

    bool shouldPrewarm =
        (cfg.decoyType == DecoyType::ExternalAppOrFile && cfg.prewarmExternal && !utils::Trim(cfg.externalPath).empty()) ||
        (cfg.decoyType == DecoyType::ExternalUrl && cfg.prewarmUrl && !utils::Trim(cfg.urlAddress).empty());

    if (!shouldPrewarm) return;

    // Already have a valid pre-warmed window?
    if (externalHwnd_ && IsWindow(externalHwnd_)) {
        if (IsWindowVisible(externalHwnd_)) {
            ShowWindowAsync(externalHwnd_, SW_HIDE);
        }
        return;
    }

    externalHwnd_ = nullptr;
    externalPid_ = 0;
    preLaunchSnapshot_ = SnapshotVisibleUserWindows();

    HANDLE hProc = nullptr;
    if (LaunchExternalTarget(cfg, true, &hProc)) {
        DWORD pid = hProc ? GetProcessId(hProc) : 0;
        if (hProc) CloseHandle(hProc);
        StartWatchingNewWindow(ownerHwnd, pid, true);
    } else {
        preLaunchSnapshot_.clear();
    }
}

void DecoyManager::Activate(HWND ownerHwnd, HINSTANCE hInst, const AppConfig& cfg) {
    switch (cfg.decoyType) {
        case DecoyType::BuiltinNotes:
        case DecoyType::BuiltinTerminal:
        case DecoyType::BuiltinSpreadsheet:
        case DecoyType::BuiltinImage:
            builtin_.Show(hInst, cfg);
            break;

        case DecoyType::ExternalAppOrFile:
        case DecoyType::ExternalUrl: {
            if (externalHwnd_ && IsWindow(externalHwnd_)) {
                // INSTANT (<10ms) show of pre-warmed window!
                if (IsIconic(externalHwnd_)) {
                    ShowWindowAsync(externalHwnd_, cfg.externalMaximized ? SW_SHOWMAXIMIZED : SW_RESTORE);
                } else {
                    ShowWindowAsync(externalHwnd_, cfg.externalMaximized ? SW_SHOWMAXIMIZED : SW_SHOW);
                }
                utils::ForceForegroundWindow(externalHwnd_);
            } else {
                // On-demand fallback launch
                preLaunchSnapshot_ = SnapshotVisibleUserWindows();
                HANDLE hProc = nullptr;
                if (LaunchExternalTarget(cfg, false, &hProc)) {
                    DWORD pid = hProc ? GetProcessId(hProc) : 0;
                    if (hProc) CloseHandle(hProc);
                    // Watch so we know its HWND when un-panicking later
                    StartWatchingNewWindow(ownerHwnd, pid, false);
                } else {
                    preLaunchSnapshot_.clear();
                }
            }
            break;
        }

        case DecoyType::CapturedWindow: {
            HWND h = GetCapturedWindow();
            if (h) {
                if (IsIconic(h)) {
                    ShowWindowAsync(h, SW_RESTORE);
                } else {
                    ShowWindowAsync(h, SW_SHOW);
                }
                utils::ForceForegroundWindow(h);
            }
            break;
        }

        case DecoyType::CleanDesktop:
        default:
            break;
    }
}

void DecoyManager::Deactivate(const AppConfig& cfg) {
    switch (cfg.decoyType) {
        case DecoyType::BuiltinNotes:
        case DecoyType::BuiltinTerminal:
        case DecoyType::BuiltinSpreadsheet:
        case DecoyType::BuiltinImage:
            builtin_.Hide();
            break;

        case DecoyType::ExternalAppOrFile:
        case DecoyType::ExternalUrl: {
            if (externalHwnd_ && IsWindow(externalHwnd_)) {
                if (cfg.decoyRestoreAction == DecoyRestoreAction::KeepHidden) {
                    ShowWindowAsync(externalHwnd_, SW_HIDE);
                } else if (cfg.decoyRestoreAction == DecoyRestoreAction::Close) {
                    PostMessageW(externalHwnd_, WM_CLOSE, 0, 0);
                    externalHwnd_ = nullptr;
                    externalPid_ = 0;
                }
                // LeaveOpen: do nothing
            }
            break;
        }

        case DecoyType::CapturedWindow: {
            HWND h = GetCapturedWindow();
            if (h && !cfg.capturedKeepVisibleUntilPanic) {
                ShowWindowAsync(h, SW_HIDE);
                capturedWasHiddenByUs_ = true;
            }
            break;
        }

        case DecoyType::CleanDesktop:
        default:
            break;
    }
}

bool DecoyManager::IsPrewarmedReady(const AppConfig& cfg) const {
    switch (cfg.decoyType) {
        case DecoyType::BuiltinNotes:
        case DecoyType::BuiltinTerminal:
        case DecoyType::BuiltinSpreadsheet:
        case DecoyType::BuiltinImage:
        case DecoyType::CleanDesktop:
            return true;
        case DecoyType::ExternalAppOrFile:
        case DecoyType::ExternalUrl:
            return externalHwnd_ != nullptr && IsWindow(externalHwnd_);
        case DecoyType::CapturedWindow:
            return GetCapturedWindow() != nullptr;
    }
    return false;
}

std::wstring DecoyManager::GetStatusText(const AppConfig& cfg) const {
    bool sp = i18n::IsSpanish();
    switch (cfg.decoyType) {
        case DecoyType::BuiltinNotes:
            return sp ? L"Integrado listo en memoria (< 2 ms): Notas / Informe"
                      : L"Built-in ready in RAM (< 2 ms): Notes / Document";
        case DecoyType::BuiltinTerminal:
            return sp ? L"Integrado listo en memoria (< 2 ms): Terminal"
                      : L"Built-in ready in RAM (< 2 ms): Developer Terminal";
        case DecoyType::BuiltinSpreadsheet:
            return sp ? L"Integrado listo en memoria (< 2 ms): Hoja de cálculo"
                      : L"Built-in ready in RAM (< 2 ms): Spreadsheet / Metrics";
        case DecoyType::BuiltinImage:
            if (cfg.imagePath.empty()) {
                return sp ? L"Falta elegir imagen de camuflaje" : L"No camouflage image chosen yet";
            }
            return sp ? L"Imagen precargada en RAM (< 1 ms)" : L"Image pre-loaded in RAM (< 1 ms)";
        case DecoyType::ExternalAppOrFile:
        case DecoyType::ExternalUrl:
            if (externalHwnd_ && IsWindow(externalHwnd_)) {
                return sp ? L"Pre-cargado en segundo plano ✓ (< 10 ms)"
                          : L"Pre-warmed in background ✓ (< 10 ms)";
            }
            if (watchingNewWindow_) {
                return sp ? L"Pre-cargando ventana en segundo plano..."
                          : L"Pre-warming window in background...";
            }
            return sp ? L"Sin pre-cargar (se abrirá al pulsar el atajo)"
                      : L"Not pre-warmed (will launch on trigger)";
        case DecoyType::CapturedWindow:
            if (GetCapturedWindow()) {
                return (sp ? L"Ventana capturada lista ✓: " : L"Captured window ready ✓: ") + GetCapturedWindowTitle();
            }
            return sp ? L"Ninguna ventana capturada todavía" : L"No window captured yet";
        case DecoyType::CleanDesktop:
            return sp ? L"Escritorio limpio (sin ventana camuflaje)"
                      : L"Clean desktop (clear windows only)";
    }
    return {};
}
