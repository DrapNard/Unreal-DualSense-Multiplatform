// SPDX-License-Identifier: MPL-2.0
#include "DualSenseCoreManager.h"
#include "Platform/DualSensePlatformHardware.h"

#include "GCore/Interfaces/IPlatformHardware.h"
#include "GCore/Templates/TBasicDeviceRegistry.h"
#include "GCore/Types/Structs/Context/DeviceContext.h"
#include "GImplementations/Libraries/Base/GamepadBase.h"

#include <algorithm>
#include <mutex>
#include <unordered_set>

namespace
{
    using namespace DualSense;

    DeviceType ToDeviceType(EDSDeviceType Type)
    {
        switch (Type)
        {
            case EDSDeviceType::DualSense: return DeviceType::DualSense;
            case EDSDeviceType::DualSenseEdge: return DeviceType::DualSenseEdge;
            case EDSDeviceType::DualShock4: return DeviceType::DualShock4;
            default: return DeviceType::Unknown;
        }
    }

    ConnectionType ToConnectionType(EDSDeviceConnection Type)
    {
        switch (Type)
        {
            case EDSDeviceConnection::Usb: return ConnectionType::USB;
            case EDSDeviceConnection::Bluetooth: return ConnectionType::Bluetooth;
            default: return ConnectionType::Unknown;
        }
    }

    EDSGamepadHand ToHand(Hand InHand)
    {
        switch (InHand)
        {
            case Hand::Left: return EDSGamepadHand::Left;
            case Hand::Right: return EDSGamepadHand::Right;
            default: return EDSGamepadHand::AnyHand;
        }
    }

    EDSPlayer ToPlayerLed(PlayerLed Led)
    {
        switch (Led)
        {
            case PlayerLed::One: return EDSPlayer::One;
            case PlayerLed::Two: return EDSPlayer::Two;
            case PlayerLed::Three: return EDSPlayer::Three;
            case PlayerLed::All: return EDSPlayer::All;
            default: return EDSPlayer::Off;
        }
    }

    EDSMic ToMicrophoneLed(MicrophoneLed Led)
    {
        switch (Led)
        {
            case MicrophoneLed::On: return EDSMic::MicOn;
            case MicrophoneLed::Pulse: return EDSMic::Pulse;
            default: return EDSMic::MicOff;
        }
    }

    State ToState(const FInputContext& Input)
    {
        State Out;
        Out.LeftStick = {Input.LeftAnalog.X, Input.LeftAnalog.Y};
        Out.RightStick = {Input.RightAnalog.X, Input.RightAnalog.Y};
        Out.LeftTrigger = Input.LeftTriggerAnalog;
        Out.RightTrigger = Input.RightTriggerAnalog;
        Out.Gyroscope = {Input.Gyroscope.X, Input.Gyroscope.Y, Input.Gyroscope.Z};
        Out.Accelerometer = {Input.Accelerometer.X, Input.Accelerometer.Y, Input.Accelerometer.Z};
        Out.Gravity = {Input.Gravity.X, Input.Gravity.Y, Input.Gravity.Z};
        Out.Tilt = {Input.Tilt.X, Input.Tilt.Y, Input.Tilt.Z};
        Out.TouchId = Input.TouchId;
        Out.TouchFingerCount = Input.TouchFingerCount;
        Out.TouchDirectionRaw = Input.DirectionRaw;
        Out.IsTouching = Input.bIsTouching;
        Out.TouchRadius = {Input.TouchRadius.X, Input.TouchRadius.Y};
        Out.TouchPosition = {Input.TouchPosition.X, Input.TouchPosition.Y};
        Out.TouchRelative = {Input.TouchRelative.X, Input.TouchRelative.Y};
        Out.BatteryLevel = Input.BatteryLevel;
        Out.HasPhoneConnected = Input.bHasPhoneConnected;

        auto Set = [&Out](Button B, bool Value)
        {
            Out.Buttons[static_cast<std::size_t>(B)] = Value;
        };
        Set(Button::Cross, Input.bCross);
        Set(Button::Square, Input.bSquare);
        Set(Button::Triangle, Input.bTriangle);
        Set(Button::Circle, Input.bCircle);
        Set(Button::DPadUp, Input.bDpadUp);
        Set(Button::DPadDown, Input.bDpadDown);
        Set(Button::DPadLeft, Input.bDpadLeft);
        Set(Button::DPadRight, Input.bDpadRight);
        Set(Button::L1, Input.bLeftShoulder);
        Set(Button::R1, Input.bRightShoulder);
        Set(Button::L3, Input.bLeftStick);
        Set(Button::R3, Input.bRightStick);
        Set(Button::PS, Input.bPSButton);
        Set(Button::Share, Input.bShare);
        Set(Button::Options, Input.bStart);
        Set(Button::Touchpad, Input.bTouch);
        Set(Button::Mute, Input.bMute);
        Set(Button::FnLeft, Input.bFn1);
        Set(Button::FnRight, Input.bFn2);
        Set(Button::PaddleLeft, Input.bPaddleLeft);
        Set(Button::PaddleRight, Input.bPaddleRight);
        return Out;
    }
}

namespace DualSense
{
    class Manager::Impl
    {
    public:
        struct RegistryPolicy
        {
            using EngineIdType = DeviceId;
            struct Hasher { std::size_t operator()(DeviceId Id) const noexcept { return static_cast<std::size_t>(Id); } };

            Impl* Owner = nullptr;
            DeviceId NextId = 0;

            DeviceId AllocEngineDevice() { return NextId++; }
            void DisconnectDevice(DeviceId Id) { if (Owner) Owner->OnDisconnected(Id); }
            void DispatchNewGamepad(DeviceId Id) { if (Owner) Owner->OnConnected(Id); }
        };

        GamepadCore::TBasicDeviceRegistry<RegistryPolicy> Registry;
        bool Running = false;
        ConnectionCallback ConnectedCallback;
        DisconnectionCallback DisconnectedCallback;
        std::unordered_set<DeviceId> ConnectedIds;
        mutable std::mutex Mutex;

        Impl()
        {
            Registry.Policy.Owner = this;
        }

        IGamepadBase* Get(DeviceId Id)
        {
            return Registry.GetLibrary(Id);
        }

        const IGamepadBase* Get(DeviceId Id) const
        {
            return const_cast<Impl*>(this)->Registry.GetLibrary(Id);
        }

        void OnConnected(DeviceId Id)
        {
            DeviceInfo Info;
            {
                std::lock_guard<std::mutex> Lock(Mutex);
                ConnectedIds.insert(Id);
            }
            if (GetInfo(Id, Info) && ConnectedCallback)
            {
                ConnectedCallback(Info);
            }
        }

        void OnDisconnected(DeviceId Id)
        {
            {
                std::lock_guard<std::mutex> Lock(Mutex);
                ConnectedIds.erase(Id);
            }
            if (DisconnectedCallback)
            {
                DisconnectedCallback(Id);
            }
        }

        bool GetInfo(DeviceId Id, DeviceInfo& OutInfo) const
        {
            const IGamepadBase* Pad = Get(Id);
            if (!Pad)
            {
                return false;
            }
            FDeviceContext* Context = const_cast<IGamepadBase*>(Pad)->GetMutableDeviceContext();
            if (!Context)
            {
                return false;
            }
            OutInfo.Id = Id;
            OutInfo.Type = ToDeviceType(Context->DeviceType);
            OutInfo.Connection = ToConnectionType(Context->ConnectionType);
            OutInfo.Path = Context->Path;
            return true;
        }

        bool Flush(IGamepadBase* Pad, bool ApplyImmediately)
        {
            if (!Pad)
            {
                return false;
            }
            if (ApplyImmediately)
            {
                Pad->UpdateOutput();
            }
            return true;
        }
    };

    Manager::Manager() : Pimpl(std::make_unique<Impl>()) {}
    Manager::~Manager() { Stop(); }

    bool Manager::Start()
    {
        if (Pimpl->Running)
        {
            return true;
        }
        std::unique_ptr<IPlatformHardware> Hardware = Platform::CreateHardware();
        if (!Hardware)
        {
            return false;
        }
        IPlatformHardware::SetInstance(std::move(Hardware));
        Pimpl->Running = true;
        Pimpl->Registry.RequestImmediateDetection();
        return true;
    }

    void Manager::Stop()
    {
        if (!Pimpl || !Pimpl->Running)
        {
            return;
        }
        const std::vector<DeviceId> Ids = GetConnectedDeviceIds();
        for (DeviceId Id : Ids)
        {
            Pimpl->Registry.RemoveLibraryInstance(Id);
        }
        Pimpl->Running = false;
    }

    void Manager::Tick(float DeltaSeconds)
    {
        if (!Pimpl->Running)
        {
            return;
        }
        Pimpl->Registry.PlugAndPlay(DeltaSeconds);
        const std::vector<DeviceId> Ids = GetConnectedDeviceIds();
        for (DeviceId Id : Ids)
        {
            if (IGamepadBase* Pad = Pimpl->Get(Id))
            {
                Pad->UpdateInput(DeltaSeconds);
            }
        }
    }

    void Manager::RequestImmediateDetection() { Pimpl->Registry.RequestImmediateDetection(); }
    bool Manager::IsRunning() const { return Pimpl->Running; }

    bool Manager::IsConnected(DeviceId Id) const
    {
        const IGamepadBase* Pad = Pimpl->Get(Id);
        return Pad && const_cast<IGamepadBase*>(Pad)->IsConnected();
    }

    std::vector<DeviceId> Manager::GetConnectedDeviceIds() const
    {
        std::lock_guard<std::mutex> Lock(Pimpl->Mutex);
        std::vector<DeviceId> Result(Pimpl->ConnectedIds.begin(), Pimpl->ConnectedIds.end());
        std::sort(Result.begin(), Result.end());
        return Result;
    }

    bool Manager::GetDeviceInfo(DeviceId Id, DeviceInfo& OutInfo) const { return Pimpl->GetInfo(Id, OutInfo); }

    bool Manager::GetState(DeviceId Id, State& OutState) const
    {
        const IGamepadBase* Pad = Pimpl->Get(Id);
        if (!Pad)
        {
            return false;
        }
        FDeviceContext* Context = const_cast<IGamepadBase*>(Pad)->GetMutableDeviceContext();
        if (!Context)
        {
            return false;
        }
        FInputContext* Input = Context->GetInputState();
        if (!Input)
        {
            return false;
        }
        OutState = ToState(*Input);
        return true;
    }

    Capabilities Manager::GetCapabilities(DeviceId Id) const
    {
        Capabilities Out;
        const IGamepadBase* ConstPad = Pimpl->Get(Id);
        IGamepadBase* Pad = const_cast<IGamepadBase*>(ConstPad);
        if (!Pad) return Out;
        Out.Lightbar = Pad->GetIGamepadLightbar() != nullptr;
        Out.PlayerLed = Pad->GetDeviceType() == EDSDeviceType::DualSense || Pad->GetDeviceType() == EDSDeviceType::DualSenseEdge;
        Out.MicrophoneLed = Out.PlayerLed;
        Out.Rumble = Pad->GetIGamepadRumbles() != nullptr;
        Out.AdaptiveTriggers = Pad->GetIGamepadTrigger() != nullptr;
        Out.Motion = Pad->GetIGamepadSensors() != nullptr;
        Out.Touch = Pad->GetIGamepadTouch() != nullptr;
        Out.AudioHaptics = Pad->GetIGamepadHaptics() != nullptr;
        Out.AudioSettings = Pad->GetIGamepadSettings() != nullptr;
        return Out;
    }

    void Manager::SetConnectionCallback(ConnectionCallback Callback) { Pimpl->ConnectedCallback = std::move(Callback); }
    void Manager::SetDisconnectionCallback(DisconnectionCallback Callback) { Pimpl->DisconnectedCallback = std::move(Callback); }

    bool Manager::ApplyOutput(DeviceId Id)
    {
        IGamepadBase* Pad = Pimpl->Get(Id);
        if (!Pad) return false;
        Pad->UpdateOutput();
        return true;
    }

    bool Manager::SetVibration(DeviceId Id, std::uint8_t Left, std::uint8_t Right, bool ApplyImmediately)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadRumbles* Api = Pad->GetIGamepadRumbles(); if (!Api) return false;
        Api->SetVibration(Left, Right); return Pimpl->Flush(Pad, ApplyImmediately);
    }

    bool Manager::SetLightbar(DeviceId Id, Color C, bool ApplyImmediately)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadLightbar* Api = Pad->GetIGamepadLightbar(); if (!Api) return false;
        Api->SetLightbar({C.R, C.G, C.B, 255}); return Pimpl->Flush(Pad, ApplyImmediately);
    }

    bool Manager::SetLightbarFlash(DeviceId Id, Color C, float BrightnessTime, float ToggleTime, bool ApplyImmediately)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadLightbar* Api = Pad->GetIGamepadLightbar(); if (!Api) return false;
        Api->SetLightbarFlash({C.R, C.G, C.B, 255}, BrightnessTime, ToggleTime); return Pimpl->Flush(Pad, ApplyImmediately);
    }

    bool Manager::SetPlayerLed(DeviceId Id, PlayerLed Led, std::uint8_t Brightness, bool ApplyImmediately)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadLightbar* Api = Pad->GetIGamepadLightbar(); if (!Api) return false;
        Api->SetPlayerLed(ToPlayerLed(Led), Brightness); return Pimpl->Flush(Pad, ApplyImmediately);
    }

    bool Manager::SetMicrophoneLed(DeviceId Id, MicrophoneLed Led, bool ApplyImmediately)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadLightbar* Api = Pad->GetIGamepadLightbar(); if (!Api) return false;
        Api->SetMicrophoneLed(ToMicrophoneLed(Led)); return Pimpl->Flush(Pad, ApplyImmediately);
    }

    bool Manager::ResetLights(DeviceId Id, bool ApplyImmediately)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadLightbar* Api = Pad->GetIGamepadLightbar(); if (!Api) return false;
        Api->ResetLights(); return Pimpl->Flush(Pad, ApplyImmediately);
    }

#define DS_TRIGGER_CALL(Call) \
    IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false; \
    IGamepadTrigger* Api = Pad->GetIGamepadTrigger(); if (!Api) return false; \
    Api->Call; return Pimpl->Flush(Pad, ApplyImmediately)

    bool Manager::StopTrigger(DeviceId Id, Hand InHand, bool ApplyImmediately) { DS_TRIGGER_CALL(StopTrigger(ToHand(InHand))); }
    bool Manager::SetGameCubeTrigger(DeviceId Id, Hand InHand, bool ApplyImmediately) { DS_TRIGGER_CALL(SetGameCube(ToHand(InHand))); }
    bool Manager::SetResistanceTrigger(DeviceId Id, std::uint8_t StartZone, std::uint8_t Strength, Hand InHand, bool ApplyImmediately) { DS_TRIGGER_CALL(SetResistance(StartZone, Strength, ToHand(InHand))); }
    bool Manager::SetBowTrigger(DeviceId Id, std::uint8_t StartZone, std::uint8_t SnapBack, Hand InHand, bool ApplyImmediately) { DS_TRIGGER_CALL(SetBow22(StartZone, SnapBack, ToHand(InHand))); }
    bool Manager::SetGallopingTrigger(DeviceId Id, std::uint8_t StartPosition, std::uint8_t EndPosition, std::uint8_t FirstFoot, std::uint8_t SecondFoot, std::uint8_t Frequency, Hand InHand, bool ApplyImmediately) { DS_TRIGGER_CALL(SetGalloping23(StartPosition, EndPosition, FirstFoot, SecondFoot, Frequency, ToHand(InHand))); }
    bool Manager::SetWeaponTrigger(DeviceId Id, std::uint8_t StartZone, std::uint8_t Amplitude, std::uint8_t Behavior, std::uint8_t Trigger, Hand InHand, bool ApplyImmediately) { DS_TRIGGER_CALL(SetWeapon25(StartZone, Amplitude, Behavior, Trigger, ToHand(InHand))); }
    bool Manager::SetMachineGunTrigger(DeviceId Id, std::uint8_t StartZone, std::uint8_t Behavior, std::uint8_t Amplitude, std::uint8_t Frequency, Hand InHand, bool ApplyImmediately) { DS_TRIGGER_CALL(SetMachineGun26(StartZone, Behavior, Amplitude, Frequency, ToHand(InHand))); }
    bool Manager::SetMachineTrigger(DeviceId Id, std::uint8_t StartZone, std::uint8_t BehaviorFlag, std::uint8_t Force, std::uint8_t Amplitude, std::uint8_t Period, std::uint8_t Frequency, Hand InHand, bool ApplyImmediately) { DS_TRIGGER_CALL(SetMachine27(StartZone, BehaviorFlag, Force, Amplitude, Period, Frequency, ToHand(InHand))); }

    bool Manager::SetCustomTrigger(DeviceId Id, Hand InHand, const std::array<std::uint8_t, 10>& Bytes, bool ApplyImmediately)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadTrigger* Api = Pad->GetIGamepadTrigger(); if (!Api) return false;
        const std::vector<std::uint8_t> Data(Bytes.begin(), Bytes.end());
        Api->SetCustomTrigger(ToHand(InHand), Data); return Pimpl->Flush(Pad, ApplyImmediately);
    }
#undef DS_TRIGGER_CALL

    bool Manager::EnableMotion(DeviceId Id, bool Enabled)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadSensors* Api = Pad->GetIGamepadSensors(); if (!Api) return false;
        Api->EnableMotionSensor(Enabled); return true;
    }

    bool Manager::ResetGyroOrientation(DeviceId Id)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadSensors* Api = Pad->GetIGamepadSensors(); if (!Api) return false;
        Api->ResetGyroOrientation(); return true;
    }

    bool Manager::EnableTouch(DeviceId Id, bool Enabled)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadTouch* Api = Pad->GetIGamepadTouch(); if (!Api) return false;
        Api->EnableTouch(Enabled); return true;
    }

    bool Manager::EnableGesture(DeviceId Id, bool Enabled)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadTouch* Api = Pad->GetIGamepadTouch(); if (!Api) return false;
        Api->EnableGesture(Enabled); return true;
    }

    bool Manager::ConfigureDualSense(DeviceId Id, bool MicEnabled, bool HeadsetEnabled, bool SpeakerEnabled, std::uint8_t MicVolume, std::uint8_t AudioVolume, std::uint8_t RumbleMode, std::uint8_t RumbleReduce, std::uint8_t TriggerReduce, bool ApplyImmediately)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadSettings* Api = Pad->GetIGamepadSettings(); if (!Api) return false;
        Api->DualSenseSettings(MicEnabled ? 1 : 0, HeadsetEnabled ? 1 : 0, SpeakerEnabled ? 1 : 0, MicVolume, AudioVolume, RumbleMode, RumbleReduce, TriggerReduce);
        return Pimpl->Flush(Pad, ApplyImmediately);
    }

    bool Manager::SendAudioHaptics(DeviceId Id, const std::vector<std::uint8_t>& AudioData)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadHaptics* Api = Pad->GetIGamepadHaptics(); if (!Api) return false;
        Api->AudioHapticUpdate(AudioData); return true;
    }

    bool Manager::SendAudioHaptics(DeviceId Id, const std::vector<float>& AudioData)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadHaptics* Api = Pad->GetIGamepadHaptics(); if (!Api) return false;
        Api->AudioHapticUpdate(AudioData); return true;
    }

    bool Manager::SendAudioHaptics(DeviceId Id, const std::vector<std::uint8_t>& HapticsData, const std::vector<std::uint8_t>& AudioData)
    {
        IGamepadBase* Pad = Pimpl->Get(Id); if (!Pad) return false;
        IGamepadHaptics* Api = Pad->GetIGamepadHaptics(); if (!Api) return false;
        Api->AudioHapticUpdate(HapticsData, AudioData); return true;
    }
}
