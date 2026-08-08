// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "DualSenseCoreTypes.h"
#include <functional>
#include <memory>
#include <vector>

namespace DualSense
{
    class DUALSENSECORE_API Manager final
    {
    public:
        using ConnectionCallback = std::function<void(const DeviceInfo&)>;
        using DisconnectionCallback = std::function<void(DeviceId)>;

        Manager();
        ~Manager();
        Manager(const Manager&) = delete;
        Manager& operator=(const Manager&) = delete;

        bool Start();
        void Stop();
        void Tick(float DeltaSeconds);
        void RequestImmediateDetection();

        [[nodiscard]] bool IsRunning() const;
        [[nodiscard]] bool IsConnected(DeviceId Id) const;
        [[nodiscard]] std::vector<DeviceId> GetConnectedDeviceIds() const;
        [[nodiscard]] bool GetDeviceInfo(DeviceId Id, DeviceInfo& OutInfo) const;
        [[nodiscard]] bool GetState(DeviceId Id, State& OutState) const;
        [[nodiscard]] Capabilities GetCapabilities(DeviceId Id) const;

        void SetConnectionCallback(ConnectionCallback Callback);
        void SetDisconnectionCallback(DisconnectionCallback Callback);

        bool ApplyOutput(DeviceId Id);
        bool SetVibration(DeviceId Id, std::uint8_t Left, std::uint8_t Right, bool ApplyImmediately = true);
        bool SetLightbar(DeviceId Id, Color InColor, bool ApplyImmediately = true);
        bool SetLightbarFlash(DeviceId Id, Color InColor, float BrightnessTime, float ToggleTime, bool ApplyImmediately = true);
        bool SetPlayerLed(DeviceId Id, PlayerLed Led, std::uint8_t Brightness, bool ApplyImmediately = true);
        bool SetMicrophoneLed(DeviceId Id, MicrophoneLed Led, bool ApplyImmediately = true);
        bool ResetLights(DeviceId Id, bool ApplyImmediately = true);

        bool StopTrigger(DeviceId Id, Hand InHand, bool ApplyImmediately = true);
        bool SetGameCubeTrigger(DeviceId Id, Hand InHand, bool ApplyImmediately = true);
        bool SetResistanceTrigger(DeviceId Id, std::uint8_t StartZone, std::uint8_t Strength, Hand InHand, bool ApplyImmediately = true);
        bool SetBowTrigger(DeviceId Id, std::uint8_t StartZone, std::uint8_t SnapBack, Hand InHand, bool ApplyImmediately = true);
        bool SetGallopingTrigger(DeviceId Id, std::uint8_t StartPosition, std::uint8_t EndPosition, std::uint8_t FirstFoot, std::uint8_t SecondFoot, std::uint8_t Frequency, Hand InHand, bool ApplyImmediately = true);
        bool SetWeaponTrigger(DeviceId Id, std::uint8_t StartZone, std::uint8_t Amplitude, std::uint8_t Behavior, std::uint8_t Trigger, Hand InHand, bool ApplyImmediately = true);
        bool SetMachineGunTrigger(DeviceId Id, std::uint8_t StartZone, std::uint8_t Behavior, std::uint8_t Amplitude, std::uint8_t Frequency, Hand InHand, bool ApplyImmediately = true);
        bool SetMachineTrigger(DeviceId Id, std::uint8_t StartZone, std::uint8_t BehaviorFlag, std::uint8_t Force, std::uint8_t Amplitude, std::uint8_t Period, std::uint8_t Frequency, Hand InHand, bool ApplyImmediately = true);
        bool SetCustomTrigger(DeviceId Id, Hand InHand, const std::array<std::uint8_t, 10>& Bytes, bool ApplyImmediately = true);

        bool EnableMotion(DeviceId Id, bool Enabled);
        bool ResetGyroOrientation(DeviceId Id);
        bool EnableTouch(DeviceId Id, bool Enabled);
        bool EnableGesture(DeviceId Id, bool Enabled);

        bool ConfigureDualSense(DeviceId Id, bool MicEnabled, bool HeadsetEnabled, bool SpeakerEnabled, std::uint8_t MicVolume, std::uint8_t AudioVolume, std::uint8_t RumbleMode, std::uint8_t RumbleReduce, std::uint8_t TriggerReduce, bool ApplyImmediately = true);
        bool SendAudioHaptics(DeviceId Id, const std::vector<std::uint8_t>& AudioData);
        bool SendAudioHaptics(DeviceId Id, const std::vector<float>& AudioData);
        bool SendAudioHaptics(DeviceId Id, const std::vector<std::uint8_t>& HapticsData, const std::vector<std::uint8_t>& AudioData);

    private:
        class Impl;
        std::unique_ptr<Impl> Pimpl;
    };
}
