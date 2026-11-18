cmake_minimum_required(VERSION 3.20)
project(PanicButton LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/utils.cpp
    src/config.cpp
    src/i18n.cpp
    src/audio_control.cpp
    src/window_manager.cpp
    src/builtin_decoys.cpp
    src/decoy_manager.cpp
    src/tray_icon.cpp
    src/settings_ui.cpp
    resources/app.rc
)

add_executable(PanicButton WIN32 ${SOURCES})
target_compile_definitions(PanicButton PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX _WIN32_WINNT=0x0A00)

if(MINGW)
    target_compile_options(PanicButton PRIVATE -O3 -flto=auto -Wall -Wextra -municode)
    target_link_options(PanicButton PRIVATE -municode -mwindows -s -static -flto=auto)
    target_link_libraries(PanicButton PRIVATE
        user32 gdi32 shell32 ole32 oleaut32 uuid comctl32 comdlg32 advapi32 dwmapi uxtheme gdiplus psapi
    )
elseif(MSVC)
    target_compile_options(PanicButton PRIVATE /O2 /W4 /utf-8)
    target_link_libraries(PanicButton PRIVATE
        user32 gdi32 shell32 ole32 oleaut32 comctl32 comdlg32 advapi32 dwmapi uxtheme gdiplus psapi
    )
endif()
