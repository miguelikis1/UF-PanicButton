#include "audio_control.h"

#include <objbase.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

#ifndef __IAudioMeterInformation_INTERFACE_DEFINED__
#define __IAudioMeterInformation_INTERFACE_DEFINED__
MIDL_INTERFACE("C02216F6-8C67-4B5B-9D00-D008E73E0064")
IAudioMeterInformation : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetPeakValue(float *pfPeak) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMeteringChannelCount(UINT *pnChannelCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetChannelsPeakValues(UINT u32ChannelCount, float *afPeakValues) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryHardwareSupport(DWORD *pdwHardwareSupportMask) = 0;
};
#endif

namespace {

// Explicit COM GUIDs to avoid toolchain/linker UUID discrepancies
const CLSID kClsidMMDeviceEnumerator = {
    0xBCDE0395, 0xE52F, 0x467C, {0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E}
};
const IID kIidIMMDeviceEnumerator = {
    0xA95664D2, 0x9614, 0x4F35, {0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6}
};
const IID kIidIAudioEndpointVolume = {
    0x5CDF2C82, 0x841E, 0x4546, {0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A}
};
const IID kIidIAudioMeterInformation = {
    0xC02216F6, 0x8C67, 0x4B5B, {0x9D, 0x00, 0xD0, 0x08, 0xE7, 0x3E, 0x00, 0x64}
};

IMMDevice* GetDefaultRenderDevice() {
    IMMDeviceEnumerator* enumerator = nullptr;
    HRESULT hr = CoCreateInstance(
        kClsidMMDeviceEnumerator, nullptr, CLSCTX_ALL,
        kIidIMMDeviceEnumerator, reinterpret_cast<void**>(&enumerator));
    if (FAILED(hr) || !enumerator) return nullptr;

    IMMDevice* device = nullptr;
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    enumerator->Release();
    if (FAILED(hr)) return nullptr;
    return device;
}

void SendMediaPlayPauseKey() {
    INPUT inputs[2]{};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_MEDIA_PLAY_PAUSE;
    inputs[0].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_MEDIA_PLAY_PAUSE;
    inputs[1].ki.dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

} // namespace

float AudioControl::GetCurrentAudioPeak() {
    IMMDevice* device = GetDefaultRenderDevice();
    if (!device) return 0.0f;

    IAudioMeterInformation* meter = nullptr;
    HRESULT hr = device->Activate(kIidIAudioMeterInformation, CLSCTX_ALL, nullptr,
                                  reinterpret_cast<void**>(&meter));
    device->Release();
    if (FAILED(hr) || !meter) return 0.0f;

    float peak = 0.0f;
    meter->GetPeakValue(&peak);
    meter->Release();
    return peak;
}

void AudioControl::OnPanic(bool muteMaster, bool pauseMedia) {
    didWeMute_ = false;
    didWePauseMedia_ = false;

    // 1. Intelligent multimedia pause via WASAPI peak check
    if (pauseMedia) {
        float peak = GetCurrentAudioPeak();
        if (peak > 0.001f) {
            SendMediaPlayPauseKey();
            didWePauseMedia_ = true;
        }
    }

    // 2. Instant master mute via IAudioEndpointVolume
    if (muteMaster) {
        IMMDevice* device = GetDefaultRenderDevice();
        if (device) {
            IAudioEndpointVolume* vol = nullptr;
            HRESULT hr = device->Activate(kIidIAudioEndpointVolume, CLSCTX_ALL, nullptr,
                                          reinterpret_cast<void**>(&vol));
            device->Release();
            if (SUCCEEDED(hr) && vol) {
                BOOL alreadyMuted = FALSE;
                if (SUCCEEDED(vol->GetMute(&alreadyMuted)) && !alreadyMuted) {
                    if (SUCCEEDED(vol->SetMute(TRUE, nullptr))) {
                        didWeMute_ = true;
                    }
                }
                vol->Release();
            }
        }
    }
}

void AudioControl::OnRestore() {
    // 1. Restore unmuted state if we were the ones who muted it
    if (didWeMute_) {
        IMMDevice* device = GetDefaultRenderDevice();
        if (device) {
            IAudioEndpointVolume* vol = nullptr;
            HRESULT hr = device->Activate(kIidIAudioEndpointVolume, CLSCTX_ALL, nullptr,
                                          reinterpret_cast<void**>(&vol));
            device->Release();
            if (SUCCEEDED(hr) && vol) {
                vol->SetMute(FALSE, nullptr);
                vol->Release();
            }
        }
        didWeMute_ = false;
    }

    // 2. Resume multimedia playback if we paused it
    if (didWePauseMedia_) {
        SendMediaPlayPauseKey();
        didWePauseMedia_ = false;
    }
}
