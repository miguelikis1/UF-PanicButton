#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <string>
#include <vector>

namespace utils {

std::string ToUtf8(const std::wstring& w);
std::wstring FromUtf8(const std::string& s);
std::wstring ToLower(std::wstring s);
std::wstring Trim(const std::wstring& s);
std::vector<std::wstring> SplitLinesOrCommas(const std::wstring& s);

std::wstring GetExePath();
std::wstring GetExeDir();
std::wstring GetAppDataDir();
std::wstring GetProcessBaseName(DWORD pid);

std::wstring FormatHotkey(UINT modifiers, UINT vk);
std::vector<std::wstring> FormatHotkeyParts(UINT modifiers, UINT vk);
std::wstring GetVirtualKeyName(UINT vk);

bool IsSystemDarkMode();
void ApplyWin11WindowStyle(HWND hwnd, bool dark);
void ApplyWin11ModernFrame(HWND hwnd, bool dark, COLORREF captionColor, COLORREF borderColor);
UINT GetDpiForWindowSafe(HWND hwnd);
int Scale(int value, UINT dpi);
HFONT CreateUIFont(UINT dpi, int ptSize, bool bold = false, const wchar_t* face = L"Segoe UI");

bool ForceForegroundWindow(HWND hwnd);

bool IsRunningAsAdmin();
bool RelaunchAsAdmin();

double GetProcessMemoryMB();

// Programmatic crisp GDI+ icons (caller must DestroyIcon)
enum class DecoyIconKind {
    Document,
    Terminal,
    Spreadsheet,
    Image
};
HICON CreateAppIcon(int size, bool panicActive);
HICON CreateDecoyIcon(int size, DecoyIconKind kind);

std::wstring FindChromiumBrowserPath(bool preferChrome = false);

// Escape/unescape newlines for INI storage
std::wstring EscapeIniValue(const std::wstring& s);
std::wstring UnescapeIniValue(const std::wstring& s);

} // namespace utils
