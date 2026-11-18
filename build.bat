@echo off
chcp 65001 >nul
setlocal

echo [1/3] Compilando recursos (manifiesto Windows 11 + DPI PerMonitorV2)...
if not exist build mkdir build
windres resources\app.rc -O coff -o build\app.res
if errorlevel 1 (
    echo ERROR al compilar recursos.
    exit /b 1
)

echo [2/3] Compilando PanicButton.exe (C++20 -O3 -flto nativo Win32)...
if exist PanicButton.old.exe del /f /q PanicButton.old.exe >nul 2>&1
if exist PanicButton.exe move /y PanicButton.exe PanicButton.old.exe >nul 2>&1

g++ -std=c++20 -O3 -flto=auto -DNDEBUG -Wall -Wextra -Wno-unused-parameter ^
    -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_WIN32_WINNT=0x0A00 ^
    -municode -mwindows -s -static ^
    src\main.cpp ^
    src\utils.cpp ^
    src\config.cpp ^
    src\i18n.cpp ^
    src\audio_control.cpp ^
    src\window_manager.cpp ^
    src\builtin_decoys.cpp ^
    src\decoy_manager.cpp ^
    src\tray_icon.cpp ^
    src\settings_ui.cpp ^
    build\app.res ^
    -o PanicButton.exe ^
    -luser32 -lgdi32 -lshell32 -lole32 -loleaut32 -luuid -lcomctl32 -lcomdlg32 -ladvapi32 -ldwmapi -luxtheme -lgdiplus -lpsapi

if errorlevel 1 (
    echo ERROR al compilar el ejecutable.
    exit /b 1
)

echo [3/3] Compilacion completada con exito: PanicButton.exe
endlocal
