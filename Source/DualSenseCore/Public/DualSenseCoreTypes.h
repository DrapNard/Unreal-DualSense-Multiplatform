// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace DualSense
{
    using DeviceId = std::uint32_t;
    inline constexpr DeviceId InvalidDeviceId = 0xFFFFFFFFu;

    enum class DeviceType : std::uint8_t
    {
        DualSense,
        DualSenseEdge,
        DualShock4,
        Unknown
    };

    enum class ConnectionType : std::uint8_t
    {
        USB,
        Bluetooth,
        Unknown
    };

    enum class Hand : std::uint8_t
    {
        Left,
        Right,
        Both
    };

    enum class PlayerLed : std::uint8_t
    {
        Off,
        One,
        Two,
        Three,
        All
    };

    enum class MicrophoneLed : std::uint8_t
    {
        On,
        Off,
        Pulse
    };

    enum class Button : std::uint8_t
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
        Count
    };

    struct Vector2
    {
        float X = 0.0f;
        float Y = 0.0f;
    };

    struct Vector3
    {
        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;
    };

    struct Color
    {
        std::uint8_t R = 0;
        std::uint8_t G = 0;
        std::uint8_t B = 0;
    };

    struct DeviceInfo
    {
        DeviceId Id = InvalidDeviceId;
        DeviceType Type = DeviceType::Unknown;
        ConnectionType Connection = ConnectionType::Unknown;
        std::string Path;
    };

    struct State
    {
        Vector2 LeftStick;
        Vector2 RightStick;
        float LeftTrigger = 0.0f;
        float RightTrigger = 0.0f;

        Vector3 Gyroscope;
        Vector3 Accelerometer;
        Vector3 Gravity;
        Vector3 Tilt;

        std::int32_t TouchId = 0;
        std::int32_t TouchFingerCount = 0;
        std::uint8_t TouchDirectionRaw = 0;
        bool IsTouching = false;
        Vector2 TouchRadius;
        Vector2 TouchPosition;
        Vector2 TouchRelative;

        std::array<bool, static_cast<std::size_t>(Button::Count)> Buttons{};
        float BatteryLevel = 0.0f;
        bool HasPhoneConnected = false;
    };

    struct Capabilities
    {
        bool Lightbar = false;
        bool PlayerLed = false;
        bool MicrophoneLed = false;
        bool Rumble = false;
        bool AdaptiveTriggers = false;
        bool Motion = false;
        bool Touch = false;
        bool AudioHaptics = false;
        bool AudioSettings = false;
    };
}
