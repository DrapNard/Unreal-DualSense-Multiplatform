// SPDX-License-Identifier: MPL-2.0
#if defined(__linux__)

#include "DualSensePlatformHardware.h"
#include "GCore/Interfaces/IPlatformHardware.h"
#include "GCore/Types/Structs/Context/DeviceContext.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <linux/hidraw.h>
#include <linux/input.h>
#include <memory>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace
{
    constexpr unsigned short SonyVendorId = 0x054C;
    constexpr unsigned short DualSensePid = 0x0CE6;
    constexpr unsigned short DualSenseEdgePid = 0x0DF2;
    constexpr unsigned short DualShock4Pid1 = 0x05C4;
    constexpr unsigned short DualShock4Pid2 = 0x09CC;

    struct LinuxHandle
    {
        int Fd = -1;
    };

    EDSDeviceType DeviceTypeFromProduct(unsigned short Product)
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

    EDSDeviceConnection ConnectionFromBus(unsigned short Bus)
    {
        if (Bus == BUS_USB) return EDSDeviceConnection::Usb;
        if (Bus == BUS_BLUETOOTH) return EDSDeviceConnection::Bluetooth;
        return EDSDeviceConnection::Unrecognized;
    }

    std::size_t OutputSize(const FDeviceContext& Context)
    {
        if (Context.ConnectionType == EDSDeviceConnection::Bluetooth) return 78;
        if (Context.DeviceType == EDSDeviceType::DualShock4) return 32;
        return 48;
    }

    class LinuxHardware final : public IPlatformHardware
    {
    public:
        void Read(FDeviceContext* Context) override
        {
            LinuxHandle* Handle = GetHandle(Context);
            if (!Handle) return;

            unsigned char Temp[78] = {};
            const ssize_t ReadCount = ::read(Handle->Fd, Temp, sizeof(Temp));
            if (ReadCount > 0)
            {
                std::memset(Context->Buffer, 0, sizeof(Context->Buffer));
                std::memcpy(Context->Buffer, Temp, std::min<std::size_t>(static_cast<std::size_t>(ReadCount), sizeof(Context->Buffer)));
                if (Context->DeviceType == EDSDeviceType::DualShock4)
                {
                    std::memset(Context->BufferDS4, 0, sizeof(Context->BufferDS4));
                    std::memcpy(Context->BufferDS4, Temp, std::min<std::size_t>(static_cast<std::size_t>(ReadCount), sizeof(Context->BufferDS4)));
                }
                Context->IsConnected = true;
                return;
            }

            if (ReadCount < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
            {
                Context->IsConnected = false;
            }
        }

        void Write(FDeviceContext* Context) override
        {
            LinuxHandle* Handle = GetHandle(Context);
            if (!Handle) return;
            const unsigned char* Buffer = Context->GetRawOutputBuffer();
            const std::size_t Length = OutputSize(*Context);
            const ssize_t Result = ::write(Handle->Fd, Buffer, Length);
            if (Result < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
            {
                Context->IsConnected = false;
            }
        }

        void Detect(std::vector<FDeviceContext>& Devices) override
        {
            namespace fs = std::filesystem;
            const fs::path Root("/sys/class/hidraw");
            std::error_code Error;
            if (!fs::exists(Root, Error)) return;

            for (const fs::directory_entry& Entry : fs::directory_iterator(Root, Error))
            {
                if (Error) break;
                const std::string Name = Entry.path().filename().string();
                if (Name.rfind("hidraw", 0) != 0) continue;

                const std::string DevicePath = "/dev/" + Name;
                const int Fd = ::open(DevicePath.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
                if (Fd < 0) continue;

                hidraw_devinfo Info{};
                const int IoctlResult = ::ioctl(Fd, HIDIOCGRAWINFO, &Info);
                ::close(Fd);
                if (IoctlResult < 0 || Info.vendor != SonyVendorId) continue;

                const EDSDeviceType Type = DeviceTypeFromProduct(Info.product);
                if (Type == EDSDeviceType::NotFound) continue;

                FDeviceContext Context;
                Context.Path = DevicePath;
                Context.DeviceType = Type;
                Context.ConnectionType = ConnectionFromBus(Info.bustype);
                Context.IsConnected = false;
                Devices.push_back(Context);
            }
        }

        bool CreateHandle(FDeviceContext* Context) override
        {
            if (!Context || Context->Path.empty()) return false;
            const int Fd = ::open(Context->Path.c_str(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
            if (Fd < 0) return false;

            auto* State = new LinuxHandle();
            State->Fd = Fd;
            Context->Handle = State;
            Context->IsConnected = true;
            return true;
        }

        void InvalidateHandle(FDeviceContext* Context) override
        {
            LinuxHandle* Handle = GetHandle(Context);
            if (Handle)
            {
                if (Handle->Fd >= 0) ::close(Handle->Fd);
                delete Handle;
            }
            if (Context)
            {
                Context->Handle = INVALID_PLATFORM_HANDLE;
                Context->IsConnected = false;
            }
        }

        void ProcessAudioHaptic(FDeviceContext* Context) override
        {
            LinuxHandle* Handle = GetHandle(Context);
            if (!Handle || !Context) return;
            const ssize_t Result = ::write(Handle->Fd, Context->BufferHapitcs, sizeof(Context->BufferHapitcs));
            if (Result < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
            {
                Context->IsConnected = false;
            }
        }

    private:
        static LinuxHandle* GetHandle(FDeviceContext* Context)
        {
            return Context ? static_cast<LinuxHandle*>(Context->Handle) : nullptr;
        }
    };
}

namespace DualSense::Platform
{
    std::unique_ptr<IPlatformHardware> CreateHardware()
    {
        return std::make_unique<LinuxHardware>();
    }
}

#endif
