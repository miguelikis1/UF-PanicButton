#pragma once

#include "utils.h"
#include <string>
#include <vector>

enum class DecoyType : int {
    BuiltinNotes = 0,
    BuiltinTerminal = 1,
    BuiltinSpreadsheet = 2,
    BuiltinImage = 3,
    ExternalAppOrFile = 4,
    ExternalUrl = 5,
    CapturedWindow = 6,
    CleanDesktop = 7
};

enum class DecoyRestoreAction : int {
    KeepHidden = 0,
    Close = 1,
    LeaveOpen = 2
};

enum class UrlBrowserMode : int {
    StandaloneWindow = 0, // Edge/Chrome --new-window
    AppWindow = 1,        // Edge/Chrome --app=
    DefaultBrowser = 2    // ShellExecute URL
};

enum class MouseTrigger : int {
    Disabled = 0,
    XButton1 = 1,  // Mouse Back button
    XButton2 = 2,  // Mouse Forward button
    AnyXButton = 3 // Either side button
};

struct AppConfig {
    // Hotkeys & Triggers
    UINT panicModifiers = MOD_CONTROL | MOD_ALT;
    UINT panicVk = 'P';
    UINT settingsModifiers = MOD_CONTROL | MOD_ALT | MOD_SHIFT;
    UINT settingsVk = 'P';
    MouseTrigger mouseTrigger = MouseTrigger::Disabled;

    // Behavior
    bool muteAudioOnPanic = true;
    bool pauseMediaOnPanic = true;
    bool stealthTray = false;
    bool startWithWindows = false;
    bool showStartupNotification = true;
    int uiTheme = 0;  // 0 = Auto (Windows), 1 = Light, 2 = Dark
    int language = 0; // 0 = Auto (Windows), 1 = English, 2 = Spanish

    // Decoy selection
    DecoyType decoyType = DecoyType::BuiltinNotes;

    // External App / File
    std::wstring externalPath;
    std::wstring externalArgs;
    bool externalMaximized = true;
    bool prewarmExternal = true;
    DecoyRestoreAction decoyRestoreAction = DecoyRestoreAction::KeepHidden;

    // External URL
    std::wstring urlAddress = L"https://es.wikipedia.org/wiki/Wikipedia:Portada";
    UrlBrowserMode urlBrowserMode = UrlBrowserMode::StandaloneWindow;
    bool prewarmUrl = true;

    // Captured existing window
    bool capturedKeepVisibleUntilPanic = false;

    // Built-in Notes
    std::wstring notesTitle = L"Informe_Trimestral_Q4.txt - Bloc de notas";
    std::wstring notesContent;
    int notesTheme = 0; // 0 = Auto, 1 = Light, 2 = Dark

    // Built-in Terminal
    std::wstring terminalTitle = L"Windows PowerShell - build-pipeline";
    std::wstring terminalContent;
    bool terminalSimulateActivity = true;

    // Built-in Spreadsheet
    std::wstring sheetTitle = L"Analisis_Operaciones_2026.xlsx - Hoja de cálculo";
    int sheetTemplate = 0; // 0 = KPIs, 1 = Project Tasks, 2 = Financial Budget

    // Built-in Image
    std::wstring imagePath;
    bool imageFullscreen = true;

    // Process exclusions (lowercase executable basenames separated by commas/newlines)
    std::wstring excludedProcesses;

    AppConfig();

    static std::wstring GetConfigPath();
    bool Load(const std::wstring& path = GetConfigPath());
    bool Save(const std::wstring& path = GetConfigPath()) const;

    std::vector<std::wstring> GetExcludedProcessList() const;

    static std::wstring GetDefaultNotesTemplate(int idx, bool spanish = true);
    static std::wstring GetDefaultTerminalTemplate(int idx, bool spanish = true);
};

// Manage HKCU\Software\Microsoft\Windows\CurrentVersion\Run
bool SetStartWithWindows(bool enable);
bool GetStartWithWindows();
