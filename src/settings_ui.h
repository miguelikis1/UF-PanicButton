#pragma once

#include "config.h"
#include "window_manager.h"
#include "i18n.h"
#include <functional>
#include <vector>
#include <string>

struct SettingsCallbacks {
    std::function<bool(const AppConfig& newCfg, std::wstring& errorMsg)> onApplyConfig;
    std::function<void()> onTogglePanic;
    std::function<void()> onPrepareDecoy;
    std::function<void(HWND hwnd, bool hideNow)> onCaptureWindow;
    std::function<void()> onReleaseCapturedWindow;
    std::function<void()> onBeginHotkeyCapture;
    std::function<void()> onEndHotkeyCapture;
    std::function<void()> onExitApp;
    std::function<void()> onRelaunchAdmin;

    std::function<bool()> isPanicActive;
    std::function<std::wstring()> getDecoyStatus;
    std::function<std::wstring()> getCapturedWindowTitle;
    std::function<PanicStats()> getLastHideStats;
    std::function<PanicStats()> getLastRestoreStats;
    std::function<size_t()> getHiddenCount;
};

class SettingsWindow {
public:
    SettingsWindow() = default;
    ~SettingsWindow();

    void Show(HINSTANCE hInst, const AppConfig& cfg, SettingsCallbacks callbacks);
    void Hide();
    void RefreshStatus(const AppConfig& cfg);
    void Destroy();

    HWND GetHwnd() const { return hwnd_; }
    bool IsVisible() const { return hwnd_ && IsWindowVisible(hwnd_); }
    bool IsCapturingHotkey() const { return captureSlot_ != 0; }

private:
    enum class HitId : int {
        None = 0,
        NavGeneral,
        NavDecoy,
        NavEngine,
        BtnThemeToggle,
        BtnTestPanic,
        BtnMinimize,
        BtnExit,
        BtnApply,

        // Tab 0
        HotkeyPanicBox,
        HotkeySettingsBox,
        Preset1,
        Preset2,
        Preset3,
        Preset4,
        ToggleMute,
        TogglePauseMedia,
        ToggleStealth,
        ToggleStartWin,
        ToggleStartTip,
        BtnRelaunchAdmin,

        // Tab 1 Decoy Tiles (8)
        DecoyTile0,
        DecoyTile1,
        DecoyTile2,
        DecoyTile3,
        DecoyTile4,
        DecoyTile5,
        DecoyTile6,
        DecoyTile7,

        // Tab 1 Sub-actions
        BtnNotesLoadTpl,
        BtnTermLoadTpl,
        ToggleTermSim,
        BtnImgBrowse,
        ToggleImgFullscreen,
        BtnExtBrowse,
        ToggleExtMax,
        ToggleExtPrewarm,
        BtnExtPrepare,
        ToggleUrlPrewarm,
        BtnUrlPrepare,
        BtnCapRefresh,
        ToggleCapKeepVis,
        BtnCapCapture,
        BtnCapRelease,

        // Tab 2
        BtnDiagRefresh,
        BtnDiagRestore
    };

    struct HitRegion {
        HitId id = HitId::None;
        RECT rc{};
        bool enabled = true;
    };

    struct ManagedChild {
        HWND hwnd = nullptr;
        int tab = 0;           // 0, 1, 2
        int decoyFilter = -1;  // -1 = always on that tab, else (int)DecoyType
        int baseX = 0;         // unscaled (96 DPI) coordinates
        int baseY = 0;
        int baseW = 0;
        int baseH = 0;
    };

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    void CreateChildControls();
    void ApplyTheme(bool dark);
    void PopulateFromConfig(const AppConfig& cfg);
    AppConfig GatherConfig() const;
    void UpdateComboLanguageTexts();

    void SelectTab(int newTab);
    void SelectDecoy(DecoyType newType);
    void StartAnimTimer();
    void StopAnimTimerIfIdle();
    void TickAnimations();
    void LayoutActiveChildren(int slideOffsetPx);
    void ShowToast(const std::wstring& msg, bool isError);

    void RefreshOpenWindowsList();
    void BrowseForFile(HWND targetEdit, const wchar_t* filter, const wchar_t* title);
    void BeginHotkeyCapture(int slot);
    void EndHotkeyCapture();
    void HandleHitClick(HitId id);

    // Master double-buffered GDI+ renderer
    void Render(HDC targetDc);
    HitId HitTest(int x, int y) const;

    HINSTANCE hInst_ = nullptr;
    HWND hwnd_ = nullptr;
    AppConfig workingCfg_;
    SettingsCallbacks cb_;

    UINT dpi_ = 96;
    bool dark_ = true;
    int uiThemePref_ = 0; // 0 = auto, 1 = light, 2 = dark

    // GDI resources for native child controls (EDIT / COMBOBOX)
    HFONT fontInput_ = nullptr;
    HFONT fontMono_ = nullptr;
    HBRUSH inputBgBrush_ = nullptr;
    HBRUSH cardBgBrush_ = nullptr;
    HICON iconBig_ = nullptr;
    HICON iconSmall_ = nullptr;

    // Navigation & animation state
    int activeTab_ = 0;
    float pillY_ = 162.0f;
    float pillTargetY_ = 162.0f;
    float pageAnim_ = 1.0f;      // 0 -> 1 ease-out cubic on tab switch
    float subCardAnim_ = 1.0f;   // 0 -> 1 ease-out cubic on decoy switch
    bool animTimerRunning_ = false;
    ULONGLONG lastAnimTick_ = 0;

    // Smooth toggle switch visual states (0.0 = off, 1.0 = on)
    struct ToggleState {
        bool value = false;
        float visual = 0.0f;
    };
    ToggleState togMute_{true, 1.0f};
    ToggleState togPauseMedia_{true, 1.0f};
    ToggleState togStealth_{false, 0.0f};
    ToggleState togStartWin_{false, 0.0f};
    ToggleState togStartTip_{true, 1.0f};
    ToggleState togTermSim_{true, 1.0f};
    ToggleState togImgFull_{true, 1.0f};
    ToggleState togExtMax_{true, 1.0f};
    ToggleState togExtPrewarm_{true, 1.0f};
    ToggleState togUrlPrewarm_{true, 1.0f};
    ToggleState togCapKeepVis_{false, 0.0f};

    // Toast notification state
    std::wstring toastText_;
    bool toastIsError_ = false;
    float toastAlpha_ = 0.0f;
    ULONGLONG toastExpireTick_ = 0;

    // Hover & Press tracking
    HitId hoveredHit_ = HitId::None;
    HitId pressedHit_ = HitId::None;
    bool mouseTracked_ = false;
    mutable std::vector<HitRegion> hitRegions_;

    // Hotkey capture state
    int captureSlot_ = 0; // 0 = none, 1 = panic, 2 = settings
    UINT recPanicMods_ = MOD_CONTROL | MOD_ALT;
    UINT recPanicVk_ = 'P';
    UINT recSettingsMods_ = MOD_CONTROL | MOD_ALT | MOD_SHIFT;
    UINT recSettingsVk_ = 'P';

    // Active decoy selection
    DecoyType selDecoy_ = DecoyType::BuiltinNotes;

    // Native child controls
    std::vector<ManagedChild> children_;
    HWND comboMouseTrigger_ = nullptr;
    HWND comboLanguage_ = nullptr;

    HWND editNotesTitle_ = nullptr;
    HWND comboNotesTpl_ = nullptr;
    HWND comboNotesTheme_ = nullptr;
    HWND editNotesContent_ = nullptr;

    HWND editTermTitle_ = nullptr;
    HWND comboTermTpl_ = nullptr;
    HWND editTermContent_ = nullptr;

    HWND editSheetTitle_ = nullptr;
    HWND comboSheetTpl_ = nullptr;

    HWND editImagePath_ = nullptr;

    HWND editExtPath_ = nullptr;
    HWND editExtArgs_ = nullptr;
    HWND comboExtRestore_ = nullptr;

    HWND editUrlAddr_ = nullptr;
    HWND comboUrlMode_ = nullptr;
    HWND comboUrlRestore_ = nullptr;

    HWND comboOpenWindows_ = nullptr;
    std::vector<HWND> openWindowHandles_;

    HWND editExclusions_ = nullptr;
};
