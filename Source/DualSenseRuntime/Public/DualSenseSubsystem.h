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

    UPROPERTY(BlueprintAssignable, Category="DualSense|Events") FDualSenseDeviceConnectedSignature OnDeviceConnected;
    UPROPERTY(BlueprintAssignable, Category="DualSense|Events") FDualSenseDeviceDisconnectedSignature OnDeviceDisconnected;
    UPROPERTY(BlueprintAssignable, Category="DualSense|Events") FDualSenseButtonSignature OnButtonPressed;
    UPROPERTY(BlueprintAssignable, Category="DualSense|Events") FDualSenseButtonSignature OnButtonReleased;

    UFUNCTION(BlueprintCallable, Category="DualSense|Devices") void RequestImmediateDetection();
    UFUNCTION(BlueprintPure, Category="DualSense|Devices") TArray<int32> GetConnectedDeviceIds() const;
    UFUNCTION(BlueprintPure, Category="DualSense|Devices") bool IsDeviceConnected(int32 DeviceId) const;
    UFUNCTION(BlueprintPure, Category="DualSense|Devices") bool GetDeviceInfo(int32 DeviceId, FDualSenseDeviceInfo& DeviceInfo) const;
    UFUNCTION(BlueprintPure, Category="DualSense|Devices") bool GetState(int32 DeviceId, FDualSenseState& State) const;
    UFUNCTION(BlueprintPure, Category="DualSense|Devices") FDualSenseCapabilities GetCapabilities(int32 DeviceId) const;
    UFUNCTION(BlueprintPure, Category="DualSense|Buttons") bool IsButtonDown(int32 DeviceId, EDualSenseButton Button) const;

    UFUNCTION(BlueprintCallable, Category="DualSense|Output") bool ApplyOutput(int32 DeviceId);
    UFUNCTION(BlueprintCallable, Category="DualSense|Output", meta=(ClampMin="0", ClampMax="255")) bool SetVibration(int32 DeviceId, int32 LeftMotor, int32 RightMotor, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Lights") bool SetLightbar(int32 DeviceId, FLinearColor Color, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Lights") bool SetLightbarFlash(int32 DeviceId, FLinearColor Color, float BrightnessTime, float ToggleTime, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Lights", meta=(ClampMin="0", ClampMax="255")) bool SetPlayerLed(int32 DeviceId, EDualSensePlayerLed Led, int32 Brightness = 255, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Lights") bool SetMicrophoneLed(int32 DeviceId, EDualSenseMicrophoneLed Led, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Lights") bool ResetLights(int32 DeviceId, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers") bool StopTrigger(int32 DeviceId, EDualSenseHand Hand, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers") bool SetGameCubeTrigger(int32 DeviceId, EDualSenseHand Hand, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers") bool SetResistanceTrigger(int32 DeviceId, int32 StartZone, int32 Strength, EDualSenseHand Hand, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers") bool SetBowTrigger(int32 DeviceId, int32 StartZone, int32 SnapBack, EDualSenseHand Hand, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers") bool SetGallopingTrigger(int32 DeviceId, int32 StartPosition, int32 EndPosition, int32 FirstFoot, int32 SecondFoot, int32 Frequency, EDualSenseHand Hand, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers") bool SetWeaponTrigger(int32 DeviceId, int32 StartZone, int32 Amplitude, int32 Behavior, int32 Trigger, EDualSenseHand Hand, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers") bool SetMachineGunTrigger(int32 DeviceId, int32 StartZone, int32 Behavior, int32 Amplitude, int32 Frequency, EDualSenseHand Hand, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers") bool SetMachineTrigger(int32 DeviceId, int32 StartZone, int32 BehaviorFlag, int32 Force, int32 Amplitude, int32 Period, int32 Frequency, EDualSenseHand Hand, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Triggers") bool SetCustomTrigger(int32 DeviceId, EDualSenseHand Hand, const TArray<uint8>& TenBytes, bool bApplyImmediately = true);

    UFUNCTION(BlueprintCallable, Category="DualSense|Sensors") bool EnableMotion(int32 DeviceId, bool bEnabled);
    UFUNCTION(BlueprintCallable, Category="DualSense|Sensors") bool ResetGyroOrientation(int32 DeviceId);
    UFUNCTION(BlueprintCallable, Category="DualSense|Touch") bool EnableTouch(int32 DeviceId, bool bEnabled);
    UFUNCTION(BlueprintCallable, Category="DualSense|Touch") bool EnableGesture(int32 DeviceId, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category="DualSense|Settings") bool ConfigureDualSense(int32 DeviceId, bool bMicEnabled, bool bHeadsetEnabled, bool bSpeakerEnabled, int32 MicVolume, int32 AudioVolume, int32 RumbleMode, int32 RumbleReduce, int32 TriggerReduce, bool bApplyImmediately = true);
    UFUNCTION(BlueprintCallable, Category="DualSense|Audio Haptics") bool SendAudioHapticsBytes(int32 DeviceId, const TArray<uint8>& AudioData);
    UFUNCTION(BlueprintCallable, Category="DualSense|Audio Haptics") bool SendAudioHapticsFloats(int32 DeviceId, const TArray<float>& AudioData);
    UFUNCTION(BlueprintCallable, Category="DualSense|Audio Haptics") bool SendAudioAndHapticsBytes(int32 DeviceId, const TArray<uint8>& HapticsData, const TArray<uint8>& AudioData);

private:
    bool bInitialized = false;
    TSet<int32> KnownDeviceIds;
    TMap<int32, FDualSenseState> PreviousStates;
    void PollEvents();
};
