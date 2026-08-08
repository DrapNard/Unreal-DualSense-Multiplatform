// SPDX-License-Identifier: MPL-2.0
#if defined(__APPLE__)

#include "DualSensePlatformHardware.h"
#include "GCore/Interfaces/IPlatformHardware.h"
#include "GCore/Types/Structs/Context/DeviceContext.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDLib.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace
{
    constexpr long SonyVendorId = 0x054C;
    constexpr long DualSensePid = 0x0CE6;
    constexpr long DualSenseEdgePid = 0x0DF2;
    constexpr long DualShock4Pid1 = 0x05C4;
    constexpr long DualShock4Pid2 = 0x09CC;

    struct MacHandle
    {
        IOHIDDeviceRef Device = nullptr;
    };

    long NumberProperty(IOHIDDeviceRef Device, CFStringRef Key)
    {
        CFTypeRef Value = IOHIDDeviceGetProperty(Device, Key);
        if (!Value || CFGetTypeID(Value) != CFNumberGetTypeID()) return -1;
        long Result = -1;
        CFNumberGetValue(static_cast<CFNumberRef>(Value), kCFNumberLongType, &Result);
        return Result;
    }

    bool StringPropertyEquals(IOHIDDeviceRef Device, CFStringRef Key, CFStringRef Expected)
    {
        CFTypeRef Value = IOHIDDeviceGetProperty(Device, Key);
        return Value && CFGetTypeID(Value) == CFStringGetTypeID() && CFStringCompare(static_cast<CFStringRef>(Value), Expected, kCFCompareCaseInsensitive) == kCFCompareEqualTo;
    }

    EDSDeviceType DeviceTypeFromProduct(long Product)
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

    EDSDeviceConnection ConnectionTypeFor(IOHIDDeviceRef Device)
    {
        if (StringPropertyEquals(Device, CFSTR(kIOHIDTransportKey), CFSTR("Bluetooth"))) return EDSDeviceConnection::Bluetooth;
        if (StringPropertyEquals(Device, CFSTR(kIOHIDTransportKey), CFSTR("USB"))) return EDSDeviceConnection::Usb;
        return EDSDeviceConnection::Unrecognized;
    }

    std::uint64_t RegistryId(IOHIDDeviceRef Device)
    {
        io_service_t Service = IOHIDDeviceGetService(Device);
        std::uint64_t Id = 0;
        if (Service) IORegistryEntryGetRegistryEntryID(Service, &Id);
        return Id;
    }

    std::string PathFor(IOHIDDeviceRef Device)
    {
        return "iokit://" + std::to_string(RegistryId(Device));
    }

    std::uint64_t ParsePath(const std::string& Path)
    {
        constexpr const char* Prefix = "iokit://";
        if (Path.rfind(Prefix, 0) != 0) return 0;
        try { return static_cast<std::uint64_t>(std::stoull(Path.substr(std::strlen(Prefix)))); }
        catch (...) { return 0; }
    }

    std::uint8_t InputReportId(const FDeviceContext& Context)
    {
        if (Context.ConnectionType == EDSDeviceConnection::Bluetooth)
        {
            return Context.DeviceType == EDSDeviceType::DualShock4 ? 0x11 : 0x31;
        }
        return 0x01;
    }

    std::size_t OutputSize(const FDeviceContext& Context)
    {
        if (Context.ConnectionType == EDSDeviceConnection::Bluetooth) return 78;
        if (Context.DeviceType == EDSDeviceType::DualShock4) return 32;
        return 48;
    }

    class MacHardware final : public IPlatformHardware
    {
    public:
        void Read(FDeviceContext* Context) override
        {
            MacHandle* State = GetHandle(Context);
            if (!State || !State->Device || !Context) return;

            const std::uint8_t ReportId = InputReportId(*Context);
            std::array<std::uint8_t, 77> Payload{};
            CFIndex Length = static_cast<CFIndex>(Payload.size());
            const IOReturn Result = IOHIDDeviceGetReport(State->Device, kIOHIDReportTypeInput, ReportId, Payload.data(), &Length);
            if (Result == kIOReturnSuccess)
            {
                std::memset(Context->Buffer, 0, sizeof(Context->Buffer));
                Context->Buffer[0] = ReportId;
                const std::size_t CopyLength = std::min<std::size_t>(static_cast<std::size_t>(Length), sizeof(Context->Buffer) - 1);
                std::memcpy(&Context->Buffer[1], Payload.data(), CopyLength);
                if (Context->DeviceType == EDSDeviceType::DualShock4)
                {
                    std::memset(Context->BufferDS4, 0, sizeof(Context->BufferDS4));
                    Context->BufferDS4[0] = ReportId;
                    std::memcpy(&Context->BufferDS4[1], Payload.data(), std::min<std::size_t>(static_cast<std::size_t>(Length), sizeof(Context->BufferDS4) - 1));
                }
                Context->IsConnected = true;
            }
            else if (Result == kIOReturnNoDevice || Result == kIOReturnNotOpen)
            {
                Context->IsConnected = false;
            }
        }

        void Write(FDeviceContext* Context) override
        {
            MacHandle* State = GetHandle(Context);
            if (!State || !State->Device || !Context) return;
            unsigned char* Buffer = Context->GetRawOutputBuffer();
            const std::size_t Length = OutputSize(*Context);
            const std::uint8_t ReportId = Buffer[0];
            const IOReturn Result = IOHIDDeviceSetReport(State->Device, kIOHIDReportTypeOutput, ReportId, &Buffer[1], static_cast<CFIndex>(Length - 1));
            if (Result == kIOReturnNoDevice || Result == kIOReturnNotOpen) Context->IsConnected = false;
        }

        void Detect(std::vector<FDeviceContext>& Devices) override
        {
            IOHIDManagerRef Manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
            if (!Manager) return;
            IOHIDManagerSetDeviceMatching(Manager, nullptr);
            if (IOHIDManagerOpen(Manager, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
            {
                CFRelease(Manager);
                return;
            }

            CFSetRef DeviceSet = IOHIDManagerCopyDevices(Manager);
            if (DeviceSet)
            {
                const CFIndex Count = CFSetGetCount(DeviceSet);
                std::vector<const void*> Values(static_cast<std::size_t>(Count));
                CFSetGetValues(DeviceSet, Values.data());
                for (const void* Value : Values)
                {
                    IOHIDDeviceRef Device = (IOHIDDeviceRef)Value;
                    const long Vendor = NumberProperty(Device, CFSTR(kIOHIDVendorIDKey));
                    const long Product = NumberProperty(Device, CFSTR(kIOHIDProductIDKey));
                    if (Vendor != SonyVendorId) continue;
                    const EDSDeviceType Type = DeviceTypeFromProduct(Product);
                    if (Type == EDSDeviceType::NotFound) continue;

                    const long UsagePage = NumberProperty(Device, CFSTR(kIOHIDPrimaryUsagePageKey));
                    const long Usage = NumberProperty(Device, CFSTR(kIOHIDPrimaryUsageKey));
                    if (UsagePage != 0x01 || (Usage != 0x04 && Usage != 0x05)) continue;

                    FDeviceContext Context;
                    Context.Path = PathFor(Device);
                    Context.DeviceType = Type;
                    Context.ConnectionType = ConnectionTypeFor(Device);
                    Devices.push_back(Context);
                }
                CFRelease(DeviceSet);
            }
            IOHIDManagerClose(Manager, kIOHIDOptionsTypeNone);
            CFRelease(Manager);
        }

        bool CreateHandle(FDeviceContext* Context) override
        {
            if (!Context) return false;
            const std::uint64_t WantedId = ParsePath(Context->Path);
            if (WantedId == 0) return false;

            IOHIDManagerRef Manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
            if (!Manager) return false;
            IOHIDManagerSetDeviceMatching(Manager, nullptr);
            if (IOHIDManagerOpen(Manager, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
            {
                CFRelease(Manager);
                return false;
            }

            IOHIDDeviceRef Match = nullptr;
            CFSetRef DeviceSet = IOHIDManagerCopyDevices(Manager);
            if (DeviceSet)
            {
                const CFIndex Count = CFSetGetCount(DeviceSet);
                std::vector<const void*> Values(static_cast<std::size_t>(Count));
                CFSetGetValues(DeviceSet, Values.data());
                for (const void* Value : Values)
                {
                    IOHIDDeviceRef Candidate = (IOHIDDeviceRef)Value;
                    if (RegistryId(Candidate) == WantedId)
                    {
                        Match = Candidate;
                        CFRetain(Match);
                        break;
                    }
                }
                CFRelease(DeviceSet);
            }
            IOHIDManagerClose(Manager, kIOHIDOptionsTypeNone);
            CFRelease(Manager);
            if (!Match) return false;

            // Do not seize the device: the plugin should coexist with Unreal's
            // regular input stack and other user-space input consumers.
            if (IOHIDDeviceOpen(Match, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
            {
                CFRelease(Match);
                return false;
            }

            auto* State = new MacHandle();
            State->Device = Match;
            Context->Handle = State;
            Context->IsConnected = true;
            return true;
        }

        void InvalidateHandle(FDeviceContext* Context) override
        {
            MacHandle* State = GetHandle(Context);
            if (State)
            {
                if (State->Device)
                {
                    IOHIDDeviceClose(State->Device, kIOHIDOptionsTypeNone);
                    CFRelease(State->Device);
                }
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
            MacHandle* State = GetHandle(Context);
            if (!State || !State->Device || !Context) return;
            const std::uint8_t ReportId = Context->BufferHapitcs[0];
            const IOReturn Result = IOHIDDeviceSetReport(State->Device, kIOHIDReportTypeOutput, ReportId, &Context->BufferHapitcs[1], static_cast<CFIndex>(sizeof(Context->BufferHapitcs) - 1));
            if (Result == kIOReturnNoDevice || Result == kIOReturnNotOpen) Context->IsConnected = false;
        }

    private:
        static MacHandle* GetHandle(FDeviceContext* Context)
        {
            return Context ? static_cast<MacHandle*>(Context->Handle) : nullptr;
        }
    };
}

namespace DualSense::Platform
{
    std::unique_ptr<IPlatformHardware> CreateHardware()
    {
        return std::make_unique<MacHardware>();
    }
}

#endif
