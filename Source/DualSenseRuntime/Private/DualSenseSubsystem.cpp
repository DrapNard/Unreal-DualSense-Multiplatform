// SPDX-License-Identifier: MPL-2.0
#include "DualSenseSubsystem.h"

#include "DualSenseCoreManager.h"
#include "DualSenseCoreModule.h"
#include "Engine/GameInstance.h"

#include <array>
#include <vector>

namespace
{
    DualSense::Manager& NativeManager()
    {
        return IDualSenseCoreModule::Get().GetManager();
    }

    uint8 ByteValue(int32 Value)
    {
        return static_cast<uint8>(FMath::Clamp(Value, 0, 255));
    }

    DualSense::DeviceId NativeId(int32 Id)
    {
        return Id < 0 ? DualSense::InvalidDeviceId : static_cast<DualSense::DeviceId>(Id);
    }

    DualSense::Hand NativeHand(EDualSenseHand Hand)
    {
        switch (Hand)
        {
            case EDualSenseHand::Left: return DualSense::Hand::Left;
            case EDualSenseHand::Right: return DualSense::Hand::Right;
            default: return DualSense::Hand::Both;
        }
    }

    DualSense::PlayerLed NativePlayerLed(EDualSensePlayerLed Led)
    {
        switch (Led)
        {
            case EDualSensePlayerLed::One: return DualSense::PlayerLed::One;
            case EDualSensePlayerLed::Two: return DualSense::PlayerLed::Two;
            case EDualSensePlayerLed::Three: return DualSense::PlayerLed::Three;
            case EDualSensePlayerLed::All: return DualSense::PlayerLed::All;
            default: return DualSense::PlayerLed::Off;
        }
    }

    DualSense::MicrophoneLed NativeMicrophoneLed(EDualSenseMicrophoneLed Led)
    {
        switch (Led)
        {
            case EDualSenseMicrophoneLed::On: return DualSense::MicrophoneLed::On;
            case EDualSenseMicrophoneLed::Pulse: return DualSense::MicrophoneLed::Pulse;
            default: return DualSense::MicrophoneLed::Off;
        }
    }

    EDualSenseDeviceType UnrealDeviceType(DualSense::DeviceType Type)
    {
        switch (Type)
        {
            case DualSense::DeviceType::DualSense: return EDualSenseDeviceType::DualSense;
            case DualSense::DeviceType::DualSenseEdge: return EDualSenseDeviceType::DualSenseEdge;
            case DualSense::DeviceType::DualShock4: return EDualSenseDeviceType::DualShock4;
            default: return EDualSenseDeviceType::Unknown;
        }
    }

    EDualSenseConnectionType UnrealConnectionType(DualSense::ConnectionType Type)
    {
        switch (Type)
        {
            case DualSense::ConnectionType::USB: return EDualSenseConnectionType::USB;
            case DualSense::ConnectionType::Bluetooth: return EDualSenseConnectionType::Bluetooth;
            default: return EDualSenseConnectionType::Unknown;
        }
    }

    EDualSenseButton UnrealButton(DualSense::Button Button)
    {
        return static_cast<EDualSenseButton>(static_cast<uint8>(Button));
    }

    DualSense::Button NativeButton(EDualSenseButton Button)
    {
        return static_cast<DualSense::Button>(static_cast<uint8>(Button));
    }

    FDualSenseDeviceInfo UnrealInfo(const DualSense::DeviceInfo& Info)
    {
        FDualSenseDeviceInfo Out;
        Out.DeviceId = static_cast<int32>(Info.Id);
        Out.DeviceType = UnrealDeviceType(Info.Type);
        Out.ConnectionType = UnrealConnectionType(Info.Connection);
        Out.DevicePath = UTF8_TO_TCHAR(Info.Path.c_str());
        return Out;
    }

    FDualSenseState UnrealState(const DualSense::State& State)
    {
        FDualSenseState Out;
        Out.LeftStick = FVector2D(State.LeftStick.X, State.LeftStick.Y);
        Out.RightStick = FVector2D(State.RightStick.X, State.RightStick.Y);
        Out.LeftTrigger = State.LeftTrigger;
        Out.RightTrigger = State.RightTrigger;
        Out.Gyroscope = FVector(State.Gyroscope.X, State.Gyroscope.Y, State.Gyroscope.Z);
        Out.Accelerometer = FVector(State.Accelerometer.X, State.Accelerometer.Y, State.Accelerometer.Z);
        Out.Gravity = FVector(State.Gravity.X, State.Gravity.Y, State.Gravity.Z);
        Out.Tilt = FVector(State.Tilt.X, State.Tilt.Y, State.Tilt.Z);
        Out.TouchId = State.TouchId;
        Out.TouchFingerCount = State.TouchFingerCount;
        Out.TouchDirectionRaw = State.TouchDirectionRaw;
        Out.bIsTouching = State.IsTouching;
        Out.TouchRadius = FVector2D(State.TouchRadius.X, State.TouchRadius.Y);
        Out.TouchPosition = FVector2D(State.TouchPosition.X, State.TouchPosition.Y);
        Out.TouchRelative = FVector2D(State.TouchRelative.X, State.TouchRelative.Y);
        Out.BatteryLevel = State.BatteryLevel;
        Out.bHasPhoneConnected = State.HasPhoneConnected;
        for (std::size_t Index = 0; Index < State.Buttons.size(); ++Index)
        {
            if (State.Buttons[Index])
            {
                Out.PressedButtons.Add(UnrealButton(static_cast<DualSense::Button>(Index)));
            }
        }
        return Out;
    }

    DualSense::Color NativeColor(const FLinearColor& Color)
    {
        const FColor Bytes = Color.GetClamped().ToFColor(false);
        return {Bytes.R, Bytes.G, Bytes.B};
    }
}

void UDualSenseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bInitialized = NativeManager().Start();
    if (bInitialized)
    {
        NativeManager().RequestImmediateDetection();
    }
}

void UDualSenseSubsystem::Deinitialize()
{
    KnownDeviceIds.Empty();
    PreviousStates.Empty();
    bInitialized = false;
    Super::Deinitialize();
}

void UDualSenseSubsystem::Tick(float DeltaTime)
{
    if (!bInitialized) return;
    NativeManager().Tick(DeltaTime);
    PollEvents();
}

TStatId UDualSenseSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UDualSenseSubsystem, STATGROUP_Tickables);
}

UWorld* UDualSenseSubsystem::GetTickableGameObjectWorld() const
{
    const UGameInstance* Instance = GetGameInstance();
    return Instance ? Instance->GetWorld() : nullptr;
}

void UDualSenseSubsystem::PollEvents()
{
    const TArray<int32> CurrentIds = GetConnectedDeviceIds();
    TSet<int32> CurrentSet;
    for (int32 Id : CurrentIds) CurrentSet.Add(Id);

    for (int32 Id : CurrentIds)
    {
        if (!KnownDeviceIds.Contains(Id))
        {
            FDualSenseDeviceInfo Info;
            if (GetDeviceInfo(Id, Info)) OnDeviceConnected.Broadcast(Info);
        }

        FDualSenseState State;
        if (GetState(Id, State))
        {
            const FDualSenseState* Previous = PreviousStates.Find(Id);
            if (Previous)
            {
                for (uint8 RawButton = 0; RawButton <= static_cast<uint8>(EDualSenseButton::PaddleRight); ++RawButton)
                {
                    const EDualSenseButton Button = static_cast<EDualSenseButton>(RawButton);
                    const bool WasDown = Previous->PressedButtons.Contains(Button);
                    const bool IsDown = State.PressedButtons.Contains(Button);
                    if (IsDown && !WasDown) OnButtonPressed.Broadcast(Id, Button);
                    if (!IsDown && WasDown) OnButtonReleased.Broadcast(Id, Button);
                }
            }
            PreviousStates.Add(Id, State);
        }
    }

    for (int32 OldId : KnownDeviceIds)
    {
        if (!CurrentSet.Contains(OldId))
        {
            PreviousStates.Remove(OldId);
            OnDeviceDisconnected.Broadcast(OldId);
        }
    }
    KnownDeviceIds = MoveTemp(CurrentSet);
}

void UDualSenseSubsystem::RequestImmediateDetection() { NativeManager().RequestImmediateDetection(); }

TArray<int32> UDualSenseSubsystem::GetConnectedDeviceIds() const
{
    TArray<int32> Result;
    for (DualSense::DeviceId Id : NativeManager().GetConnectedDeviceIds()) Result.Add(static_cast<int32>(Id));
    return Result;
}

bool UDualSenseSubsystem::IsDeviceConnected(int32 DeviceId) const { return NativeManager().IsConnected(NativeId(DeviceId)); }

bool UDualSenseSubsystem::GetDeviceInfo(int32 DeviceId, FDualSenseDeviceInfo& DeviceInfo) const
{
    DualSense::DeviceInfo Info;
    if (!NativeManager().GetDeviceInfo(NativeId(DeviceId), Info)) return false;
    DeviceInfo = UnrealInfo(Info);
    return true;
}

bool UDualSenseSubsystem::GetState(int32 DeviceId, FDualSenseState& State) const
{
    DualSense::State Native;
    if (!NativeManager().GetState(NativeId(DeviceId), Native)) return false;
    State = UnrealState(Native);
    return true;
}

FDualSenseCapabilities UDualSenseSubsystem::GetCapabilities(int32 DeviceId) const
{
    const DualSense::Capabilities Caps = NativeManager().GetCapabilities(NativeId(DeviceId));
    FDualSenseCapabilities Out;
    Out.bLightbar = Caps.Lightbar;
    Out.bPlayerLed = Caps.PlayerLed;
    Out.bMicrophoneLed = Caps.MicrophoneLed;
    Out.bRumble = Caps.Rumble;
    Out.bAdaptiveTriggers = Caps.AdaptiveTriggers;
    Out.bMotion = Caps.Motion;
    Out.bTouch = Caps.Touch;
    Out.bAudioHaptics = Caps.AudioHaptics;
    Out.bAudioSettings = Caps.AudioSettings;
    return Out;
}

bool UDualSenseSubsystem::IsButtonDown(int32 DeviceId, EDualSenseButton Button) const
{
    DualSense::State State;
    return NativeManager().GetState(NativeId(DeviceId), State) && State.Buttons[static_cast<std::size_t>(NativeButton(Button))];
}

bool UDualSenseSubsystem::ApplyOutput(int32 DeviceId) { return NativeManager().ApplyOutput(NativeId(DeviceId)); }
bool UDualSenseSubsystem::SetVibration(int32 DeviceId, int32 LeftMotor, int32 RightMotor, bool bApplyImmediately) { return NativeManager().SetVibration(NativeId(DeviceId), ByteValue(LeftMotor), ByteValue(RightMotor), bApplyImmediately); }
bool UDualSenseSubsystem::SetLightbar(int32 DeviceId, FLinearColor Color, bool bApplyImmediately) { return NativeManager().SetLightbar(NativeId(DeviceId), NativeColor(Color), bApplyImmediately); }
bool UDualSenseSubsystem::SetLightbarFlash(int32 DeviceId, FLinearColor Color, float BrightnessTime, float ToggleTime, bool bApplyImmediately) { return NativeManager().SetLightbarFlash(NativeId(DeviceId), NativeColor(Color), FMath::Max(0.0f, BrightnessTime), FMath::Max(0.0f, ToggleTime), bApplyImmediately); }
bool UDualSenseSubsystem::SetPlayerLed(int32 DeviceId, EDualSensePlayerLed Led, int32 Brightness, bool bApplyImmediately) { return NativeManager().SetPlayerLed(NativeId(DeviceId), NativePlayerLed(Led), ByteValue(Brightness), bApplyImmediately); }
bool UDualSenseSubsystem::SetMicrophoneLed(int32 DeviceId, EDualSenseMicrophoneLed Led, bool bApplyImmediately) { return NativeManager().SetMicrophoneLed(NativeId(DeviceId), NativeMicrophoneLed(Led), bApplyImmediately); }
bool UDualSenseSubsystem::ResetLights(int32 DeviceId, bool bApplyImmediately) { return NativeManager().ResetLights(NativeId(DeviceId), bApplyImmediately); }

bool UDualSenseSubsystem::StopTrigger(int32 DeviceId, EDualSenseHand Hand, bool bApplyImmediately) { return NativeManager().StopTrigger(NativeId(DeviceId), NativeHand(Hand), bApplyImmediately); }
bool UDualSenseSubsystem::SetGameCubeTrigger(int32 DeviceId, EDualSenseHand Hand, bool bApplyImmediately) { return NativeManager().SetGameCubeTrigger(NativeId(DeviceId), NativeHand(Hand), bApplyImmediately); }
bool UDualSenseSubsystem::SetResistanceTrigger(int32 DeviceId, int32 StartZone, int32 Strength, EDualSenseHand Hand, bool bApplyImmediately) { return NativeManager().SetResistanceTrigger(NativeId(DeviceId), ByteValue(StartZone), ByteValue(Strength), NativeHand(Hand), bApplyImmediately); }
bool UDualSenseSubsystem::SetBowTrigger(int32 DeviceId, int32 StartZone, int32 SnapBack, EDualSenseHand Hand, bool bApplyImmediately) { return NativeManager().SetBowTrigger(NativeId(DeviceId), ByteValue(StartZone), ByteValue(SnapBack), NativeHand(Hand), bApplyImmediately); }
bool UDualSenseSubsystem::SetGallopingTrigger(int32 DeviceId, int32 StartPosition, int32 EndPosition, int32 FirstFoot, int32 SecondFoot, int32 Frequency, EDualSenseHand Hand, bool bApplyImmediately) { return NativeManager().SetGallopingTrigger(NativeId(DeviceId), ByteValue(StartPosition), ByteValue(EndPosition), ByteValue(FirstFoot), ByteValue(SecondFoot), ByteValue(Frequency), NativeHand(Hand), bApplyImmediately); }
bool UDualSenseSubsystem::SetWeaponTrigger(int32 DeviceId, int32 StartZone, int32 Amplitude, int32 Behavior, int32 Trigger, EDualSenseHand Hand, bool bApplyImmediately) { return NativeManager().SetWeaponTrigger(NativeId(DeviceId), ByteValue(StartZone), ByteValue(Amplitude), ByteValue(Behavior), ByteValue(Trigger), NativeHand(Hand), bApplyImmediately); }
bool UDualSenseSubsystem::SetMachineGunTrigger(int32 DeviceId, int32 StartZone, int32 Behavior, int32 Amplitude, int32 Frequency, EDualSenseHand Hand, bool bApplyImmediately) { return NativeManager().SetMachineGunTrigger(NativeId(DeviceId), ByteValue(StartZone), ByteValue(Behavior), ByteValue(Amplitude), ByteValue(Frequency), NativeHand(Hand), bApplyImmediately); }
bool UDualSenseSubsystem::SetMachineTrigger(int32 DeviceId, int32 StartZone, int32 BehaviorFlag, int32 Force, int32 Amplitude, int32 Period, int32 Frequency, EDualSenseHand Hand, bool bApplyImmediately) { return NativeManager().SetMachineTrigger(NativeId(DeviceId), ByteValue(StartZone), ByteValue(BehaviorFlag), ByteValue(Force), ByteValue(Amplitude), ByteValue(Period), ByteValue(Frequency), NativeHand(Hand), bApplyImmediately); }

bool UDualSenseSubsystem::SetCustomTrigger(int32 DeviceId, EDualSenseHand Hand, const TArray<uint8>& TenBytes, bool bApplyImmediately)
{
    if (TenBytes.Num() != 10) return false;
    std::array<std::uint8_t, 10> Bytes{};
    for (int32 Index = 0; Index < 10; ++Index) Bytes[static_cast<std::size_t>(Index)] = TenBytes[Index];
    return NativeManager().SetCustomTrigger(NativeId(DeviceId), NativeHand(Hand), Bytes, bApplyImmediately);
}

bool UDualSenseSubsystem::EnableMotion(int32 DeviceId, bool bEnabled) { return NativeManager().EnableMotion(NativeId(DeviceId), bEnabled); }
bool UDualSenseSubsystem::ResetGyroOrientation(int32 DeviceId) { return NativeManager().ResetGyroOrientation(NativeId(DeviceId)); }
bool UDualSenseSubsystem::EnableTouch(int32 DeviceId, bool bEnabled) { return NativeManager().EnableTouch(NativeId(DeviceId), bEnabled); }
bool UDualSenseSubsystem::EnableGesture(int32 DeviceId, bool bEnabled) { return NativeManager().EnableGesture(NativeId(DeviceId), bEnabled); }

bool UDualSenseSubsystem::ConfigureDualSense(int32 DeviceId, bool bMicEnabled, bool bHeadsetEnabled, bool bSpeakerEnabled, int32 MicVolume, int32 AudioVolume, int32 RumbleMode, int32 RumbleReduce, int32 TriggerReduce, bool bApplyImmediately)
{
    return NativeManager().ConfigureDualSense(NativeId(DeviceId), bMicEnabled, bHeadsetEnabled, bSpeakerEnabled, ByteValue(MicVolume), ByteValue(AudioVolume), ByteValue(RumbleMode), ByteValue(RumbleReduce), ByteValue(TriggerReduce), bApplyImmediately);
}

bool UDualSenseSubsystem::SendAudioHapticsBytes(int32 DeviceId, const TArray<uint8>& AudioData)
{
    std::vector<std::uint8_t> Data;
    Data.reserve(static_cast<std::size_t>(AudioData.Num()));
    for (uint8 Value : AudioData) Data.push_back(Value);
    return NativeManager().SendAudioHaptics(NativeId(DeviceId), Data);
}

bool UDualSenseSubsystem::SendAudioHapticsFloats(int32 DeviceId, const TArray<float>& AudioData)
{
    std::vector<float> Data;
    Data.reserve(static_cast<std::size_t>(AudioData.Num()));
    for (float Value : AudioData) Data.push_back(Value);
    return NativeManager().SendAudioHaptics(NativeId(DeviceId), Data);
}

bool UDualSenseSubsystem::SendAudioAndHapticsBytes(int32 DeviceId, const TArray<uint8>& HapticsData, const TArray<uint8>& AudioData)
{
    std::vector<std::uint8_t> Haptics;
    std::vector<std::uint8_t> Audio;
    Haptics.reserve(static_cast<std::size_t>(HapticsData.Num()));
    Audio.reserve(static_cast<std::size_t>(AudioData.Num()));
    for (uint8 Value : HapticsData) Haptics.push_back(Value);
    for (uint8 Value : AudioData) Audio.push_back(Value);
    return NativeManager().SendAudioHaptics(NativeId(DeviceId), Haptics, Audio);
}
