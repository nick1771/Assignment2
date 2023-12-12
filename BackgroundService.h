#include <string_view>
#include <vector>

#include "InterfaceAddress.h"

namespace BackgroundService::Messages {

    inline constexpr std::string_view EXIT = "Exit";
    inline constexpr std::string_view LIST_DEVICES = "ListDevices";
}

namespace BackgroundService {

    struct DeviceInformation {
        std::string ip{};
        std::string mac{};
        float timeToLive = 30.f;

        inline bool operator==(const DeviceInformation& other) const {
            return other.ip == ip && other.mac == mac;
        }

        inline bool operator==(const InterfaceAddress::Interface& interface) const {
            return interface.ip == ip && interface.mac == mac;
        }
    };

    std::vector<DeviceInformation> parseBroadcastDeviceList(std::string_view);

    void updateDeviceInformation(
        std::vector<BackgroundService::DeviceInformation>&, 
        const std::vector<BackgroundService::DeviceInformation>&,
        const InterfaceAddress::Interface&
    );

    void run();
}
