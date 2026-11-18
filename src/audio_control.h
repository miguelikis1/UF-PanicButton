#pragma once

#include "utils.h"

class AudioControl {
public:
    AudioControl() = default;
    ~AudioControl() = default;

    // Activates panic audio handling:
    // 1. If pauseMedia is true and active audio is playing (peak > 0.001f),
    //    simulates VK_MEDIA_PLAY_PAUSE key to pause YouTube/Spotify/media.
    // 2. If muteMaster is true, mutes master endpoint via WASAPI.
    void OnPanic(bool muteMaster, bool pauseMedia);

    // Restores audio state upon leaving panic mode:
    // 1. Unmutes master volume ONLY if we were the ones who muted it.
    // 2. Unpauses media ONLY if we were the ones who sent the pause key.
    void OnRestore();

    // Returns the current master audio peak level (0.0f to 1.0f) using IAudioMeterInformation
    float GetCurrentAudioPeak();

    bool DidWeMute() const { return didWeMute_; }
    bool DidWePauseMedia() const { return didWePauseMedia_; }

private:
    bool didWeMute_ = false;
    bool didWePauseMedia_ = false;
};
