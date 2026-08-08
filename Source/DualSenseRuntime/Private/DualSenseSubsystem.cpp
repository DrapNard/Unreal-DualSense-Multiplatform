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

    float ClampedUnit(float Value)
    {
        return FMath::Clamp(Value, 0.0f, 1.0f);
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
    AccessibilitySettings.Empty();
    LastRequestedLightbarColors.Empty();
    LastLightbarColors.Empty();
    LightbarTransitions.Empty();
    LastPlayerLedModes.Empty();
    LastPlayerLedBrightness.Empty();
    PlayerLedTransitions.Empty();
    LastMicrophoneLedModes.Empty();
    bInitialized = false;
    Super::Deinitialize();
}

void UDualSenseSubsystem::Tick(float DeltaTime)
{
    if (!bInitialized) return;
    NativeManager().Tick(DeltaTime);
    UpdateOutputTransitions(DeltaTime);
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
                for (uint8 RawButton = 0; RawButton < static_cast<uint8>(EDualSenseButton::Count); ++RawButton)
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
            AccessibilitySettings.Remove(OldId);
            LastRequestedLightbarColors.Remove(OldId);
            LastLightbarColors.Remove(OldId);
            LightbarTransitions.Remove(OldId);
            LastPlayerLedModes.Remove(OldId);
            LastPlayerLedBrightness.Remove(OldId);
            PlayerLedTransitions.Remove(OldId);
            LastMicrophoneLedModes.Remove(OldId);
            OnDeviceDisconnected.Broadcast(OldId);
        }
    }
    KnownDeviceIds = MoveTemp(CurrentSet);
}

void UDualSenseSubsystem::UpdateOutputTransitions(float DeltaTime)
{
    TSet<int32> DirtyDevices;

    for (auto It = LightbarTransitions.CreateIterator(); It; ++It)
    {
        const int32 DeviceId = It.Key();
        if (!IsDeviceConnected(DeviceId))
        {
            It.RemoveCurrent();
            continue;
        }

        FLightbarTransition& Transition = It.Value();
        Transition.Elapsed += FMath::Max(0.0f, DeltaTime);
        const float LinearAlpha = Transition.Duration <= KINDA_SMALL_NUMBER
            ? 1.0f
            : FMath::Clamp(Transition.Elapsed / Transition.Duration, 0.0f, 1.0f);
        const float SmoothAlpha = LinearAlpha * LinearAlpha * (3.0f - 2.0f * LinearAlpha);
        const FLinearColor CurrentColor = FMath::Lerp(Transition.StartColor, Transition.TargetColor, SmoothAlpha);

        NativeManager().SetLightbar(NativeId(DeviceId), NativeColor(CurrentColor), false);
        LastLightbarColors.Add(DeviceId, CurrentColor);
        DirtyDevices.Add(DeviceId);

        if (LinearAlpha >= 1.0f)
        {
            if (Transition.bStartFlashOnComplete)
            {
                NativeManager().SetLightbarFlash(
                    NativeId(DeviceId),
                    NativeColor(Transition.TargetColor),
                    Transition.FlashBrightnessTime,
                    Transition.FlashToggleTime,
                    false);
            }
            if (Transition.bResetLightsOnComplete)
            {
                NativeManager().ResetLights(NativeId(DeviceId), false);
                LastLightbarColors.Add(DeviceId, FLinearColor::Black);
                LastPlayerLedModes.Add(DeviceId, EDualSensePlayerLed::Off);
                LastPlayerLedBrightness.Add(DeviceId, 0);
                LastMicrophoneLedModes.Add(DeviceId, EDualSenseMicrophoneLed::Off);
            }
            It.RemoveCurrent();
        }
    }

    for (auto It = PlayerLedTransitions.CreateIterator(); It; ++It)
    {
        const int32 DeviceId = It.Key();
        if (!IsDeviceConnected(DeviceId))
        {
            It.RemoveCurrent();
            continue;
        }

        FPlayerLedTransition& Transition = It.Value();
        Transition.Elapsed += FMath::Max(0.0f, DeltaTime);
        const float LinearAlpha = Transition.Duration <= KINDA_SMALL_NUMBER
            ? 1.0f
            : FMath::Clamp(Transition.Elapsed / Transition.Duration, 0.0f, 1.0f);
        const float SmoothAlpha = LinearAlpha * LinearAlpha * (3.0f - 2.0f * LinearAlpha);
        const int32 CurrentBrightness = FMath::RoundToInt(FMath::Lerp(
            static_cast<float>(Transition.StartBrightness),
            static_cast<float>(Transition.TargetBrightness),
            SmoothAlpha));

        NativeManager().SetPlayerLed(NativeId(DeviceId), NativePlayerLed(Transition.Led), ByteValue(CurrentBrightness), false);
        LastPlayerLedModes.Add(DeviceId, Transition.Led);
        LastPlayerLedBrightness.Add(DeviceId, CurrentBrightness);
        DirtyDevices.Add(DeviceId);

        if (LinearAlpha >= 1.0f)
        {
            It.RemoveCurrent();
        }
    }

    for (int32 DeviceId : DirtyDevices)
    {
        NativeManager().ApplyOutput(NativeId(DeviceId));
    }
}

const FDualSenseAccessibilitySettings& UDualSenseSubsystem::AccessibilityFor(int32 DeviceId) const
{
    if (const FDualSenseAccessibilitySettings* Found = AccessibilitySettings.Find(DeviceId))
    {
        return *Found;
    }

    static const FDualSenseAccessibilitySettings Defaults;
    return Defaults;
}

FLinearColor UDualSenseSubsystem::ApplyLightAccessibility(int32 DeviceId, const FLinearColor& Color) const
{
    const float Scale = ClampedUnit(AccessibilityFor(DeviceId).LightBrightnessScale);
    FLinearColor Result = Color.GetClamped();
    Result.R *= Scale;
    Result.G *= Scale;
    Result.B *= Scale;
    return Result;
}

uint8 UDualSenseSubsystem::ApplyRumbleAccessibility(int32 DeviceId, int32 Value) const
{
    const FDualSenseAccessibilitySettings& Settings = AccessibilityFor(DeviceId);
    if (Settings.bDisableRumble) return 0;
    return ByteValue(FMath::RoundToInt(static_cast<float>(FMath::Clamp(Value, 0, 255)) * ClampedUnit(Settings.RumbleIntensityScale)));
}

uint8 UDualSenseSubsystem::ApplyTriggerAccessibility(int32 DeviceId, int32 Value) const
{
    const FDualSenseAccessibilitySettings& Settings = AccessibilityFor(DeviceId);
    if (Settings.bDisableAdaptiveTriggers) return 0;
    return ByteValue(FMath::RoundToInt(static_cast<float>(FMath::Clamp(Value, 0, 255)) * ClampedUnit(Settings.TriggerIntensityScale)));
}

float UDualSenseSubsystem::EffectiveLightTransitionDuration(int32 DeviceId, float RequestedDuration) const
{
    return FMath::Max(FMath::Max(0.0f, RequestedDuration), FMath::Max(0.0f, AccessibilityFor(DeviceId).MinimumLightTransitionDuration));
}

bool UDualSenseSubsystem::AdaptiveTriggersAllowed(int32 DeviceId) const
{
    const FDualSenseAccessibilitySettings& Settings = AccessibilityFor(DeviceId);
    return !Settings.bDisableAdaptiveTriggers && ClampedUnit(Settings.TriggerIntensityScale) > KINDA_SMALL_NUMBER;
}

bool UDualSenseSubsystem::AudioHapticsAllowed(int32 DeviceId) const
{
    return !AccessibilityFor(DeviceId).bDisableAudioHaptics;
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

bool UDualSenseSubsystem::ApplyOutput(int32 DeviceId)
{
    return NativeManager().ApplyOutput(NativeId(DeviceId));
}

bool UDualSenseSubsystem::SetVibration(int32 DeviceId, int32 LeftMotor, int32 RightMotor, bool bApplyImmediately)
{
    return NativeManager().SetVibration(
        NativeId(DeviceId),
        ApplyRumbleAccessibility(DeviceId, LeftMotor),
        ApplyRumbleAccessibility(DeviceId, RightMotor),
        bApplyImmediately);
}

bool UDualSenseSubsystem::SetLightbar(int32 DeviceId, FLinearColor Color, bool bApplyImmediately, float TransitionDuration)
{
    if (!IsDeviceConnected(DeviceId)) return false;

    const FLinearColor RequestedColor = Color.GetClamped();
    LastRequestedLightbarColors.Add(DeviceId, RequestedColor);
    const FLinearColor TargetColor = ApplyLightAccessibility(DeviceId, RequestedColor);
    const float Duration = EffectiveLightTransitionDuration(DeviceId, TransitionDuration);
    LightbarTransitions.Remove(DeviceId);

    if (Duration <= KINDA_SMALL_NUMBER)
    {
        LastLightbarColors.Add(DeviceId, TargetColor);
        return NativeManager().SetLightbar(NativeId(DeviceId), NativeColor(TargetColor), bApplyImmediately);
    }

    FLightbarTransition Transition;
    Transition.StartColor = LastLightbarColors.FindRef(DeviceId);
    Transition.TargetColor = TargetColor;
    Transition.Duration = Duration;
    LightbarTransitions.Add(DeviceId, Transition);
    return true;
}

bool UDualSenseSubsystem::SetLightbarFlash(int32 DeviceId, FLinearColor Color, float BrightnessTime, float ToggleTime, bool bApplyImmediately, float TransitionDuration)
{
    if (!IsDeviceConnected(DeviceId)) return false;

    const FDualSenseAccessibilitySettings& Settings = AccessibilityFor(DeviceId);
    if (Settings.bDisableFlashingLights)
    {
        return SetLightbar(DeviceId, Color, bApplyImmediately, TransitionDuration);
    }

    const FLinearColor RequestedColor = Color.GetClamped();
    LastRequestedLightbarColors.Add(DeviceId, RequestedColor);
    const FLinearColor TargetColor = ApplyLightAccessibility(DeviceId, RequestedColor);
    const float Duration = EffectiveLightTransitionDuration(DeviceId, TransitionDuration);
    const float SafeBrightnessTime = FMath::Max(0.0f, BrightnessTime);
    const float SafeToggleTime = FMath::Max(0.0f, ToggleTime);
    LightbarTransitions.Remove(DeviceId);

    if (Duration <= KINDA_SMALL_NUMBER)
    {
        LastLightbarColors.Add(DeviceId, TargetColor);
        return NativeManager().SetLightbarFlash(NativeId(DeviceId), NativeColor(TargetColor), SafeBrightnessTime, SafeToggleTime, bApplyImmediately);
    }

    FLightbarTransition Transition;
    Transition.StartColor = LastLightbarColors.FindRef(DeviceId);
    Transition.TargetColor = TargetColor;
    Transition.Duration = Duration;
    Transition.bStartFlashOnComplete = true;
    Transition.FlashBrightnessTime = SafeBrightnessTime;
    Transition.FlashToggleTime = SafeToggleTime;
    LightbarTransitions.Add(DeviceId, Transition);
    return true;
}

bool UDualSenseSubsystem::SetPlayerLed(int32 DeviceId, EDualSensePlayerLed Led, int32 Brightness, bool bApplyImmediately, float TransitionDuration)
{
    if (!IsDeviceConnected(DeviceId)) return false;

    const float Scale = ClampedUnit(AccessibilityFor(DeviceId).LightBrightnessScale);
    const int32 ScaledBrightness = FMath::RoundToInt(static_cast<float>(FMath::Clamp(Brightness, 0, 255)) * Scale);
    const float Duration = EffectiveLightTransitionDuration(DeviceId, TransitionDuration);
    PlayerLedTransitions.Remove(DeviceId);

    if (Duration <= KINDA_SMALL_NUMBER)
    {
        LastPlayerLedModes.Add(DeviceId, Led);
        LastPlayerLedBrightness.Add(DeviceId, ScaledBrightness);
        return NativeManager().SetPlayerLed(NativeId(DeviceId), NativePlayerLed(Led), ByteValue(ScaledBrightness), bApplyImmediately);
    }

    FPlayerLedTransition Transition;
    Transition.Led = Led;
    Transition.StartBrightness = LastPlayerLedBrightness.FindRef(DeviceId);
    Transition.TargetBrightness = ScaledBrightness;
    Transition.Duration = Duration;
    PlayerLedTransitions.Add(DeviceId, Transition);
    return true;
}

bool UDualSenseSubsystem::SetMicrophoneLed(int32 DeviceId, EDualSenseMicrophoneLed Led, bool bApplyImmediately)
{
    if (AccessibilityFor(DeviceId).bDisableFlashingLights && Led == EDualSenseMicrophoneLed::Pulse)
    {
        Led = EDualSenseMicrophoneLed::On;
    }
    LastMicrophoneLedModes.Add(DeviceId, Led);
    return NativeManager().SetMicrophoneLed(NativeId(DeviceId), NativeMicrophoneLed(Led), bApplyImmediately);
}

bool UDualSenseSubsystem::ResetLights(int32 DeviceId, bool bApplyImmediately, float TransitionDuration)
{
    if (!IsDeviceConnected(DeviceId)) return false;

    LastRequestedLightbarColors.Add(DeviceId, FLinearColor::Black);
    const float Duration = EffectiveLightTransitionDuration(DeviceId, TransitionDuration);
    LightbarTransitions.Remove(DeviceId);

    if (Duration <= KINDA_SMALL_NUMBER)
    {
        LastLightbarColors.Add(DeviceId, FLinearColor::Black);
        LastPlayerLedModes.Add(DeviceId, EDualSensePlayerLed::Off);
        LastPlayerLedBrightness.Add(DeviceId, 0);
        LastMicrophoneLedModes.Add(DeviceId, EDualSenseMicrophoneLed::Off);
        return NativeManager().ResetLights(NativeId(DeviceId), bApplyImmediately);
    }

    FLightbarTransition Transition;
    Transition.StartColor = LastLightbarColors.FindRef(DeviceId);
    Transition.TargetColor = FLinearColor::Black;
    Transition.Duration = Duration;
    Transition.bResetLightsOnComplete = true;
    LightbarTransitions.Add(DeviceId, Transition);

    if (const EDualSensePlayerLed* CurrentLed = LastPlayerLedModes.Find(DeviceId))
    {
        SetPlayerLed(DeviceId, *CurrentLed, 0, false, Duration);
    }
    return true;
}

bool UDualSenseSubsystem::SetAccessibilitySettings(int32 DeviceId, const FDualSenseAccessibilitySettings& Settings)
{
    if (!IsDeviceConnected(DeviceId)) return false;

    FDualSenseAccessibilitySettings Sanitized = Settings;
    Sanitized.LightBrightnessScale = ClampedUnit(Sanitized.LightBrightnessScale);
    Sanitized.MinimumLightTransitionDuration = FMath::Max(0.0f, Sanitized.MinimumLightTransitionDuration);
    Sanitized.RumbleIntensityScale = ClampedUnit(Sanitized.RumbleIntensityScale);
    Sanitized.TriggerIntensityScale = ClampedUnit(Sanitized.TriggerIntensityScale);
    AccessibilitySettings.Add(DeviceId, Sanitized);

    // Apply hard-disable accessibility options immediately so an already-running
    // effect cannot continue after the player changes their accessibility profile.
    if (Sanitized.bDisableRumble)
    {
        NativeManager().SetVibration(NativeId(DeviceId), 0, 0, true);
    }
    if (Sanitized.bDisableAdaptiveTriggers)
    {
        NativeManager().StopTrigger(NativeId(DeviceId), DualSense::Hand::Both, true);
    }
    if (Sanitized.bDisableFlashingLights && LastRequestedLightbarColors.Contains(DeviceId))
    {
        LightbarTransitions.Remove(DeviceId);
        const FLinearColor StaticColor = ApplyLightAccessibility(DeviceId, LastRequestedLightbarColors.FindRef(DeviceId));
        LastLightbarColors.Add(DeviceId, StaticColor);
        NativeManager().SetLightbar(NativeId(DeviceId), NativeColor(StaticColor), true);
    }
    if (Sanitized.bDisableFlashingLights)
    {
        if (const EDualSenseMicrophoneLed* MicrophoneLed = LastMicrophoneLedModes.Find(DeviceId);
            MicrophoneLed && *MicrophoneLed == EDualSenseMicrophoneLed::Pulse)
        {
            SetMicrophoneLed(DeviceId, EDualSenseMicrophoneLed::On, true);
        }
    }

    return true;
}

FDualSenseAccessibilitySettings UDualSenseSubsystem::GetAccessibilitySettings(int32 DeviceId) const
{
    return AccessibilityFor(DeviceId);
}

bool UDualSenseSubsystem::ApplyAccessibilityPreset(int32 DeviceId, EDualSenseAccessibilityPreset Preset)
{
    FDualSenseAccessibilitySettings Settings;

    switch (Preset)
    {
        case EDualSenseAccessibilityPreset::ReducedHaptics:
            Settings.RumbleIntensityScale = 0.45f;
            Settings.TriggerIntensityScale = 0.45f;
            break;

        case EDualSenseAccessibilityPreset::Photosensitive:
            Settings.LightBrightnessScale = 0.55f;
            Settings.bDisableFlashingLights = true;
            Settings.MinimumLightTransitionDuration = 0.25f;
            break;

        case EDualSenseAccessibilityPreset::LowSensory:
            Settings.LightBrightnessScale = 0.45f;
            Settings.bDisableFlashingLights = true;
            Settings.MinimumLightTransitionDuration = 0.30f;
            Settings.RumbleIntensityScale = 0.25f;
            Settings.TriggerIntensityScale = 0.25f;
            Settings.bDisableAudioHaptics = true;
            break;

        case EDualSenseAccessibilityPreset::NoHaptics:
            Settings.bDisableRumble = true;
            Settings.bDisableAdaptiveTriggers = true;
            Settings.bDisableAudioHaptics = true;
            break;

        default:
            break;
    }

    return SetAccessibilitySettings(DeviceId, Settings);
}

bool UDualSenseSubsystem::ResetAccessibilitySettings(int32 DeviceId)
{
    if (!IsDeviceConnected(DeviceId)) return false;
    AccessibilitySettings.Remove(DeviceId);
    return true;
}

bool UDualSenseSubsystem::SetTriggerPreset(int32 DeviceId, EDualSenseTriggerPreset Preset, EDualSenseHand Hand, float Intensity, bool bApplyImmediately)
{
    const float SafeIntensity = ClampedUnit(Intensity);
    if (Preset == EDualSenseTriggerPreset::Off || SafeIntensity <= KINDA_SMALL_NUMBER)
    {
        return StopTrigger(DeviceId, Hand, bApplyImmediately);
    }

    switch (Preset)
    {
        case EDualSenseTriggerPreset::SoftResistance:
            return SetResistanceTrigger(DeviceId, 100, FMath::RoundToInt(80.0f * SafeIntensity), Hand, bApplyImmediately);
        case EDualSenseTriggerPreset::MediumResistance:
            return SetResistanceTrigger(DeviceId, 80, FMath::RoundToInt(160.0f * SafeIntensity), Hand, bApplyImmediately);
        case EDualSenseTriggerPreset::StrongResistance:
            return SetResistanceTrigger(DeviceId, 55, FMath::RoundToInt(230.0f * SafeIntensity), Hand, bApplyImmediately);
        case EDualSenseTriggerPreset::GameCube:
            return SetGameCubeTrigger(DeviceId, Hand, bApplyImmediately);
        case EDualSenseTriggerPreset::Bow:
            return SetBowTrigger(DeviceId, 72, FMath::RoundToInt(190.0f * SafeIntensity), Hand, bApplyImmediately);
        case EDualSenseTriggerPreset::Weapon:
            return SetWeaponTrigger(DeviceId, 4, FMath::Max(1, FMath::RoundToInt(8.0f * SafeIntensity)), 2, 6, Hand, bApplyImmediately);
        case EDualSenseTriggerPreset::Automatic:
            return SetMachineGunTrigger(DeviceId, 48, 2, SafeIntensity < 0.55f ? 1 : 2, 30, Hand, bApplyImmediately);
        default:
            return StopTrigger(DeviceId, Hand, bApplyImmediately);
    }
}

bool UDualSenseSubsystem::StopTrigger(int32 DeviceId, EDualSenseHand Hand, bool bApplyImmediately)
{
    return NativeManager().StopTrigger(NativeId(DeviceId), NativeHand(Hand), bApplyImmediately);
}

bool UDualSenseSubsystem::SetGameCubeTrigger(int32 DeviceId, EDualSenseHand Hand, bool bApplyImmediately)
{
    if (!AdaptiveTriggersAllowed(DeviceId)) return StopTrigger(DeviceId, Hand, bApplyImmediately);
    return NativeManager().SetGameCubeTrigger(NativeId(DeviceId), NativeHand(Hand), bApplyImmediately);
}

bool UDualSenseSubsystem::SetResistanceTrigger(int32 DeviceId, int32 StartZone, int32 Strength, EDualSenseHand Hand, bool bApplyImmediately)
{
    if (!AdaptiveTriggersAllowed(DeviceId)) return StopTrigger(DeviceId, Hand, bApplyImmediately);
    return NativeManager().SetResistanceTrigger(NativeId(DeviceId), ByteValue(StartZone), ApplyTriggerAccessibility(DeviceId, Strength), NativeHand(Hand), bApplyImmediately);
}

bool UDualSenseSubsystem::SetBowTrigger(int32 DeviceId, int32 StartZone, int32 SnapBack, EDualSenseHand Hand, bool bApplyImmediately)
{
    if (!AdaptiveTriggersAllowed(DeviceId)) return StopTrigger(DeviceId, Hand, bApplyImmediately);
    return NativeManager().SetBowTrigger(NativeId(DeviceId), ByteValue(StartZone), ApplyTriggerAccessibility(DeviceId, SnapBack), NativeHand(Hand), bApplyImmediately);
}

bool UDualSenseSubsystem::SetGallopingTrigger(int32 DeviceId, int32 StartPosition, int32 EndPosition, int32 FirstFoot, int32 SecondFoot, int32 Frequency, EDualSenseHand Hand, bool bApplyImmediately)
{
    if (!AdaptiveTriggersAllowed(DeviceId)) return StopTrigger(DeviceId, Hand, bApplyImmediately);
    return NativeManager().SetGallopingTrigger(
        NativeId(DeviceId), ByteValue(StartPosition), ByteValue(EndPosition),
        ApplyTriggerAccessibility(DeviceId, FirstFoot), ApplyTriggerAccessibility(DeviceId, SecondFoot),
        ByteValue(Frequency), NativeHand(Hand), bApplyImmediately);
}

bool UDualSenseSubsystem::SetWeaponTrigger(int32 DeviceId, int32 StartZone, int32 Amplitude, int32 Behavior, int32 Trigger, EDualSenseHand Hand, bool bApplyImmediately)
{
    if (!AdaptiveTriggersAllowed(DeviceId)) return StopTrigger(DeviceId, Hand, bApplyImmediately);
    return NativeManager().SetWeaponTrigger(NativeId(DeviceId), ByteValue(StartZone), ApplyTriggerAccessibility(DeviceId, Amplitude), ByteValue(Behavior), ByteValue(Trigger), NativeHand(Hand), bApplyImmediately);
}

bool UDualSenseSubsystem::SetMachineGunTrigger(int32 DeviceId, int32 StartZone, int32 Behavior, int32 Amplitude, int32 Frequency, EDualSenseHand Hand, bool bApplyImmediately)
{
    if (!AdaptiveTriggersAllowed(DeviceId)) return StopTrigger(DeviceId, Hand, bApplyImmediately);
    const float Scale = ClampedUnit(AccessibilityFor(DeviceId).TriggerIntensityScale);
    int32 EffectiveAmplitude = FMath::Clamp(Amplitude, 1, 2);
    if (Scale < 0.55f) EffectiveAmplitude = 1;
    return NativeManager().SetMachineGunTrigger(NativeId(DeviceId), ByteValue(StartZone), ByteValue(Behavior), ByteValue(EffectiveAmplitude), ByteValue(Frequency), NativeHand(Hand), bApplyImmediately);
}

bool UDualSenseSubsystem::SetMachineTrigger(int32 DeviceId, int32 StartZone, int32 BehaviorFlag, int32 Force, int32 Amplitude, int32 Period, int32 Frequency, EDualSenseHand Hand, bool bApplyImmediately)
{
    if (!AdaptiveTriggersAllowed(DeviceId)) return StopTrigger(DeviceId, Hand, bApplyImmediately);
    return NativeManager().SetMachineTrigger(
        NativeId(DeviceId), ByteValue(StartZone), ByteValue(BehaviorFlag),
        ApplyTriggerAccessibility(DeviceId, Force), ApplyTriggerAccessibility(DeviceId, Amplitude),
        ByteValue(Period), ByteValue(Frequency), NativeHand(Hand), bApplyImmediately);
}

bool UDualSenseSubsystem::SetCustomTrigger(int32 DeviceId, EDualSenseHand Hand, const TArray<uint8>& TenBytes, bool bApplyImmediately)
{
    if (!AdaptiveTriggersAllowed(DeviceId)) return StopTrigger(DeviceId, Hand, bApplyImmediately);
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
    if (!AudioHapticsAllowed(DeviceId)) return false;
    std::vector<std::uint8_t> Data;
    Data.reserve(static_cast<std::size_t>(AudioData.Num()));
    for (uint8 Value : AudioData) Data.push_back(Value);
    return NativeManager().SendAudioHaptics(NativeId(DeviceId), Data);
}

bool UDualSenseSubsystem::SendAudioHapticsFloats(int32 DeviceId, const TArray<float>& AudioData)
{
    if (!AudioHapticsAllowed(DeviceId)) return false;
    std::vector<float> Data;
    Data.reserve(static_cast<std::size_t>(AudioData.Num()));
    for (float Value : AudioData) Data.push_back(Value);
    return NativeManager().SendAudioHaptics(NativeId(DeviceId), Data);
}

bool UDualSenseSubsystem::SendAudioAndHapticsBytes(int32 DeviceId, const TArray<uint8>& HapticsData, const TArray<uint8>& AudioData)
{
    if (!AudioHapticsAllowed(DeviceId)) return false;
    std::vector<std::uint8_t> Haptics;
    std::vector<std::uint8_t> Audio;
    Haptics.reserve(static_cast<std::size_t>(HapticsData.Num()));
    Audio.reserve(static_cast<std::size_t>(AudioData.Num()));
    for (uint8 Value : HapticsData) Haptics.push_back(Value);
    for (uint8 Value : AudioData) Audio.push_back(Value);
    return NativeManager().SendAudioHaptics(NativeId(DeviceId), Haptics, Audio);
}
