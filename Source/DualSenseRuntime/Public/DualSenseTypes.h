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
    PaddleRight
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
