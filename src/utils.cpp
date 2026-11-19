#include "utils.h"

#include <objidl.h>
#include <gdiplus.h>
#include <dwmapi.h>
#include <psapi.h>
#include <shlobj.h>
#include <shellapi.h>
#include <algorithm>
#include <cwctype>
#include <sstream>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif
#ifndef DWMWA_TEXT_COLOR
#define DWMWA_TEXT_COLOR 36
#endif
#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif

namespace utils {

std::string ToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string out(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), out.data(), len, nullptr, nullptr);
    return out;
}

std::wstring FromUtf8(const std::string& s) {
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    if (len <= 0) return {};
    std::wstring out(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), len);
    return out;
}

std::wstring ToLower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) {
        return (wchar_t)towlower(c);
    });
    return s;
}

std::wstring Trim(const std::wstring& s) {
    size_t start = 0;
    while (start < s.size() && iswspace(s[start])) ++start;
    size_t end = s.size();
    while (end > start && iswspace(s[end - 1])) --end;
    return s.substr(start, end - start);
}

std::vector<std::wstring> SplitLinesOrCommas(const std::wstring& s) {
    std::vector<std::wstring> result;
    std::wstring cur;
    for (wchar_t c : s) {
        if (c == L',' || c == L';' || c == L'\n' || c == L'\r') {
            auto t = Trim(cur);
            if (!t.empty()) result.push_back(t);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    auto t = Trim(cur);
    if (!t.empty()) result.push_back(t);
    return result;
}

std::wstring GetExePath() {
    wchar_t buf[MAX_PATH * 4]{};
    DWORD len = GetModuleFileNameW(nullptr, buf, (DWORD)(std::size(buf) - 1));
    return std::wstring(buf, len);
}

std::wstring GetExeDir() {
    std::wstring path = GetExePath();
    auto pos = path.find_last_of(L"\\/");
    return (pos == std::wstring::npos) ? L"." : path.substr(0, pos);
}

std::wstring GetAppDataDir() {
    PWSTR appData = nullptr;
    std::wstring dir;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appData)) && appData) {
        dir = std::wstring(appData) + L"\\PanicButton";
        CoTaskMemFree(appData);
    } else {
        dir = GetExeDir();
    }
    CreateDirectoryW(dir.c_str(), nullptr);

    // Seamless migration: If old BotonDelPanico folder exists, migrate existing config
    std::wstring newCfg = dir + L"\\config.ini";
    if (GetFileAttributesW(newCfg.c_str()) == INVALID_FILE_ATTRIBUTES) {
        PWSTR oldAppData = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &oldAppData)) && oldAppData) {
            std::wstring oldCfg = std::wstring(oldAppData) + L"\\BotonDelPanico\\config.ini";
            CoTaskMemFree(oldAppData);
            if (GetFileAttributesW(oldCfg.c_str()) != INVALID_FILE_ATTRIBUTES) {
                CopyFileW(oldCfg.c_str(), newCfg.c_str(), TRUE);
            }
        }
    }

    return dir;
}

std::wstring GetProcessBaseName(DWORD pid) {
    if (pid == 0) return {};
    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProc) return {};
    wchar_t buf[MAX_PATH * 2]{};
    DWORD sz = (DWORD)std::size(buf);
    std::wstring name;
    if (QueryFullProcessImageNameW(hProc, 0, buf, &sz)) {
        std::wstring full(buf, sz);
        auto pos = full.find_last_of(L"\\/");
        name = (pos == std::wstring::npos) ? full : full.substr(pos + 1);
    }
    CloseHandle(hProc);
    return ToLower(name);
}

std::wstring GetVirtualKeyName(UINT vk) {
    switch (vk) {
        case VK_SPACE: return L"Espacio";
        case VK_RETURN: return L"Enter";
        case VK_TAB: return L"Tab";
        case VK_ESCAPE: return L"Esc";
        case VK_BACK: return L"Retroceso";
        case VK_DELETE: return L"Supr";
        case VK_INSERT: return L"Insert";
        case VK_HOME: return L"Inicio";
        case VK_END: return L"Fin";
        case VK_PRIOR: return L"RePág";
        case VK_NEXT: return L"AvPág";
        case VK_PAUSE: return L"Pausa";
        case VK_SCROLL: return L"BloqDespl";
        case VK_SNAPSHOT: return L"ImprPant";
        case VK_UP: return L"Arriba";
        case VK_DOWN: return L"Abajo";
        case VK_LEFT: return L"Izquierda";
        case VK_RIGHT: return L"Derecha";
        case VK_OEM_3: return L"º";
        case VK_OEM_MINUS: return L"-";
        case VK_OEM_PLUS: return L"+";
        case VK_OEM_COMMA: return L",";
        case VK_OEM_PERIOD: return L".";
        case VK_MULTIPLY: return L"Num *";
        case VK_ADD: return L"Num +";
        case VK_SUBTRACT: return L"Num -";
        case VK_DIVIDE: return L"Num /";
        case VK_DECIMAL: return L"Num .";
        default: break;
    }
    if (vk >= VK_F1 && vk <= VK_F24) {
        return L"F" + std::to_wstring(vk - VK_F1 + 1);
    }
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) {
        return L"Num " + std::to_wstring(vk - VK_NUMPAD0);
    }
    if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z')) {
        return std::wstring(1, (wchar_t)vk);
    }
    UINT scan = MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
    wchar_t nameBuf[64]{};
    if (scan != 0 && GetKeyNameTextW((LONG)(scan << 16), nameBuf, 64) > 0) {
        return nameBuf;
    }
    wchar_t hexBuf[32]{};
    swprintf_s(hexBuf, L"VK_0x%02X", vk);
    return hexBuf;
}

std::vector<std::wstring> FormatHotkeyParts(UINT modifiers, UINT vk) {
    std::vector<std::wstring> parts;
    if (vk == 0) return parts;
    if (modifiers & MOD_CONTROL) parts.push_back(L"Ctrl");
    if (modifiers & MOD_ALT)     parts.push_back(L"Alt");
    if (modifiers & MOD_SHIFT)   parts.push_back(L"Shift");
    if (modifiers & MOD_WIN)     parts.push_back(L"Win");
    parts.push_back(GetVirtualKeyName(vk));
    return parts;
}

std::wstring FormatHotkey(UINT modifiers, UINT vk) {
    auto parts = FormatHotkeyParts(modifiers, vk);
    if (parts.empty()) return L"(Sin asignar)";
    std::wstring res;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) res += L" + ";
        res += parts[i];
    }
    return res;
}

bool IsSystemDarkMode() {
    DWORD val = 1;
    DWORD sz = sizeof(val);
    if (RegGetValueW(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            L"AppsUseLightTheme",
            RRF_RT_REG_DWORD,
            nullptr,
            &val,
            &sz) == ERROR_SUCCESS) {
        return val == 0;
    }
    return false;
}

void ApplyWin11WindowStyle(HWND hwnd, bool dark) {
    if (!hwnd) return;
    BOOL useDark = dark ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDark, sizeof(useDark));
    DWORD corner = DWMWCP_ROUND;
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));
}

void ApplyWin11ModernFrame(HWND hwnd, bool dark, COLORREF captionColor, COLORREF borderColor) {
    ApplyWin11WindowStyle(hwnd, dark);
    if (!hwnd) return;
    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));
    COLORREF textCol = dark ? RGB(238, 240, 246) : RGB(24, 26, 31);
    DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &textCol, sizeof(textCol));
    DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &borderColor, sizeof(borderColor));
}

UINT GetDpiForWindowSafe(HWND hwnd) {
    using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
    static auto fn = reinterpret_cast<GetDpiForWindowFn>(
        reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow")));
    if (fn && hwnd) {
        UINT dpi = fn(hwnd);
        if (dpi >= 96) return dpi;
    }
    HDC hdc = GetDC(hwnd);
    UINT dpi = hdc ? (UINT)GetDeviceCaps(hdc, LOGPIXELSX) : 96;
    if (hdc) ReleaseDC(hwnd, hdc);
    return dpi >= 96 ? dpi : 96;
}

int Scale(int value, UINT dpi) {
    return MulDiv(value, (int)dpi, 96);
}

HFONT CreateUIFont(UINT dpi, int ptSize, bool bold, const wchar_t* face) {
    int height = -MulDiv(ptSize, (int)dpi, 72);
    return CreateFontW(
        height, 0, 0, 0,
        bold ? FW_SEMIBOLD : FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        face);
}

bool ForceForegroundWindow(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) return false;

    if (GetForegroundWindow() == hwnd) return true;

    // Ensure the target window is shown
    if (!IsWindowVisible(hwnd)) {
        ShowWindow(hwnd, SW_SHOW);
    }

    // Modern clean Windows foreground unlock:
    // A synthetic Alt keypress grants foreground activation rights without attaching thread inputs.
    AllowSetForegroundWindow(ASFW_ANY);
    keybd_event(VK_MENU, 0, 0, 0);
    keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);

    BringWindowToTop(hwnd);
    BOOL ok = SetForegroundWindow(hwnd);

    // Fallback: brief pulse of topmost strictly without focus hijack, immediately cleared
    if (!ok || GetForegroundWindow() != hwnd) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        ok = SetForegroundWindow(hwnd);
    }

    // Strict guarantee: Never leave WS_EX_TOPMOST on the window to prevent Z-order trapping
    LONG_PTR exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOPMOST) {
        SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TOPMOST);
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }

    return ok != 0;
}

bool IsRunningAsAdmin() {
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID adminGroup = nullptr;
    if (AllocateAndInitializeSid(&ntAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin != FALSE;
}

bool RelaunchAsAdmin() {
    std::wstring exe = GetExePath();
    SHELLEXECUTEINFOW sei{};
    sei.cbSize = sizeof(sei);
    sei.lpVerb = L"runas";
    sei.lpFile = exe.c_str();
    sei.lpParameters = L"--settings";
    sei.nShow = SW_SHOWNORMAL;
    return ShellExecuteExW(&sei) != FALSE;
}

double GetProcessMemoryMB() {
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
    }
    return 0.0;
}

static void AddRoundedRectPath(Gdiplus::GraphicsPath& path, float x, float y, float w, float h, float r) {
    float d = r * 2.0f;
    path.AddArc(x, y, d, d, 180.0f, 90.0f);
    path.AddArc(x + w - d, y, d, d, 270.0f, 90.0f);
    path.AddArc(x + w - d, y + h - d, d, d, 0.0f, 90.0f);
    path.AddArc(x, y + h - d, d, d, 90.0f, 90.0f);
    path.CloseFigure();
}

HICON CreateAppIcon(int size, bool panicActive) {
    Gdiplus::Bitmap bmp(size, size, PixelFormat32bppARGB);
    Gdiplus::Graphics g(&bmp);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.Clear(Gdiplus::Color(0, 0, 0, 0));

    float s = (float)size;
    float pad = s * 0.06f;

    // Background rounded badge: subtle slate-blue when idle, neutral muted gray when panic active
    Gdiplus::GraphicsPath bg;
    AddRoundedRectPath(bg, pad, pad, s - 2 * pad, s - 2 * pad, s * 0.24f);

    Gdiplus::Color cTop = panicActive ? Gdiplus::Color(255, 95, 108, 122) : Gdiplus::Color(255, 224, 58, 72);
    Gdiplus::Color cBot = panicActive ? Gdiplus::Color(255, 62, 72, 84)   : Gdiplus::Color(255, 170, 26, 44);
    Gdiplus::LinearGradientBrush grad(
        Gdiplus::PointF(0, pad),
        Gdiplus::PointF(0, s - pad),
        cTop, cBot);
    g.FillPath(&grad, &bg);

    // Inner stylized shield / lightning / eye glyph
    Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(245, 255, 255, 255));
    if (!panicActive) {
        // Clean bold shield / button ring
        Gdiplus::Pen ringPen(Gdiplus::Color(240, 255, 255, 255), s * 0.10f);
        g.DrawEllipse(&ringPen, s * 0.25f, s * 0.25f, s * 0.50f, s * 0.50f);
        g.FillEllipse(&whiteBrush, s * 0.40f, s * 0.40f, s * 0.20f, s * 0.20f);
    } else {
        // Subtle "hidden" horizontal slash icon
        Gdiplus::Pen pen(Gdiplus::Color(235, 255, 255, 255), s * 0.11f);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawLine(&pen, s * 0.26f, s * 0.50f, s * 0.74f, s * 0.50f);
    }

    HICON hIcon = nullptr;
    bmp.GetHICON(&hIcon);
    return hIcon;
}

HICON CreateDecoyIcon(int size, DecoyIconKind kind) {
    Gdiplus::Bitmap bmp(size, size, PixelFormat32bppARGB);
    Gdiplus::Graphics g(&bmp);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.Clear(Gdiplus::Color(0, 0, 0, 0));

    float s = (float)size;
    float pad = s * 0.08f;
    Gdiplus::GraphicsPath bg;
    AddRoundedRectPath(bg, pad, pad, s - 2 * pad, s - 2 * pad, s * 0.18f);

    if (kind == DecoyIconKind::Document) {
        Gdiplus::SolidBrush fill(Gdiplus::Color(255, 34, 118, 206));
        g.FillPath(&fill, &bg);
        Gdiplus::Pen linePen(Gdiplus::Color(245, 255, 255, 255), s * 0.08f);
        g.DrawLine(&linePen, s * 0.26f, s * 0.34f, s * 0.74f, s * 0.34f);
        g.DrawLine(&linePen, s * 0.26f, s * 0.52f, s * 0.74f, s * 0.52f);
        g.DrawLine(&linePen, s * 0.26f, s * 0.70f, s * 0.58f, s * 0.70f);
    } else if (kind == DecoyIconKind::Terminal) {
        Gdiplus::SolidBrush fill(Gdiplus::Color(255, 28, 32, 38));
        g.FillPath(&fill, &bg);
        Gdiplus::Pen border(Gdiplus::Color(255, 90, 98, 110), s * 0.05f);
        g.DrawPath(&border, &bg);
        Gdiplus::Pen p(Gdiplus::Color(255, 235, 235, 235), s * 0.09f);
        p.SetLineJoin(Gdiplus::LineJoinRound);
        g.DrawLine(&p, s * 0.24f, s * 0.34f, s * 0.44f, s * 0.50f);
        g.DrawLine(&p, s * 0.44f, s * 0.50f, s * 0.24f, s * 0.66f);
        g.DrawLine(&p, s * 0.52f, s * 0.66f, s * 0.76f, s * 0.66f);
    } else if (kind == DecoyIconKind::Spreadsheet) {
        Gdiplus::SolidBrush fill(Gdiplus::Color(255, 24, 124, 68));
        g.FillPath(&fill, &bg);
        Gdiplus::Pen grid(Gdiplus::Color(235, 255, 255, 255), s * 0.06f);
        g.DrawRectangle(&grid, s * 0.22f, s * 0.24f, s * 0.56f, s * 0.52f);
        g.DrawLine(&grid, s * 0.22f, s * 0.44f, s * 0.78f, s * 0.44f);
        g.DrawLine(&grid, s * 0.22f, s * 0.60f, s * 0.78f, s * 0.60f);
        g.DrawLine(&grid, s * 0.48f, s * 0.24f, s * 0.48f, s * 0.76f);
    } else {
        Gdiplus::SolidBrush fill(Gdiplus::Color(255, 58, 128, 198));
        g.FillPath(&fill, &bg);
        Gdiplus::SolidBrush white(Gdiplus::Color(240, 255, 255, 255));
        Gdiplus::PointF pts[3] = {
            {s * 0.22f, s * 0.74f},
            {s * 0.48f, s * 0.38f},
            {s * 0.78f, s * 0.74f}
        };
        g.FillPolygon(&white, pts, 3);
    }

    HICON hIcon = nullptr;
    bmp.GetHICON(&hIcon);
    return hIcon;
}

std::wstring FindChromiumBrowserPath(bool preferChrome) {
    std::vector<std::wstring> edgePaths = {
        L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe",
        L"C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe"
    };
    std::vector<std::wstring> chromePaths = {
        L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
        L"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe"
    };

    wchar_t localApp[MAX_PATH]{};
    if (GetEnvironmentVariableW(L"LOCALAPPDATA", localApp, MAX_PATH) > 0) {
        chromePaths.push_back(std::wstring(localApp) + L"\\Google\\Chrome\\Application\\chrome.exe");
        edgePaths.push_back(std::wstring(localApp) + L"\\Microsoft\\Edge\\Application\\msedge.exe");
    }

    auto checkList = [](const std::vector<std::wstring>& list) -> std::wstring {
        for (const auto& p : list) {
            DWORD attr = GetFileAttributesW(p.c_str());
            if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                return p;
            }
        }
        return {};
    };

    if (preferChrome) {
        auto c = checkList(chromePaths);
        if (!c.empty()) return c;
        return checkList(edgePaths);
    } else {
        auto e = checkList(edgePaths);
        if (!e.empty()) return e;
        return checkList(chromePaths);
    }
}

std::wstring EscapeIniValue(const std::wstring& s) {
    std::wstring out;
    out.reserve(s.size() + 16);
    for (wchar_t c : s) {
        if (c == L'\\') out += L"\\\\";
        else if (c == L'\n') out += L"\\n";
        else if (c == L'\r') out += L"\\r";
        else out.push_back(c);
    }
    return out;
}

std::wstring UnescapeIniValue(const std::wstring& s) {
    std::wstring out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == L'\\' && i + 1 < s.size()) {
            wchar_t n = s[i + 1];
            if (n == L'n') { out.push_back(L'\n'); ++i; continue; }
            if (n == L'r') { out.push_back(L'\r'); ++i; continue; }
            if (n == L'\\') { out.push_back(L'\\'); ++i; continue; }
        }
        out.push_back(s[i]);
    }
    return out;
}

} // namespace utils
