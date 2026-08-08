// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "CoreMinimal.h"
#include "DualSenseTypes.generated.h"

UENUM(BlueprintType)
enum class EDualSenseDeviceType : uint8
{
    DualSense,
    DualSenseEdge,
    DualShock4,
    Unknown
};

UENUM(BlueprintType)
enum class EDualSenseConnectionType : uint8
{
    USB,
    Bluetooth,
    Unknown
};

UENUM(BlueprintType)
enum class EDualSenseHand : uint8
{
    Left,
    Right,
    Both
};

UENUM(BlueprintType)
enum class EDualSensePlayerLed : uint8
{
    Off,
    One,
    Two,
    Three,
    All
};

UENUM(BlueprintType)
enum class EDualSenseMicrophoneLed : uint8
{
    On,
    Off,
    Pulse
};

/** High-level adaptive-trigger effects intended for normal gameplay Blueprints. */
UENUM(BlueprintType)
enum class EDualSenseTriggerPreset : uint8
{
    Off UMETA(DisplayName="Off", ToolTip="Disable the adaptive-trigger effect."),
    SoftResistance UMETA(DisplayName="Soft Resistance", ToolTip="Gentle resistance suitable for frequent actions."),
    MediumResistance UMETA(DisplayName="Medium Resistance", ToolTip="Balanced resistance for general gameplay."),
    StrongResistance UMETA(DisplayName="Strong Resistance", ToolTip="Firm resistance for deliberate actions."),
    GameCube UMETA(DisplayName="GameCube Style", ToolTip="GameCube-style trigger stop using the upstream fixed profile."),
    Bow UMETA(DisplayName="Bow", ToolTip="Bow-string tension and snap-back effect."),
    Weapon UMETA(DisplayName="Weapon", ToolTip="Short weapon-break effect."),
    Automatic UMETA(DisplayName="Automatic", ToolTip="Repeating automatic-fire style effect.")
};

/** Ready-made sensory/accessibility profiles. Custom settings remain available for fine control. */
UENUM(BlueprintType)
enum class EDualSenseAccessibilityPreset : uint8
{
    Default UMETA(DisplayName="Default", ToolTip="Full controller feedback with no accessibility reductions."),
    ReducedHaptics UMETA(DisplayName="Reduced Haptics", ToolTip="Lower rumble and adaptive-trigger intensity."),
    Photosensitive UMETA(DisplayName="Reduced Flashing", ToolTip="Disable flashing, lower LED brightness, and smooth abrupt light changes."),
    LowSensory UMETA(DisplayName="Low Sensory", ToolTip="Reduce light intensity, flashing, rumble, trigger force, and audio haptics."),
    NoHaptics UMETA(DisplayName="No Haptics", ToolTip="Disable rumble, adaptive-trigger effects, and audio haptics.")
};

UENUM(BlueprintType)
enum class EDualSenseButton : uint8
{
    Cross,
    Square,
    Triangle,
    Circle,
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,
    L1,
    R1,
    L3,
    R3,
    PS,
    Share,
    Options,
    Touchpad,
    Mute,
    FnLeft,
    FnRight,
    PaddleLeft,
    PaddleRight,
    LeftTriggerThreshold,
    RightTriggerThreshold,
    LeftStickRight,
    LeftStickUp,
    LeftStickDown,
    LeftStickLeft,
    RightStickLeft,
    RightStickDown,
    RightStickUp,
    RightStickRight,
    Count UMETA(Hidden)
};

/** Per-controller output limits for accessibility and sensory comfort. */
USTRUCT(BlueprintType)
struct DUALSENSERUNTIME_API FDualSenseAccessibilitySettings
{
    GENERATED_BODY()

    /** Multiplies RGB light intensity. 0 disables the lightbar; 1 keeps requested brightness. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DualSense|Accessibility", meta=(ClampMin="0.0", ClampMax="1.0", ToolTip="Maximum lightbar intensity multiplier. 0 disables RGB output and 1 keeps the requested brightness."))
    float LightBrightnessScale = 1.0f;

    /** Flash requests become a static color when enabled. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DualSense|Accessibility", meta=(ToolTip="Replace flashing lightbar effects with a static color."))
    bool bDisableFlashingLights = false;

    /** Minimum fade duration applied to lightbar changes, even when a node requests an instant transition. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DualSense|Accessibility", meta=(ClampMin="0.0", Units="s", ToolTip="Minimum smooth-transition duration for lightbar changes. 0 allows instant changes."))
    float MinimumLightTransitionDuration = 0.0f;

    /** Multiplies normal motor vibration intensity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DualSense|Accessibility", meta=(ClampMin="0.0", ClampMax="1.0", ToolTip="Rumble intensity multiplier from 0 to 1."))
    float RumbleIntensityScale = 1.0f;

    /** Multiplies configurable adaptive-trigger force/amplitude values. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DualSense|Accessibility", meta=(ClampMin="0.0", ClampMax="1.0", ToolTip="Adaptive-trigger intensity multiplier from 0 to 1. Fixed upstream profiles may only be disabled, not rescaled."))
    float TriggerIntensityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DualSense|Accessibility", meta=(ToolTip="Disable standard motor vibration regardless of gameplay requests."))
    bool bDisableRumble = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DualSense|Accessibility", meta=(ToolTip="Disable adaptive-trigger effects regardless of gameplay requests."))
    bool bDisableAdaptiveTriggers = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DualSense|Accessibility", meta=(ToolTip="Reject audio-haptics output while this accessibility profile is active."))
    bool bDisableAudioHaptics = false;
};

USTRUCT(BlueprintType)
struct DUALSENSERUNTIME_API FDualSenseDeviceInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="DualSense") int32 DeviceId = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly, Category="DualSense") EDualSenseDeviceType DeviceType = EDualSenseDeviceType::Unknown;
    UPROPERTY(BlueprintReadOnly, Category="DualSense") EDualSenseConnectionType ConnectionType = EDualSenseConnectionType::Unknown;
    UPROPERTY(BlueprintReadOnly, Category="DualSense") FString DevicePath;
};

USTRUCT(BlueprintType)
struct DUALSENSERUNTIME_API FDualSenseCapabilities
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bLightbar = false;
    UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bPlayerLed = false;
    UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bMicrophoneLed = false;
    UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bRumble = false;
    UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bAdaptiveTriggers = false;
    UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bMotion = false;
    UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bTouch = false;
    UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bAudioHaptics = false;
    UPROPERTY(BlueprintReadOnly, Category="DualSense") bool bAudioSettings = false;
};

USTRUCT(BlueprintType)
struct DUALSENSERUNTIME_API FDualSenseState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="DualSense|Analog") FVector2D LeftStick = FVector2D::ZeroVector;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Analog") FVector2D RightStick = FVector2D::ZeroVector;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Analog") float LeftTrigger = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Analog") float RightTrigger = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="DualSense|Motion") FVector Gyroscope = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Motion") FVector Accelerometer = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Motion") FVector Gravity = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Motion") FVector Tilt = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category="DualSense|Touch") int32 TouchId = 0;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Touch") int32 TouchFingerCount = 0;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Touch") uint8 TouchDirectionRaw = 0;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Touch") bool bIsTouching = false;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Touch") FVector2D TouchRadius = FVector2D::ZeroVector;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Touch") FVector2D TouchPosition = FVector2D::ZeroVector;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Touch") FVector2D TouchRelative = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category="DualSense|Buttons") TArray<EDualSenseButton> PressedButtons;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Status", meta=(ClampMin="0.0", ClampMax="1.0")) float BatteryLevel = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="DualSense|Status") bool bHasPhoneConnected = false;
};
