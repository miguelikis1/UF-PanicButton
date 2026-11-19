# Panic Button for Windows 11 (C++20 / Pure Win32)

**English** | [Español](README.es.md)

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](#compilation-and-building)
[![Platform](https://img.shields.io/badge/platform-Windows%2011%20%7C%2010-0078D6.svg?logo=windows)](https://microsoft.com)
[![Language](https://img.shields.io/badge/language-C%2B%2B20-00599C.svg?logo=c%2B%2B)](https://en.cppreference.com/w/cpp/20)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![CPU Usage](https://img.shields.io/badge/idle%20CPU-0.00%25-success.svg)](#performance--architecture)
[![Dependencies](https://img.shields.io/badge/dependencies-none%20(pure%20Win32)-orange.svg)](#performance--architecture)

**Panic Button** is an ultra-fast, zero-overhead desktop hiding utility for Windows 11, engineered in modern **C++20** with the raw **Win32 API**.

With a single customizable hotkey or mouse side button:
1. **Instantly hides (< 10 ms)** all open user windows across all monitors, the taskbar, `Alt+Tab`, and Task View—without terminating processes or losing unsaved work.
2. **Intelligently pauses multimedia & mutes system volume**: samples real audio peaks via WASAPI (`IAudioMeterInformation`) and toggles `VK_MEDIA_PLAY_PAUSE` only if sound was playing.
3. **Instantly launches a camouflage decoy** (built-in work report, simulated terminal, spreadsheet, image, external app, URL app, or captured window).
4. **Restores everything perfectly** on the same hotkey, preserving exact Z-order, maximized/minimized window states, and foreground window focus.

---

## Demonstration

<img width="2541" height="1479" alt="Decoy Preview " src="https://github.com/user-attachments/assets/8ff6c10c-838b-43ef-a4f6-5cd1033500f6" />

<img width="2540" height="1478" alt="App preview" src="https://github.com/user-attachments/assets/ac43402a-0157-4352-8ca6-a7dfb80dab8d" />


---

## Core Technical Features

### 1. Windows 11 Deep Engine & Strict Filtering
- **Virtual Desktop & Cloak Filtering**: Queries `DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, ...)` to ignore hidden system cloaked windows or windows residing on other virtual desktops.
- **System Shell Window Exclusion**: Automatically filters `Shell_TrayWnd`, `Progman`, `WorkerW`, `Windows.UI.Core.CoreWindow`, and handles UWP `ApplicationFrameWindow` containers safely.
- **Asynchronous Parallel Hiding**: Uses `ShowWindowAsync(hwnd, SW_HIDE)` so that slow or unresponsive external applications never delay hiding other windows.

### 2. Exact Z-Order Reconstruction
- Windows are recorded from top to bottom during enumeration.
- Upon panic deactivation, all windows are repositioned in atomic order using Win32 `BeginDeferWindowPos`, `DeferWindowPos`, and `EndDeferWindowPos` with `SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE` before returning foreground focus to the originally active window.

### 3. Smart WASAPI Multimedia Detection
- Queries real-time hardware audio peaks via `IAudioMeterInformation::GetPeakValue()`.
- If audio was actively playing (`peak > 0.001f`), it sends a virtual `VK_MEDIA_PLAY_PAUSE` keystroke via `SendInput` and mutes master volume.
- When leaving panic mode, media playback is resumed **only if the panic button paused it**.

### 4. Zero-Hook Mouse Trigger (Raw Input)
- In addition to keyboard shortcuts, panic mode can be triggered with mouse side buttons (`XBUTTON1` or `XBUTTON2`).
- Uses `RegisterRawInputDevices` with `RIDEV_INPUTSINK` targeting a hidden message-only window (`HWND_MESSAGE`). This achieves global detection without invasive global mouse hooks (`WH_MOUSE_LL`), ensuring **0.00% CPU usage in idle**.

### 5. Robust Crash Recovery & Clean Storage
- Stores settings in `%LOCALAPPDATA%\PanicButton\config.ini`.
- While panic mode is active, an atomic recovery log (`panic_state.dat`) is maintained. If the process is terminated unexpectedly or system reboots, launching `PanicButton.exe --restore` automatically unhides all captured windows.

### 6. HiDPI & Modern Fluent Aesthetics
- Embedded manifest with `PerMonitorV2` DPI awareness and Common Controls v6.
- Dynamic font and control scaling based on `GetDpiForWindow`.
- Native immersive dark mode title bar via `DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, ...)`.

### 7. In-Memory Bilingual Internationalization (EN / ES)
- In-memory string translation table with zero external file dependencies.
- Auto-detects system UI language via `GetUserDefaultUILanguage()` (`LANG_SPANISH` -> Spanish, all others -> English).
- Manual override available in the Settings dialog.

---

## 8 Built-in Camouflage Decoy Modes

| # | Decoy Mode | Description |
|---|---|---|
| **1** | **Work Notes / Report** | Native Win11 document editor with editable templates (Quarterly Report, Meeting Minutes, Project Plan). |
| **2** | **Terminal / Console** | Dark PowerShell-style terminal with simulated compilation activity (C++/CMake, Docker, Security Audit). |
| **3** | **Spreadsheet / Metrics** | Interactive data grid with selectable cells and formula bar (Quarterly KPIs, Financial Budget). |
| **4** | **Custom Screenshot / Image** | Pre-renders any `.png`, `.jpg`, or `.bmp` at fullscreen or maximized in `< 1 ms`. |
| **5** | **External App or File** | Launches any executable (`.exe`) or document (`.docx`, `.xlsx`, `.pdf`) with optional hidden pre-warming. |
| **6** | **Web Page / Independent URL** | Opens target URL in standalone application window mode (`--app`) with optional pre-warming. |
| **7** | **Capture Running Window** | Selects an existing opened window (e.g. IDE, Excel, Email) to serve as the decoy. |
| **8** | **Clean Desktop** | Hides all windows, leaving a pristine clean desktop view. |

---

## CLI Options

| Argument | Description |
|---|---|
| *(none)* | Launches application into tray icon (or shows settings UI if first run). |
| `--minimized` | Starts directly minimized to system tray (used for Windows autostart). |
| `--settings` | Forces opening the Settings & Configuration window directly. |
| `--restore` | Emergency command: immediately restores all hidden windows and exits. |

---

## Compilation and Building

### Prerequisites
- **MinGW-w64** with GCC 13+ (supporting `C++20`) or **MSVC 2022+**
- Windows SDK 10.0+ / MinGW Win32 headers

### Quick Build (MinGW-w64)
Run the automated build script:
```bat
build.bat
```

### CMake Build
```cmd
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Automated CI/CD
Every push to `main` builds and tests the executable on GitHub Actions. Pushing a version tag (`v*`) automatically creates a GitHub Release with `PanicButton.exe` and its SHA-256 checksum.

---

## License

This project is licensed under the terms of the [MIT License](LICENSE).
