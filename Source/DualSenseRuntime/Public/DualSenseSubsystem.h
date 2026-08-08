// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "DualSenseTypes.h"
#include "DualSenseSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDualSenseDeviceConnectedSignature, const FDualSenseDeviceInfo&, Device);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDualSenseDeviceDisconnectedSignature, int32, DeviceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDualSenseButtonSignature, int32, DeviceId, EDualSenseButton, Button);

UCLASS()
class DUALSENSERUNTIME_API UDualSenseSubsystem final : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !IsTemplate() && bInitialized; }
    virtual UWorld* GetTickableGameObjectWorld() const override;

    UPROPERTY(BlueprintAssignable, Category="DualSense|Events", meta=(ToolTip="Fired when a supported controller is detected and ready for use.")) FDualSenseDeviceConnectedSignature OnDeviceConnected;
    UPROPERTY(BlueprintAssignable, Category="DualSense|Events", meta=(ToolTip="Fired when a previously connected controller disappears.")) FDualSenseDeviceDisconnectedSignature OnDeviceDisconnected;
    UPROPERTY(BlueprintAssignable, Category="DualSense|Events", meta=(ToolTip="Fired once when a supported controller button changes from released to pressed.")) FDualSenseButtonSignature OnButtonPressed;
    UPROPERTY(BlueprintAssignable, Category="DualSense|Events", meta=(ToolTip="Fired once when a supported controller button changes from pressed to released.")) FDualSenseButtonSignature OnButtonReleased;

    UFUNCTION(BlueprintCallable, Category="DualSense|Devices", meta=(ToolTip="Request an immediate hardware scan instead of waiting for the next periodic detection pass."))
    void RequestImmediateDetection();

    UFUNCTION(BlueprintPure, Category="DualSense|Devices", meta=(ToolTip="Return the IDs of every currently connected supported controller."))
    TArray<int32> GetConnectedDeviceIds() const;

    UFUNCTION(BlueprintPure, Category="DualSense|Devices", meta=(ToolTip="Return true when Device Id currently refers to a connected controller."))
    bool IsDeviceConnected(int32 DeviceId) const;

    UFUNCTION(BlueprintPure, Category="DualSense|Devices", meta=(ToolTip="Read model, transport, and device-path information for a connected controller."))
    bool GetDeviceInfo(int32 DeviceId, FDualSenseDeviceInfo& DeviceInfo) const;

    UFUNCTION(BlueprintPure, Category="DualSense|Devices", meta=(ToolTip="Read the latest complete controller state, including sticks, triggers, buttons, motion, touch, and battery."))
    bool GetState(int32 DeviceId, FDualSenseState& State) const;

    UFUNCTION(BlueprintPure, Category="DualSense|Devices", meta=(ToolTip="Return which optional output/input features are available on this controller model."))
    FDualSenseCapabilities GetCapabilities(int32 DeviceId) const;

    UFUNCTION(BlueprintPure, Category="DualSense|Buttons", meta=(ToolTip="Test whether a single controller button or virtual threshold button is currently held."))
    bool IsButtonDown(int32 DeviceId, EDualSenseButton Button) const;

    UFUNCTION(BlueprintCallable, Category="DualSense|Output", meta=(ToolTip="Flush output changes that were queued with Apply Immediately disabled."))
    bool ApplyOutput(int32 DeviceId);

    UFUNCTION(BlueprintCallable, Category="DualSense|Output", meta=(ClampMin="0", ClampMax="255", ToolTip="Set left/right rumble motors. Accessibility settings can reduce or disable the requested intensity."))
    bool SetVibration(int32 DeviceId, int32 LeftMotor = 128, int32 RightMotor = 128, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Lights", meta=(ToolTip="Set the RGB lightbar. Transition Duration is in seconds: 0 changes instantly; values above 0 smoothly ease from the current requested color. Smooth transitions are tick-driven and flush their animation frames automatically. Accessibility settings can enforce a longer minimum transition and brightness limit."))
    bool SetLightbar(int32 DeviceId, FLinearColor Color, bool bApplyImmediately = true, float TransitionDuration = 0.0f);

    UFUNCTION(BlueprintCallable, Category="DualSense|Lights", meta=(ToolTip="Set a flashing lightbar effect. Transition Duration smoothly fades to the requested color before flashing starts. Accessibility settings can replace flashing with a static color."))
    bool SetLightbarFlash(int32 DeviceId, FLinearColor Color, float BrightnessTime = 0.5f, float ToggleTime = 0.5f, bool bApplyImmediately = true, float TransitionDuration = 0.0f);

    UFUNCTION(BlueprintCallable, Category="DualSense|Lights", meta=(ClampMin="0", ClampMax="255", ToolTip="Set the DualSense player LED pattern and brightness. Transition Duration smoothly fades LED brightness; 0 changes immediately. Accessibility light-brightness limits are applied automatically."))
    bool SetPlayerLed(int32 DeviceId, EDualSensePlayerLed Led = EDualSensePlayerLed::One, int32 Brightness = 255, bool bApplyImmediately = true, float TransitionDuration = 0.0f);

    UFUNCTION(BlueprintCallable, Category="DualSense|Lights", meta=(ToolTip="Set the microphone LED mode on controllers that support it. This hardware mode is discrete and does not expose a smooth brightness fade; Pulse is converted to steady On when flashing is disabled by accessibility settings."))
    bool SetMicrophoneLed(int32 DeviceId, EDualSenseMicrophoneLed Led = EDualSenseMicrophoneLed::Off, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Lights", meta=(ToolTip="Turn controller lights off. Transition Duration smoothly fades the RGB lightbar to black before the final reset; 0 resets immediately."))
    bool ResetLights(int32 DeviceId, bool bApplyImmediately = true, float TransitionDuration = 0.0f);

    UFUNCTION(BlueprintCallable, Category="DualSense|Accessibility", meta=(ToolTip="Set per-controller accessibility limits for lights, rumble, adaptive triggers, and audio haptics. The settings automatically affect later output nodes."))
    bool SetAccessibilitySettings(int32 DeviceId, const FDualSenseAccessibilitySettings& Settings);

    UFUNCTION(BlueprintPure, Category="DualSense|Accessibility", meta=(ToolTip="Get the active per-controller accessibility settings. Returns default settings when no custom profile has been assigned."))
    FDualSenseAccessibilitySettings GetAccessibilitySettings(int32 DeviceId) const;

    UFUNCTION(BlueprintCallable, Category="DualSense|Accessibility", meta=(ToolTip="Apply a ready-made accessibility profile. Use Set Accessibility Settings when you need custom limits."))
    bool ApplyAccessibilityPreset(int32 DeviceId, EDualSenseAccessibilityPreset Preset = EDualSenseAccessibilityPreset::Default);

    UFUNCTION(BlueprintCallable, Category="DualSense|Accessibility", meta=(ToolTip="Remove the custom accessibility profile for this controller and restore full feedback for future output calls."))
    bool ResetAccessibilitySettings(int32 DeviceId);

    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers", meta=(DisplayName="Set Trigger Preset (Simple)", ToolTip="Apply a gameplay-friendly adaptive-trigger preset with sensible defaults. Intensity is 0 to 1 and is further limited by the active accessibility profile."))
    bool SetTriggerPreset(int32 DeviceId, EDualSenseTriggerPreset Preset = EDualSenseTriggerPreset::MediumResistance, EDualSenseHand Hand = EDualSenseHand::Both, float Intensity = 1.0f, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers", meta=(ToolTip="Disable adaptive-trigger effects for the selected hand."))
    bool StopTrigger(int32 DeviceId, EDualSenseHand Hand = EDualSenseHand::Both, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers|Advanced", meta=(ToolTip="Apply the upstream fixed GameCube-style adaptive-trigger effect. For a simpler scalable option use Set Trigger Preset (Simple)."))
    bool SetGameCubeTrigger(int32 DeviceId, EDualSenseHand Hand = EDualSenseHand::Both, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers|Advanced", meta=(ToolTip="Apply raw resistance parameters. Start Zone and Strength use 0-255 upstream values; accessibility trigger scaling is applied to Strength."))
    bool SetResistanceTrigger(int32 DeviceId, int32 StartZone = 80, int32 Strength = 160, EDualSenseHand Hand = EDualSenseHand::Both, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers|Advanced", meta=(ToolTip="Apply the upstream bow effect. Defaults provide a usable bow-string feel; accessibility trigger scaling is applied to Snap Back."))
    bool SetBowTrigger(int32 DeviceId, int32 StartZone = 72, int32 SnapBack = 180, EDualSenseHand Hand = EDualSenseHand::Both, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers|Advanced", meta=(ToolTip="Apply the upstream galloping trigger effect. The default values are a ready-to-test profile; accessibility scaling is applied to both foot intensities."))
    bool SetGallopingTrigger(int32 DeviceId, int32 StartPosition = 2, int32 EndPosition = 6, int32 FirstFoot = 6, int32 SecondFoot = 4, int32 Frequency = 25, EDualSenseHand Hand = EDualSenseHand::Both, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers|Advanced", meta=(ToolTip="Apply the upstream weapon trigger effect. Defaults are intentionally conservative; accessibility scaling is applied to Amplitude."))
    bool SetWeaponTrigger(int32 DeviceId, int32 StartZone = 4, int32 Amplitude = 8, int32 Behavior = 2, int32 Trigger = 6, EDualSenseHand Hand = EDualSenseHand::Both, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers|Advanced", meta=(ToolTip="Apply the upstream machine-gun trigger effect. Accessibility may reduce the upstream two-level amplitude or disable the effect."))
    bool SetMachineGunTrigger(int32 DeviceId, int32 StartZone = 48, int32 Behavior = 2, int32 Amplitude = 2, int32 Frequency = 30, EDualSenseHand Hand = EDualSenseHand::Both, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers|Advanced", meta=(ToolTip="Apply the upstream machine trigger effect. Accessibility scaling is applied to Force and Amplitude."))
    bool SetMachineTrigger(int32 DeviceId, int32 StartZone = 48, int32 BehaviorFlag = 2, int32 Force = 8, int32 Amplitude = 7, int32 Period = 40, int32 Frequency = 25, EDualSenseHand Hand = EDualSenseHand::Both, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers|Advanced", meta=(ToolTip="Send an exact 10-byte custom adaptive-trigger payload. The payload is not modified by accessibility scaling, but the Disable Adaptive Triggers accessibility option is still respected."))
    bool SetCustomTrigger(int32 DeviceId, EDualSenseHand Hand, const TArray<uint8>& TenBytes, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Sensors", meta=(ToolTip="Enable or disable motion processing for this controller."))
    bool EnableMotion(int32 DeviceId, bool bEnabled = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Sensors", meta=(ToolTip="Reset the accumulated gyro orientation used by the upstream motion parser."))
    bool ResetGyroOrientation(int32 DeviceId);

    UFUNCTION(BlueprintCallable, Category="DualSense|Touch", meta=(ToolTip="Enable or disable touchpad processing for this controller."))
    bool EnableTouch(int32 DeviceId, bool bEnabled = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Touch", meta=(ToolTip="Enable or disable upstream touch gesture processing for this controller."))
    bool EnableGesture(int32 DeviceId, bool bEnabled = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Settings", meta=(ToolTip="Configure DualSense microphone, headset, speaker, volume, rumble reduction, and trigger reduction settings using the upstream raw values."))
    bool ConfigureDualSense(int32 DeviceId, bool bMicEnabled = true, bool bHeadsetEnabled = true, bool bSpeakerEnabled = true, int32 MicVolume = 255, int32 AudioVolume = 255, int32 RumbleMode = 0, int32 RumbleReduce = 0, int32 TriggerReduce = 0, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Audio Haptics", meta=(ToolTip="Send byte-form audio-haptics data through the upstream library. The call is blocked when the active accessibility profile disables audio haptics."))
    bool SendAudioHapticsBytes(int32 DeviceId, const TArray<uint8>& AudioData);

    UFUNCTION(BlueprintCallable, Category="DualSense|Audio Haptics", meta=(ToolTip="Send float-form audio-haptics data through the upstream library. The call is blocked when the active accessibility profile disables audio haptics."))
    bool SendAudioHapticsFloats(int32 DeviceId, const TArray<float>& AudioData);

    UFUNCTION(BlueprintCallable, Category="DualSense|Audio Haptics", meta=(ToolTip="Send separate haptics and audio byte buffers through the upstream library. The call is blocked when the active accessibility profile disables audio haptics."))
    bool SendAudioAndHapticsBytes(int32 DeviceId, const TArray<uint8>& HapticsData, const TArray<uint8>& AudioData);

private:
    struct FLightbarTransition
    {
        FLinearColor StartColor = FLinearColor::Black;
        FLinearColor TargetColor = FLinearColor::Black;
        float Elapsed = 0.0f;
        float Duration = 0.0f;
        bool bStartFlashOnComplete = false;
        float FlashBrightnessTime = 0.0f;
        float FlashToggleTime = 0.0f;
        bool bResetLightsOnComplete = false;
    };

    struct FPlayerLedTransition
    {
        EDualSensePlayerLed Led = EDualSensePlayerLed::Off;
        int32 StartBrightness = 0;
        int32 TargetBrightness = 0;
        float Elapsed = 0.0f;
        float Duration = 0.0f;
    };

    bool bInitialized = false;
    TSet<int32> KnownDeviceIds;
    TMap<int32, FDualSenseState> PreviousStates;
    TMap<int32, FDualSenseAccessibilitySettings> AccessibilitySettings;
    TMap<int32, FLinearColor> LastRequestedLightbarColors;
    TMap<int32, FLinearColor> LastLightbarColors;
    TMap<int32, FLightbarTransition> LightbarTransitions;
    TMap<int32, EDualSensePlayerLed> LastPlayerLedModes;
    TMap<int32, int32> LastPlayerLedBrightness;
    TMap<int32, FPlayerLedTransition> PlayerLedTransitions;
    TMap<int32, EDualSenseMicrophoneLed> LastMicrophoneLedModes;

    void PollEvents();
    void UpdateOutputTransitions(float DeltaTime);
    const FDualSenseAccessibilitySettings& AccessibilityFor(int32 DeviceId) const;
    FLinearColor ApplyLightAccessibility(int32 DeviceId, const FLinearColor& Color) const;
    uint8 ApplyRumbleAccessibility(int32 DeviceId, int32 Value) const;
    uint8 ApplyTriggerAccessibility(int32 DeviceId, int32 Value) const;
    float EffectiveLightTransitionDuration(int32 DeviceId, float RequestedDuration) const;
    bool AdaptiveTriggersAllowed(int32 DeviceId) const;
    bool AudioHapticsAllowed(int32 DeviceId) const;
};
