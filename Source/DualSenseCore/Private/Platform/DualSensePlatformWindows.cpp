// SPDX-License-Identifier: MPL-2.0
#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <hidpi.h>

#include "DualSensePlatformHardware.h"
#include "GCore/Interfaces/IPlatformHardware.h"
#include "GCore/Types/Structs/Context/DeviceContext.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace
{
    constexpr USHORT SonyVendorId = 0x054C;
    constexpr USHORT DualSensePid = 0x0CE6;
    constexpr USHORT DualSenseEdgePid = 0x0DF2;
    constexpr USHORT DualShock4Pid1 = 0x05C4;
    constexpr USHORT DualShock4Pid2 = 0x09CC;

    struct WindowsHandle
    {
        HANDLE ReadHandle = INVALID_HANDLE_VALUE;
        HANDLE WriteHandle = INVALID_HANDLE_VALUE;
        HANDLE ReadEvent = nullptr;
        OVERLAPPED ReadOverlapped{};
        bool ReadPending = false;
        USHORT InputLength = 78;
        std::array<unsigned char, 78> ReadBuffer{};
    };

    EDSDeviceType DeviceTypeFromProduct(USHORT Product)
    {
        switch (Product)
        {
            case DualSensePid: return EDSDeviceType::DualSense;
            case DualSenseEdgePid: return EDSDeviceType::DualSenseEdge;
            case DualShock4Pid1:
            case DualShock4Pid2: return EDSDeviceType::DualShock4;
            default: return EDSDeviceType::NotFound;
        }
    }

    std::string WideToUtf8(const std::wstring& Value)
    {
        if (Value.empty()) return {};
        const int Required = WideCharToMultiByte(CP_UTF8, 0, Value.c_str(), static_cast<int>(Value.size()), nullptr, 0, nullptr, nullptr);
        std::string Result(static_cast<std::size_t>(Required), '\0');
        WideCharToMultiByte(CP_UTF8, 0, Value.c_str(), static_cast<int>(Value.size()), Result.data(), Required, nullptr, nullptr);
        return Result;
    }

    std::wstring Utf8ToWide(const std::string& Value)
    {
        if (Value.empty()) return {};
        const int Required = MultiByteToWideChar(CP_UTF8, 0, Value.c_str(), static_cast<int>(Value.size()), nullptr, 0);
        std::wstring Result(static_cast<std::size_t>(Required), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, Value.c_str(), static_cast<int>(Value.size()), Result.data(), Required);
        return Result;
    }

    bool GetCaps(HANDLE Handle, HIDP_CAPS& OutCaps)
    {
        PHIDP_PREPARSED_DATA Data = nullptr;
        if (!HidD_GetPreparsedData(Handle, &Data)) return false;
        const NTSTATUS Status = HidP_GetCaps(Data, &OutCaps);
        HidD_FreePreparsedData(Data);
        return Status == HIDP_STATUS_SUCCESS;
    }

    EDSDeviceConnection ConnectionFromCaps(const HIDP_CAPS& Caps)
    {
        return Caps.InputReportByteLength > 64 ? EDSDeviceConnection::Bluetooth : EDSDeviceConnection::Usb;
    }

    std::size_t OutputSize(const FDeviceContext& Context)
    {
        if (Context.ConnectionType == EDSDeviceConnection::Bluetooth) return 78;
        if (Context.DeviceType == EDSDeviceType::DualShock4) return 32;
        return 48;
    }

    void CopyReadBuffer(FDeviceContext* Context, const unsigned char* Data, DWORD Length)
    {
        if (!Context || !Data || Length == 0) return;
        std::memset(Context->Buffer, 0, sizeof(Context->Buffer));
        std::memcpy(Context->Buffer, Data, std::min<std::size_t>(Length, sizeof(Context->Buffer)));
        if (Context->DeviceType == EDSDeviceType::DualShock4)
        {
            std::memset(Context->BufferDS4, 0, sizeof(Context->BufferDS4));
            std::memcpy(Context->BufferDS4, Data, std::min<std::size_t>(Length, sizeof(Context->BufferDS4)));
        }
    }

    class WindowsHardware final : public IPlatformHardware
    {
    public:
        void Read(FDeviceContext* Context) override
        {
            WindowsHandle* State = GetHandle(Context);
            if (!State || State->ReadHandle == INVALID_HANDLE_VALUE) return;

            if (State->ReadPending)
            {
                DWORD BytesRead = 0;
                if (GetOverlappedResult(State->ReadHandle, &State->ReadOverlapped, &BytesRead, FALSE))
                {
                    State->ReadPending = false;
                    CopyReadBuffer(Context, State->ReadBuffer.data(), BytesRead);
                    Context->IsConnected = true;
                }
                else
                {
                    const DWORD Error = GetLastError();
                    if (Error == ERROR_IO_INCOMPLETE) return;
                    State->ReadPending = false;
                    if (Error == ERROR_DEVICE_NOT_CONNECTED || Error == ERROR_INVALID_HANDLE)
                    {
                        Context->IsConnected = false;
                        return;
                    }
                }
            }

            ResetEvent(State->ReadEvent);
            std::memset(&State->ReadOverlapped, 0, sizeof(State->ReadOverlapped));
            State->ReadOverlapped.hEvent = State->ReadEvent;
            DWORD BytesRead = 0;
            const DWORD ReadLength = std::min<DWORD>(State->InputLength, static_cast<DWORD>(State->ReadBuffer.size()));
            if (ReadFile(State->ReadHandle, State->ReadBuffer.data(), ReadLength, &BytesRead, &State->ReadOverlapped))
            {
                CopyReadBuffer(Context, State->ReadBuffer.data(), BytesRead);
                Context->IsConnected = true;
            }
            else
            {
                const DWORD Error = GetLastError();
                if (Error == ERROR_IO_PENDING)
                {
                    State->ReadPending = true;
                }
                else if (Error == ERROR_DEVICE_NOT_CONNECTED || Error == ERROR_INVALID_HANDLE)
                {
                    Context->IsConnected = false;
                }
            }
        }

        void Write(FDeviceContext* Context) override
        {
            WindowsHandle* State = GetHandle(Context);
            if (!State || State->WriteHandle == INVALID_HANDLE_VALUE) return;
            DWORD Written = 0;
            const DWORD Length = static_cast<DWORD>(OutputSize(*Context));
            if (!WriteFile(State->WriteHandle, Context->GetRawOutputBuffer(), Length, &Written, nullptr))
            {
                const DWORD Error = GetLastError();
                if (Error == ERROR_DEVICE_NOT_CONNECTED || Error == ERROR_INVALID_HANDLE)
                {
                    Context->IsConnected = false;
                }
            }
        }

        void Detect(std::vector<FDeviceContext>& Devices) override
        {
            GUID HidGuid{};
            HidD_GetHidGuid(&HidGuid);
            HDEVINFO DeviceInfoSet = SetupDiGetClassDevsW(&HidGuid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
            if (DeviceInfoSet == INVALID_HANDLE_VALUE) return;

            SP_DEVICE_INTERFACE_DATA InterfaceData{};
            InterfaceData.cbSize = sizeof(InterfaceData);
            for (DWORD Index = 0; SetupDiEnumDeviceInterfaces(DeviceInfoSet, nullptr, &HidGuid, Index, &InterfaceData); ++Index)
            {
                DWORD Required = 0;
                SetupDiGetDeviceInterfaceDetailW(DeviceInfoSet, &InterfaceData, nullptr, 0, &Required, nullptr);
                if (Required == 0) continue;
                std::vector<unsigned char> DetailStorage(Required);
                auto* Detail = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(DetailStorage.data());
                Detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
                if (!SetupDiGetDeviceInterfaceDetailW(DeviceInfoSet, &InterfaceData, Detail, Required, nullptr, nullptr)) continue;

                HANDLE Probe = CreateFileW(Detail->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
                if (Probe == INVALID_HANDLE_VALUE) continue;

                HIDD_ATTRIBUTES Attributes{};
                Attributes.Size = sizeof(Attributes);
                HIDP_CAPS Caps{};
                const bool Valid = HidD_GetAttributes(Probe, &Attributes) && GetCaps(Probe, Caps);
                CloseHandle(Probe);
                if (!Valid || Attributes.VendorID != SonyVendorId) continue;

                const EDSDeviceType Type = DeviceTypeFromProduct(Attributes.ProductID);
                if (Type == EDSDeviceType::NotFound) continue;
                if (Caps.UsagePage != 0x01 || (Caps.Usage != 0x04 && Caps.Usage != 0x05)) continue;

                FDeviceContext Context;
                Context.Path = WideToUtf8(Detail->DevicePath);
                Context.DeviceType = Type;
                Context.ConnectionType = ConnectionFromCaps(Caps);
                Devices.push_back(Context);
            }
            SetupDiDestroyDeviceInfoList(DeviceInfoSet);
        }

        bool CreateHandle(FDeviceContext* Context) override
        {
            if (!Context || Context->Path.empty()) return false;
            const std::wstring Path = Utf8ToWide(Context->Path);
            HANDLE ReadHandle = CreateFileW(Path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
            if (ReadHandle == INVALID_HANDLE_VALUE) return false;

            HIDP_CAPS Caps{};
            if (!GetCaps(ReadHandle, Caps))
            {
                CloseHandle(ReadHandle);
                return false;
            }

            HANDLE WriteHandle = CreateFileW(Path.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
            auto* State = new WindowsHandle();
            State->ReadHandle = ReadHandle;
            State->WriteHandle = WriteHandle;
            State->InputLength = Caps.InputReportByteLength;
            State->ReadEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
            if (!State->ReadEvent)
            {
                if (WriteHandle != INVALID_HANDLE_VALUE) CloseHandle(WriteHandle);
                CloseHandle(ReadHandle);
                delete State;
                return false;
            }
            State->ReadOverlapped.hEvent = State->ReadEvent;
            Context->Handle = State;
            Context->IsConnected = true;
            return true;
        }

        void InvalidateHandle(FDeviceContext* Context) override
        {
            WindowsHandle* State = GetHandle(Context);
            if (State)
            {
                if (State->ReadPending && State->ReadHandle != INVALID_HANDLE_VALUE) CancelIoEx(State->ReadHandle, &State->ReadOverlapped);
                if (State->ReadEvent) CloseHandle(State->ReadEvent);
                if (State->ReadHandle != INVALID_HANDLE_VALUE) CloseHandle(State->ReadHandle);
                if (State->WriteHandle != INVALID_HANDLE_VALUE) CloseHandle(State->WriteHandle);
                delete State;
            }
            if (Context)
            {
                Context->Handle = INVALID_PLATFORM_HANDLE;
                Context->IsConnected = false;
            }
        }

        void ProcessAudioHaptic(FDeviceContext* Context) override
        {
            WindowsHandle* State = GetHandle(Context);
            if (!State || State->WriteHandle == INVALID_HANDLE_VALUE || !Context) return;
            DWORD Written = 0;
            if (!WriteFile(State->WriteHandle, Context->BufferHapitcs, static_cast<DWORD>(sizeof(Context->BufferHapitcs)), &Written, nullptr))
            {
                const DWORD Error = GetLastError();
                if (Error == ERROR_DEVICE_NOT_CONNECTED || Error == ERROR_INVALID_HANDLE) Context->IsConnected = false;
            }
        }

    private:
        static WindowsHandle* GetHandle(FDeviceContext* Context)
        {
            return Context ? static_cast<WindowsHandle*>(Context->Handle) : nullptr;
        }
    };
}

namespace DualSense::Platform
{
    std::unique_ptr<IPlatformHardware> CreateHardware()
    {
        return std::make_unique<WindowsHardware>();
    }
}

#endif
