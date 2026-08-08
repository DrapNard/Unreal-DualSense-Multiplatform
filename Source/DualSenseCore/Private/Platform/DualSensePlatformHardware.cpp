// SPDX-License-Identifier: MPL-2.0
#include "DualSensePlatformHardware.h"

#include "GCore/Interfaces/IPlatformHardware.h"
#include "GCore/Types/Structs/Context/DeviceContext.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
    // GamepadCore currently asks the platform to open every path returned by its
    // periodic detection pass, including paths that already own a live handle.
    // Keep that upstream behavior untouched and reject duplicate opens here.
    class TrackedPlatformHardware final : public IPlatformHardware
    {
    public:
        explicit TrackedPlatformHardware(std::unique_ptr<IPlatformHardware> InInner)
            : Inner(std::move(InInner))
        {
        }

        void Read(FDeviceContext* Context) override
        {
            Inner->Read(Context);
        }

        void Write(FDeviceContext* Context) override
        {
            Inner->Write(Context);
        }

        void Detect(std::vector<FDeviceContext>& Devices) override
        {
            Inner->Detect(Devices);
        }

        bool CreateHandle(FDeviceContext* Context) override
        {
            if (!Context || Context->Path.empty())
            {
                return false;
            }

            std::lock_guard<std::mutex> Lock(ActivePathsMutex);
            if (ActivePaths.contains(Context->Path))
            {
                return false;
            }

            if (!Inner->CreateHandle(Context))
            {
                return false;
            }

            ActivePaths.insert(Context->Path);
            return true;
        }

        void InvalidateHandle(FDeviceContext* Context) override
        {
            const std::string Path = Context ? Context->Path : std::string{};
            Inner->InvalidateHandle(Context);

            if (!Path.empty())
            {
                std::lock_guard<std::mutex> Lock(ActivePathsMutex);
                ActivePaths.erase(Path);
            }
        }

        void ProcessAudioHaptic(FDeviceContext* Context) override
        {
            Inner->ProcessAudioHaptic(Context);
        }

    private:
        std::unique_ptr<IPlatformHardware> Inner;
        std::unordered_set<std::string> ActivePaths;
        std::mutex ActivePathsMutex;
    };
}

namespace DualSense::Platform
{
    std::unique_ptr<IPlatformHardware> CreateHardware()
    {
        std::unique_ptr<IPlatformHardware> Native = CreateNativeHardware();
        if (!Native)
        {
            return nullptr;
        }
        return std::make_unique<TrackedPlatformHardware>(std::move(Native));
    }
}
