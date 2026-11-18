#include "window_manager.h"

#include <dwmapi.h>
#include <fstream>
#include <sstream>
#include <cstdint>

#ifndef DWMWA_CLOAKED
#define DWMWA_CLOAKED 14
#endif

namespace {

const std::unordered_set<std::wstring>& GetIgnoredClassNames() {
    static const std::unordered_set<std::wstring> kIgnored = {
        L"Progman",
        L"WorkerW",
        L"Shell_TrayWnd",
        L"Shell_SecondaryTrayWnd",
        L"NotifyIconOverflowWindow",
        L"TopLevelWindowForOverflowXamlIsland",
        L"Windows.UI.Core.CoreWindow",
        L"XamlExplorerHostIslandWindow",
        L"Xaml_WindowedPopupClass",
        L"PopupHost",
        L"ForegroundStaging",
        L"MultitaskingViewFrame",
        L"ApplicationManager_DesktopShellWindow",
        L"EdgeUiInputTopWndClass",
        L"EdgeUiInputWndClass",
        L"Shell_InputSwitchTopLevelWindow",
        L"Shell_LightDismissOverlay",
        L"Shell_Dim",
        L"TaskListThumbnailWnd",
        L"DV2ControlHost",
        L"SysShadow",
        L"tooltips_class32",
        L"IME",
        L"MSCTFIME UI",
        L"MsgrIMEWindowClass",
        L"Windows.Internal.Shell.TabProxyWindow",
        L"DummyDWMListenerWindow",
        L"Internet Explorer_Hidden"
    };
    return kIgnored;
}

const std::unordered_set<std::wstring>& GetIgnoredSystemProcesses() {
    static const std::unordered_set<std::wstring> kProcs = {
        L"shellexperiencehost.exe",
        L"startmenuexperiencehost.exe",
        L"searchhost.exe",
        L"searchapp.exe",
        L"textinputhost.exe",
        L"widgets.exe",
        L"lockapp.exe",
        L"dwm.exe",
        L"csrss.exe",
        L"sihost.exe",
        L"ctfmon.exe",
        L"taskhostw.exe",
        L"fontdrvhost.exe",
        L"winlogon.exe",
        L"runtimebroker.exe"
    };
    return kProcs;
}

double NowMs() {
    static LARGE_INTEGER freq = []() {
        LARGE_INTEGER f{};
        QueryPerformanceFrequency(&f);
        return f;
    }();
    LARGE_INTEGER now{};
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart * 1000.0 / (double)freq.QuadPart;
}

} // namespace

std::wstring WindowManager::GetStateFilePath() {
    return utils::GetAppDataDir() + L"\\panic_state.dat";
}

bool WindowManager::IsUserWindow(
    HWND hwnd,
    DWORD ownPid,
    const std::unordered_set<HWND>& excludedHwnds,
    const std::unordered_set<std::wstring>& excludedProcs,
    std::unordered_map<DWORD, std::wstring>& pidNameCache) {

    if (!hwnd || !IsWindow(hwnd)) return false;
    if (hwnd == GetShellWindow() || hwnd == GetDesktopWindow()) return false;
    if (excludedHwnds.count(hwnd)) return false;

    if (!IsWindowVisible(hwnd)) return false;

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0 || (ownPid != 0 && pid == ownPid)) return false;

    // Skip cloaked windows (other virtual desktops, suspended UWP shell surfaces)
    DWORD cloaked = 0;
    if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked != 0) {
        return false;
    }

    // Check window class
    wchar_t className[256]{};
    int classLen = GetClassNameW(hwnd, className, (int)std::size(className));
    std::wstring cls(className, classLen > 0 ? classLen : 0);

    if (GetIgnoredClassNames().count(cls)) {
        return false;
    }

    // Handle Windows 10/11 UWP containers (ApplicationFrameWindow)
    if (cls == L"ApplicationFrameWindow") {
        HWND childCore = FindWindowExW(hwnd, nullptr, L"Windows.UI.Core.CoreWindow", nullptr);
        if (childCore) {
            DWORD childCloaked = 0;
            if (SUCCEEDED(DwmGetWindowAttribute(childCore, DWMWA_CLOAKED, &childCloaked, sizeof(childCloaked))) && childCloaked != 0) {
                return false;
            }
        }
        int titleLen = GetWindowTextLengthW(hwnd);
        if (titleLen == 0 && !childCore) {
            return false;
        }
    }

    // Lookup cached process name
    std::wstring procName;
    auto it = pidNameCache.find(pid);
    if (it != pidNameCache.end()) {
        procName = it->second;
    } else {
        procName = utils::GetProcessBaseName(pid);
        pidNameCache.emplace(pid, procName);
    }

    // Special rule for explorer.exe: ONLY hide actual File Explorer windows (CabinetWClass / ExploreWClass)
    if (procName == L"explorer.exe") {
        if (cls != L"CabinetWClass" && cls != L"ExploreWClass") {
            return false;
        }
    }

    if (!procName.empty()) {
        if (GetIgnoredSystemProcesses().count(procName)) return false;
        if (excludedProcs.count(procName)) return false;
    }

    bool iconic = IsIconic(hwnd) != 0;
    if (!iconic) {
        RECT rc{};
        if (!GetWindowRect(hwnd, &rc)) return false;
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;
        if (w <= 1 || h <= 1) return false;
    }

    // Skip invisible, transparent or untitled tool overlays
    int titleLen = GetWindowTextLengthW(hwnd);
    LONG_PTR exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if (titleLen == 0) {
        if (cls != L"CabinetWClass" && cls != L"ExploreWClass") {
            if ((exStyle & WS_EX_TOOLWINDOW) || (exStyle & WS_EX_TRANSPARENT)) {
                return false;
            }
        }
    } else if ((exStyle & WS_EX_TOOLWINDOW) && (exStyle & WS_EX_TRANSPARENT) && (exStyle & WS_EX_NOACTIVATE)) {
        return false;
    }

    return true;
}

PanicStats WindowManager::HideAllWindows(
    const std::unordered_set<HWND>& excludedHwnds,
    const std::vector<std::wstring>& excludedProcesses) {

    double t0 = NowMs();

    // If somehow called while already holding hidden windows, avoid losing previous ones
    if (!hiddenWindows_.empty()) {
        return lastHideStats_;
    }

    std::unordered_set<std::wstring> excludedProcSet;
    for (const auto& p : excludedProcesses) {
        if (!p.empty()) excludedProcSet.insert(utils::ToLower(p));
    }

    savedForeground_ = GetForegroundWindow();

    struct EnumCtx {
        DWORD ownPid;
        HWND fgHwnd;
        const std::unordered_set<HWND>* excludedHwnds;
        const std::unordered_set<std::wstring>* excludedProcs;
        std::unordered_map<DWORD, std::wstring> pidCache;
        std::vector<SavedWindowState> found;
    };

    EnumCtx ctx{};
    ctx.ownPid = GetCurrentProcessId();
    ctx.fgHwnd = savedForeground_;
    ctx.excludedHwnds = &excludedHwnds;
    ctx.excludedProcs = &excludedProcSet;
    ctx.found.reserve(64);

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto* c = reinterpret_cast<EnumCtx*>(lParam);
        if (WindowManager::IsUserWindow(hwnd, c->ownPid, *c->excludedHwnds, *c->excludedProcs, c->pidCache)) {
            SavedWindowState st{};
            st.hwnd = hwnd;
            st.wasMinimized = (IsIconic(hwnd) != 0);
            st.wasMaximized = (IsZoomed(hwnd) != 0);
            st.wasForeground = (hwnd == c->fgHwnd);
            c->found.push_back(st);
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&ctx));

    hiddenWindows_ = std::move(ctx.found);

    // Hide all asynchronously so even hung/slow windows never stall the switch
    for (const auto& st : hiddenWindows_) {
        ShowWindowAsync(st.hwnd, SW_HIDE);
    }

    double t1 = NowMs();
    lastHideStats_.windowCount = hiddenWindows_.size();
    lastHideStats_.elapsedMs = t1 - t0;

    // Persist state to disk for crash recovery AFTER issuing ShowWindowAsync so visual hide is 0-delay
    SaveStateToDisk();

    return lastHideStats_;
}

PanicStats WindowManager::RestoreAllWindows() {
    double t0 = NowMs();

    size_t restoredCount = 0;
    HWND fgToRestore = nullptr;

    // 1. Show all windows asynchronously without stealing focus or altering placement
    for (const auto& st : hiddenWindows_) {
        if (!IsWindow(st.hwnd)) continue;

        if (st.wasMinimized) {
            ShowWindowAsync(st.hwnd, SW_SHOWMINNOACTIVE);
        } else {
            // SW_SHOWNA preserves exact Normal OR Maximized state without altering placement
            ShowWindowAsync(st.hwnd, SW_SHOWNA);
        }
        if (st.wasForeground && !st.wasMinimized) {
            fgToRestore = st.hwnd;
        }
        ++restoredCount;
    }

    // 2. Exact Z-order reconstruction:
    // hiddenWindows_ was collected by EnumWindows in top-to-bottom Z-order.
    // DeferWindowPos with SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE restores the exact visual hierarchy.
    if (!hiddenWindows_.empty()) {
        HDWP hdwp = BeginDeferWindowPos((int)hiddenWindows_.size());
        if (hdwp) {
            HWND prev = HWND_TOP;
            for (const auto& st : hiddenWindows_) {
                if (!IsWindow(st.hwnd) || st.wasMinimized) continue;
                hdwp = DeferWindowPos(hdwp, st.hwnd, prev, 0, 0, 0, 0,
                                      SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
                if (!hdwp) break;
                prev = st.hwnd;
            }
            if (hdwp) {
                EndDeferWindowPos(hdwp);
            }
        }
    }

    // 3. Return focus to the original active foreground window
    if (fgToRestore && IsWindow(fgToRestore)) {
        utils::ForceForegroundWindow(fgToRestore);
    } else if (savedForeground_ && IsWindow(savedForeground_) && IsWindowVisible(savedForeground_) && !IsIconic(savedForeground_)) {
        utils::ForceForegroundWindow(savedForeground_);
    }

    hiddenWindows_.clear();
    savedForeground_ = nullptr;
    ClearStateOnDisk();

    double t1 = NowMs();
    lastRestoreStats_.windowCount = restoredCount;
    lastRestoreStats_.elapsedMs = t1 - t0;
    return lastRestoreStats_;
}

std::vector<VisibleWindowInfo> WindowManager::EnumerateUserWindows(
    const std::unordered_set<HWND>& excludedHwnds) {

    struct EnumCtx {
        DWORD ownPid;
        const std::unordered_set<HWND>* excludedHwnds;
        std::unordered_set<std::wstring> emptyProcs;
        std::unordered_map<DWORD, std::wstring> pidCache;
        std::vector<VisibleWindowInfo> list;
    };

    EnumCtx ctx{};
    ctx.ownPid = GetCurrentProcessId();
    ctx.excludedHwnds = &excludedHwnds;

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto* c = reinterpret_cast<EnumCtx*>(lParam);
        if (!WindowManager::IsUserWindow(hwnd, c->ownPid, *c->excludedHwnds, c->emptyProcs, c->pidCache)) {
            return TRUE;
        }
        // For user-facing window picker, prefer windows with a title
        int len = GetWindowTextLengthW(hwnd);
        if (len <= 0) return TRUE;

        std::wstring title(len, L'\0');
        GetWindowTextW(hwnd, title.data(), len + 1);

        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        std::wstring proc = c->pidCache[pid];

        VisibleWindowInfo info{};
        info.hwnd = hwnd;
        info.title = std::move(title);
        info.processName = std::move(proc);
        info.isMinimized = (IsIconic(hwnd) != 0);
        c->list.push_back(std::move(info));
        return TRUE;
    }, reinterpret_cast<LPARAM>(&ctx));

    return ctx.list;
}

void WindowManager::SaveStateToDisk() const {
    std::wstring path = GetStateFilePath();
    if (hiddenWindows_.empty()) {
        DeleteFileW(path.c_str());
        return;
    }

    std::ostringstream oss;
    oss << "PANIC_STATE_V1\n";
    for (const auto& st : hiddenWindows_) {
        oss << reinterpret_cast<uintptr_t>(st.hwnd) << " "
            << (st.wasMinimized ? 1 : 0) << " "
            << (st.wasMaximized ? 1 : 0) << "\n";
    }
    std::string data = oss.str();

    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, data.data(), (DWORD)data.size(), &written, nullptr);
        CloseHandle(hFile);
    }
}

void WindowManager::ClearStateOnDisk() const {
    DeleteFileW(GetStateFilePath().c_str());
}

size_t WindowManager::RecoverOrphanedWindowsIfAny() {
    std::wstring path = GetStateFilePath();
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return 0;

    DWORD sz = GetFileSize(hFile, nullptr);
    std::string raw;
    if (sz > 0 && sz < 1024 * 1024) {
        raw.resize(sz);
        DWORD rd = 0;
        ReadFile(hFile, raw.data(), sz, &rd, nullptr);
        raw.resize(rd);
    }
    CloseHandle(hFile);
    DeleteFileW(path.c_str());

    std::istringstream iss(raw);
    std::string header;
    if (!std::getline(iss, header) || header.find("PANIC_STATE_V1") == std::string::npos) {
        return 0;
    }

    struct Entry { HWND hwnd; bool min; };
    std::vector<Entry> entries;
    uintptr_t hVal = 0;
    int isMin = 0, isMax = 0;
    while (iss >> hVal >> isMin >> isMax) {
        entries.push_back({reinterpret_cast<HWND>(hVal), isMin != 0});
    }

    size_t restored = 0;
    for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
        if (it->hwnd && IsWindow(it->hwnd) && !IsWindowVisible(it->hwnd)) {
            ShowWindowAsync(it->hwnd, it->min ? SW_SHOWMINNOACTIVE : SW_SHOWNA);
            ++restored;
        }
    }
    return restored;
}
