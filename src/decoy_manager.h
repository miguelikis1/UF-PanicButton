#pragma once

#include "config.h"
#include "builtin_decoys.h"
#include <unordered_set>
#include <string>

class DecoyManager {
public:
    DecoyManager() = default;
    ~DecoyManager();

    // Prepare / pre-load decoy based on current config (non-blocking for external apps)
    void Prepare(HWND ownerHwnd, HINSTANCE hInst, const AppConfig& cfg);

    // Must be called when the prewarm timer ticks on ownerHwnd. Returns true if status changed.
    bool OnPrewarmTimerTick(HWND ownerHwnd);

    // Activate decoy instantaneously on Panic!
    void Activate(HWND ownerHwnd, HINSTANCE hInst, const AppConfig& cfg);

    // Deactivate decoy on Restore (un-panic)
    void Deactivate(const AppConfig& cfg);

    // Capture an existing user window as the decoy
    void SetCapturedWindow(HWND hwnd, bool hideIntoStandby);
    void ReleaseCapturedWindow(bool restoreIfHidden);
    HWND GetCapturedWindow() const;
    std::wstring GetCapturedWindowTitle() const;

    // Close / clean up any pre-warmed external decoy we spawned
    void ReleasePrewarmedExternal(bool closeWindow);

    // Get any HWND that WindowManager should exclude from hiding during Panic
    void CollectExcludedHwnds(std::unordered_set<HWND>& out) const;

    // Status string for UI
    std::wstring GetStatusText(const AppConfig& cfg) const;
    bool IsPrewarmedReady(const AppConfig& cfg) const;

    // Clean up on app exit (restores captured window if hidden, closes spawned pre-warmed window)
    void Shutdown();

    static constexpr UINT_PTR kPrewarmTimerId = 4201;

private:
    bool LaunchExternalTarget(const AppConfig& cfg, bool forPrewarm, HANDLE* outProcess = nullptr);
    void StartWatchingNewWindow(HWND ownerHwnd, DWORD launchedPid, bool hideWhenFound);
    void StopWatchingNewWindow(HWND ownerHwnd);

    BuiltinDecoyWindow builtin_;

    // Pre-warmed or active external decoy window we launched
    HWND externalHwnd_ = nullptr;
    DWORD externalPid_ = 0;

    // User-captured existing window
    HWND capturedHwnd_ = nullptr;
    bool capturedWasHiddenByUs_ = false;

    // Async window capture state after launching an external target
    bool watchingNewWindow_ = false;
    bool watchHideWhenFound_ = false;
    DWORD watchLaunchedPid_ = 0;
    int watchTicksRemaining_ = 0;
    std::unordered_set<HWND> preLaunchSnapshot_;
};
