#pragma once

#include "config.h"
#include "utils.h"

class BuiltinDecoyWindow {
public:
    BuiltinDecoyWindow() = default;
    ~BuiltinDecoyWindow();

    // Register window class & pre-load image/resources based on config
    void Prepare(HINSTANCE hInst, const AppConfig& cfg);

    // Show immediately (< 2 ms)
    HWND Show(HINSTANCE hInst, const AppConfig& cfg);

    // Hide immediately and stop any animation timer
    void Hide();

    // Destroy window and release GDI resources
    void Destroy();

    HWND GetHwnd() const { return hwnd_; }
    bool IsVisible() const { return hwnd_ && IsWindowVisible(hwnd_); }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    void EnsureWindowCreated(HINSTANCE hInst, const AppConfig& cfg);
    void ApplyModeAndContent(const AppConfig& cfg);
    void LayoutChildren();
    void LoadImageBitmap(const std::wstring& path);
    void PaintSpreadsheet(HDC hdc, const RECT& rc);
    void PaintImage(HDC hdc, const RECT& rc);
    void AppendSimulatedTerminalLine();
    void ClearIcons();

    HINSTANCE hInst_ = nullptr;
    HWND hwnd_ = nullptr;
    HWND editCtrl_ = nullptr;
    HWND statusBar_ = nullptr;

    DecoyType currentType_ = DecoyType::BuiltinNotes;
    bool darkMode_ = false;
    bool simulateTerminal_ = false;
    bool imageFullscreen_ = true;
    int sheetTemplate_ = 0;

    HFONT editFont_ = nullptr;
    HFONT uiFont_ = nullptr;
    HFONT uiBoldFont_ = nullptr;
    HBRUSH editBgBrush_ = nullptr;
    HICON iconBig_ = nullptr;
    HICON iconSmall_ = nullptr;

    // Pre-decoded image for <1ms display
    std::wstring loadedImagePath_;
    HBITMAP imageBmp_ = nullptr;
    int imageWidth_ = 0;
    int imageHeight_ = 0;

    // Interactive spreadsheet selection
    int selRow_ = 2;
    int selCol_ = 3;

    // Terminal simulator step
    int simStep_ = 0;
};
