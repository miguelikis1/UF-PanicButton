#include "../src/utils.h"
#include "../src/config.h"
#include "../src/i18n.h"
#include "../src/audio_control.h"
#include "../src/window_manager.h"
#include "../src/builtin_decoys.h"

#include <objidl.h>
#include <gdiplus.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

static int g_failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::wcerr << L"[FAIL] " << msg << L" (" << #cond << L")\n"; \
        ++g_failures; \
    } else { \
        std::wcout << L"[PASS] " << msg << L"\n"; \
    } \
} while (0)

// Helper child process mode: creates 3 real Win32 windows (normal, maximized, minimized),
// writes their HWNDs to a temp file, and pumps messages until signalled to exit.
static int RunChildDummyWindows(const std::wstring& infoPath, const std::wstring& eventName) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"PanicTestDummyClass";
    RegisterClassExW(&wc);

    HWND hNormal = CreateWindowExW(0, L"PanicTestDummyClass", L"Dummy Normal",
                                   WS_OVERLAPPEDWINDOW, 100, 100, 320, 220,
                                   nullptr, nullptr, wc.hInstance, nullptr);
    HWND hMax = CreateWindowExW(0, L"PanicTestDummyClass", L"Dummy Maximized",
                                WS_OVERLAPPEDWINDOW, 120, 120, 320, 220,
                                nullptr, nullptr, wc.hInstance, nullptr);
    HWND hMin = CreateWindowExW(0, L"PanicTestDummyClass", L"Dummy Minimized",
                                WS_OVERLAPPEDWINDOW, 140, 140, 320, 220,
                                nullptr, nullptr, wc.hInstance, nullptr);
    HWND hHidden = CreateWindowExW(0, L"PanicTestDummyClass", L"Dummy Hidden",
                                   WS_OVERLAPPEDWINDOW, 160, 160, 320, 220,
                                   nullptr, nullptr, wc.hInstance, nullptr);

    ShowWindow(hNormal, SW_SHOWNOACTIVATE);
    ShowWindow(hMax, SW_SHOWMAXIMIZED);
    ShowWindow(hMin, SW_SHOWMINNOACTIVE);
    ShowWindow(hHidden, SW_HIDE);

    HANDLE hFile = CreateFileW(infoPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[256]{};
        int n = snprintf(buf, sizeof(buf), "%llu %llu %llu %llu\n",
                         (unsigned long long)(uintptr_t)hNormal,
                         (unsigned long long)(uintptr_t)hMax,
                         (unsigned long long)(uintptr_t)hMin,
                         (unsigned long long)(uintptr_t)hHidden);
        DWORD written = 0;
        WriteFile(hFile, buf, (DWORD)n, &written, nullptr);
        CloseHandle(hFile);
    }

    HANDLE hReady = OpenEventW(EVENT_MODIFY_STATE, FALSE, (eventName + L"_READY").c_str());
    if (hReady) { SetEvent(hReady); CloseHandle(hReady); }

    HANDLE hQuit = OpenEventW(SYNCHRONIZE, FALSE, (eventName + L"_QUIT").c_str());
    MSG msg{};
    while (true) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (hQuit && WaitForSingleObject(hQuit, 15) == WAIT_OBJECT_0) break;
    }

    if (hQuit) CloseHandle(hQuit);
    DestroyWindow(hNormal);
    DestroyWindow(hMax);
    DestroyWindow(hMin);
    DestroyWindow(hHidden);
    return 0;
}

int wmain(int argc, wchar_t** argv) {
    if (argc >= 4 && std::wstring(argv[1]) == L"--child") {
        return RunChildDummyWindows(argv[2], argv[3]);
    }

    Gdiplus::GdiplusStartupInput gdiIn;
    ULONG_PTR gdiToken = 0;
    Gdiplus::GdiplusStartup(&gdiToken, &gdiIn, nullptr);

    std::wcout << L"=== Ejecutando pruebas de Boton del Panico ===\n";

    // 1. i18n in-memory string catalog & language resolution
    {
        i18n::SetConfiguredLanguage(i18n::Language::Spanish);
        CHECK(wcscmp(i18n::Get(i18n::StrId::PanicHotkeyLabel), L"Atajo de Pánico") == 0, L"i18n Spanish translation");
        CHECK(wcscmp(i18n::Get(i18n::StrId::NavGeneral), L"General y Atajos") == 0, L"i18n Spanish NavGeneral");

        i18n::SetConfiguredLanguage(i18n::Language::English);
        CHECK(wcscmp(i18n::Get(i18n::StrId::PanicHotkeyLabel), L"Panic Shortcut") == 0, L"i18n English translation");
        CHECK(wcscmp(i18n::Get(i18n::StrId::NavCamouflage), L"Camouflage") == 0, L"i18n English NavCamouflage");

        i18n::SetConfiguredLanguage(i18n::Language::Auto);
        CHECK(wcslen(i18n::Get(i18n::StrId::AppName)) > 0, L"i18n Auto system language detection");
    }

    // 2. Hotkey formatting & key parts
    CHECK(utils::FormatHotkey(MOD_CONTROL | MOD_ALT, 'P') == L"Ctrl + Alt + P", L"FormatHotkey Ctrl+Alt+P");
    CHECK(utils::FormatHotkey(0, VK_F9) == L"F9", L"FormatHotkey F9");
    CHECK(utils::FormatHotkey(0, VK_PAUSE) == L"Pausa", L"FormatHotkey Pause");
    CHECK(utils::FormatHotkey(MOD_CONTROL | MOD_SHIFT, VK_SPACE) == L"Ctrl + Shift + Espacio", L"FormatHotkey Ctrl+Shift+Espacio");

    auto parts = utils::FormatHotkeyParts(MOD_CONTROL | MOD_ALT, 'P');
    CHECK(parts.size() == 3 && parts[0] == L"Ctrl" && parts[1] == L"Alt" && parts[2] == L"P", L"FormatHotkeyParts Ctrl+Alt+P");

    // 3. AppData directory & INI escaping
    std::wstring appData = utils::GetAppDataDir();
    CHECK(appData.find(L"PanicButton") != std::wstring::npos, L"GetAppDataDir targets %LOCALAPPDATA%\\PanicButton\\");

    std::wstring sample = L"Línea 1: áéíóú ñ\r\nC:\\Ruta\\Archivo.xlsx";
    CHECK(utils::UnescapeIniValue(utils::EscapeIniValue(sample)) == sample, L"INI Escape/Unescape UTF-8 & CRLF");

    auto procs = utils::SplitLinesOrCommas(L"spotify.exe, OBS64.exe;\r\n discord.exe ");
    CHECK(procs.size() == 3 && procs[0] == L"spotify.exe" && procs[2] == L"discord.exe", L"SplitLinesOrCommas");

    // 4. Config Save/Load round-trip (including mouse trigger, audio pause, language)
    {
        wchar_t tmpDir[MAX_PATH]{};
        GetTempPathW(MAX_PATH, tmpDir);
        std::wstring cfgPath = std::wstring(tmpDir) + L"panic_test_cfg.ini";

        AppConfig c1;
        c1.panicModifiers = MOD_ALT;
        c1.panicVk = 'Q';
        c1.muteAudioOnPanic = true;
        c1.pauseMediaOnPanic = true;
        c1.mouseTrigger = MouseTrigger::XButton2;
        c1.language = 1; // English
        c1.stealthTray = true;
        c1.decoyType = DecoyType::ExternalUrl;
        c1.urlAddress = L"https://es.wikipedia.org/wiki/C++";
        c1.urlBrowserMode = UrlBrowserMode::AppWindow;
        c1.notesTitle = L"Resumen Español.txt";
        c1.notesContent = L"Línea 1\r\nLínea 2 con ñ y acentos: ó";
        c1.excludedProcesses = L"obs64.exe, spotify.exe";
        CHECK(c1.Save(cfgPath), L"Config Save");

        AppConfig c2;
        CHECK(c2.Load(cfgPath), L"Config Load");
        CHECK(c2.panicModifiers == MOD_ALT && c2.panicVk == 'Q', L"Config Hotkey roundtrip");
        CHECK(c2.muteAudioOnPanic == true && c2.pauseMediaOnPanic == true, L"Config audio mute & media pause roundtrip");
        CHECK(c2.mouseTrigger == MouseTrigger::XButton2, L"Config mouse trigger roundtrip");
        CHECK(c2.language == 1, L"Config language preference roundtrip");
        CHECK(c2.stealthTray == true, L"Config stealth flag roundtrip");
        CHECK(c2.decoyType == DecoyType::ExternalUrl && c2.urlBrowserMode == UrlBrowserMode::AppWindow, L"Config decoy roundtrip");
        CHECK(c2.urlAddress == c1.urlAddress, L"Config URL roundtrip");
        CHECK(c2.notesContent == c1.notesContent, L"Config multiline Unicode roundtrip");
        DeleteFileW(cfgPath.c_str());
    }

    // 5. WASAPI Audio Peak detection
    {
        AudioControl audio;
        float peak = audio.GetCurrentAudioPeak();
        CHECK(peak >= 0.0f && peak <= 1.0f, L"AudioControl: GetCurrentAudioPeak returns valid range [0.0, 1.0]");
    }

    // 4. Real cross-process Win32 window classification & hide/restore fidelity test
    {
        wchar_t tmpDir[MAX_PATH]{};
        GetTempPathW(MAX_PATH, tmpDir);
        std::wstring infoPath = std::wstring(tmpDir) + L"panic_test_hwnds.txt";
        std::wstring evBase = L"PanicTestEv_" + std::to_wstring(GetCurrentProcessId());

        HANDLE hReady = CreateEventW(nullptr, TRUE, FALSE, (evBase + L"_READY").c_str());
        HANDLE hQuit = CreateEventW(nullptr, TRUE, FALSE, (evBase + L"_QUIT").c_str());

        std::wstring cmd = L"\"" + utils::GetExePath() + L"\" --child \"" + infoPath + L"\" \"" + evBase + L"\"";
        STARTUPINFOW si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};
        BOOL launched = CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
        CHECK(launched != FALSE, L"Spawn dummy window child process");

        if (launched) {
            WaitForSingleObject(hReady, 5000);
            Sleep(80); // let DWM register windows

            HWND hNormal = nullptr, hMax = nullptr, hMin = nullptr, hHidden = nullptr;
            FILE* f = _wfopen(infoPath.c_str(), L"r");
            if (f) {
                unsigned long long a = 0, b = 0, c = 0, d = 0;
                if (fscanf(f, "%llu %llu %llu %llu", &a, &b, &c, &d) == 4) {
                    hNormal = (HWND)(uintptr_t)a;
                    hMax = (HWND)(uintptr_t)b;
                    hMin = (HWND)(uintptr_t)c;
                    hHidden = (HWND)(uintptr_t)d;
                }
                fclose(f);
            }
            DeleteFileW(infoPath.c_str());

            std::unordered_set<HWND> exclHwnd;
            std::unordered_set<std::wstring> exclProc;
            std::unordered_map<DWORD, std::wstring> cache;
            DWORD myPid = GetCurrentProcessId();

            CHECK(WindowManager::IsUserWindow(hNormal, myPid, exclHwnd, exclProc, cache), L"IsUserWindow: Normal window detected");
            CHECK(WindowManager::IsUserWindow(hMax, myPid, exclHwnd, exclProc, cache), L"IsUserWindow: Maximized window detected");
            CHECK(WindowManager::IsUserWindow(hMin, myPid, exclHwnd, exclProc, cache), L"IsUserWindow: Minimized window detected");
            CHECK(!WindowManager::IsUserWindow(hHidden, myPid, exclHwnd, exclProc, cache), L"IsUserWindow: Hidden window skipped");
            CHECK(!WindowManager::IsUserWindow(FindWindowW(L"Shell_TrayWnd", nullptr), myPid, exclHwnd, exclProc, cache),
                  L"IsUserWindow: Taskbar (Shell_TrayWnd) skipped");

            exclHwnd.insert(hNormal);
            CHECK(!WindowManager::IsUserWindow(hNormal, myPid, exclHwnd, exclProc, cache), L"IsUserWindow: Excluded HWND skipped");
            exclHwnd.clear();

            // Verify hide & state-preserving restore on ONLY the 3 dummy child windows
            RECT rectBefore{};
            GetWindowRect(hNormal, &rectBefore);

            ShowWindowAsync(hNormal, SW_HIDE);
            ShowWindowAsync(hMax, SW_HIDE);
            ShowWindowAsync(hMin, SW_HIDE);
            Sleep(60);

            CHECK(!IsWindowVisible(hNormal) && !IsWindowVisible(hMax) && !IsWindowVisible(hMin),
                  L"All 3 dummy windows hidden via ShowWindowAsync");

            ShowWindowAsync(hMin, SW_SHOWMINNOACTIVE);
            ShowWindowAsync(hMax, SW_SHOWNA);
            ShowWindowAsync(hNormal, SW_SHOWNA);
            Sleep(60);

            RECT rectAfter{};
            GetWindowRect(hNormal, &rectAfter);
            CHECK(IsWindowVisible(hNormal) && rectBefore.left == rectAfter.left && rectBefore.right == rectAfter.right,
                  L"Normal window restored with exact rect");
            CHECK(IsWindowVisible(hMax) && IsZoomed(hMax), L"Maximized window restored still maximized");
            CHECK(IsWindowVisible(hMin) && IsIconic(hMin), L"Minimized window restored still minimized");

            // Verify crash recovery state file works
            ShowWindowAsync(hNormal, SW_HIDE);
            Sleep(40);
            {
                std::wstring stPath = WindowManager::GetStateFilePath();
                FILE* sf = _wfopen(stPath.c_str(), L"w");
                if (sf) {
                    fprintf(sf, "PANIC_STATE_V1\n%llu 0 0\n", (unsigned long long)(uintptr_t)hNormal);
                    fclose(sf);
                }
                WindowManager wm;
                size_t rec = wm.RecoverOrphanedWindowsIfAny();
                Sleep(50);
                CHECK(rec == 1 && IsWindowVisible(hNormal), L"Crash recovery restored hidden window from panic_state.dat");
            }

            SetEvent(hQuit);
            WaitForSingleObject(pi.hProcess, 3000);
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }

        if (hReady) CloseHandle(hReady);
        if (hQuit) CloseHandle(hQuit);
    }

    // 5. BuiltinDecoyWindow prepare + icons generation test
    {
        HICON ic1 = utils::CreateAppIcon(32, false);
        HICON ic2 = utils::CreateDecoyIcon(32, utils::DecoyIconKind::Terminal);
        CHECK(ic1 != nullptr && ic2 != nullptr, L"GDI+ programmatic icons generated cleanly");
        if (ic1) DestroyIcon(ic1);
        if (ic2) DestroyIcon(ic2);

        AppConfig cfg;
        cfg.decoyType = DecoyType::BuiltinNotes;
        BuiltinDecoyWindow decoy;
        decoy.Prepare(GetModuleHandleW(nullptr), cfg);
        CHECK(decoy.GetHwnd() != nullptr && !decoy.IsVisible(), L"BuiltinDecoyWindow pre-created hidden in RAM");
        decoy.Destroy();
    }

    Gdiplus::GdiplusShutdown(gdiToken);

    if (g_failures == 0) {
        std::wcout << L"\n>>> TODAS LAS PRUEBAS PASARON EXITOSAMENTE <<<\n";
        return 0;
    } else {
        std::wcout << L"\n>>> FALLARON " << g_failures << L" PRUEBAS <<<\n";
        return 1;
    }
}
