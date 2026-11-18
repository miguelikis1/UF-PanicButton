#pragma once

#include "utils.h"
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

struct SavedWindowState {
    HWND hwnd = nullptr;
    bool wasMinimized = false;
    bool wasMaximized = false;
    bool wasForeground = false;
};

struct VisibleWindowInfo {
    HWND hwnd = nullptr;
    std::wstring title;
    std::wstring processName;
    bool isMinimized = false;
};

struct PanicStats {
    size_t windowCount = 0;
    double elapsedMs = 0.0;
};

class WindowManager {
public:
    WindowManager() = default;

    // Hide all visible user windows (excluding our own process, excludedHwnds, and excludedProcesses).
    // Returns number of hidden windows and timing.
    PanicStats HideAllWindows(
        const std::unordered_set<HWND>& excludedHwnds,
        const std::vector<std::wstring>& excludedProcesses);

    // Restore all previously hidden windows in exact Z-order, state, and foreground focus.
    PanicStats RestoreAllWindows();

    // Enumerate user windows for the "Capture existing window" picker.
    static std::vector<VisibleWindowInfo> EnumerateUserWindows(
        const std::unordered_set<HWND>& excludedHwnds = {});

    // Check if an HWND is a valid target user window (not shell/cloaked/our own).
    static bool IsUserWindow(
        HWND hwnd,
        DWORD ownPid,
        const std::unordered_set<HWND>& excludedHwnds,
        const std::unordered_set<std::wstring>& excludedProcs,
        std::unordered_map<DWORD, std::wstring>& pidNameCache);

    // Crash recovery: checks for saved panic_state.dat from an unexpected termination and restores windows.
    size_t RecoverOrphanedWindowsIfAny();

    bool HasHiddenWindows() const { return !hiddenWindows_.empty(); }
    size_t HiddenCount() const { return hiddenWindows_.size(); }
    PanicStats LastHideStats() const { return lastHideStats_; }
    PanicStats LastRestoreStats() const { return lastRestoreStats_; }

    static std::wstring GetStateFilePath();

private:
    void SaveStateToDisk() const;
    void ClearStateOnDisk() const;

    std::vector<SavedWindowState> hiddenWindows_;
    HWND savedForeground_ = nullptr;
    PanicStats lastHideStats_{};
    PanicStats lastRestoreStats_{};
};
