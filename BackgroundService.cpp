#include "BackgroundService.h"
#include "BroadcastSocket.h"
#include "LocalSocket.h"

#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <string>

#include <string_view>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/types.h>

#include <ifaddrs.h>
#include <thread>
#include <unistd.h>
#include <net/if.h>
#include <vector>

namespace {

    constexpr std::string_view BROADCAST_HEADER = "M[";
    constexpr std::string_view BROADCAST_FOOTER = "]M";

    constexpr auto TIME_TO_LIVE = 30.f;
}

std::vector<BackgroundService::DeviceInformation> BackgroundService::parseBroadcastDeviceList(std::string_view message) {
    std::vector<BackgroundService::DeviceInformation> result{};
    auto deviceInfoStart = message.find(BROADCAST_HEADER);
    
    while (deviceInfoStart != std::string::npos) {
        auto macAddressStart = deviceInfoStart + BROADCAST_HEADER.length();

        auto macAddressEnd = message.find(',', macAddressStart);
        auto ipAddressEnd = message.find(BROADCAST_FOOTER, macAddressEnd);

        if (macAddressEnd == std::string::npos || ipAddressEnd == std::string::npos) {
            break;
        }

        BackgroundService::DeviceInformation information{};
        information.mac = message.substr(macAddressStart, macAddressEnd - macAddressStart);
        information.ip = message.substr(macAddressEnd + 1, ipAddressEnd - macAddressEnd - 1);
        information.timeToLive = TIME_TO_LIVE;

        result.push_back(information);
        
        deviceInfoStart = message.find(BROADCAST_HEADER, ipAddressEnd);
    }

    return result;
}

void BackgroundService::updateDeviceInformation(
    std::vector<BackgroundService::DeviceInformation>& devices, 
    const std::vector<BackgroundService::DeviceInformation>& broadcastDevices,
    const InterfaceAddress::Interface& localAddress
) {
    auto removeStart = std::remove_if(devices.begin(), devices.end(), [](auto& device) {
        device.timeToLive -= 1.f;
        return device.timeToLive <= 0.f;
    });

    if (removeStart != devices.end()) {
        devices.erase(removeStart);
    }

    for (auto& broadcastDevice : broadcastDevices) {
        if (broadcastDevice == localAddress) {
            continue;
        }

        auto existingDevice = std::find(devices.begin(), devices.end(), broadcastDevice);
        if (existingDevice != devices.end()) {
            existingDevice->timeToLive = TIME_TO_LIVE;
        } else {
            syslog(LOG_NOTICE, "Found new device %s", broadcastDevice.mac.c_str());
            devices.push_back(broadcastDevice);
        }
    }
}

namespace {

    void daemonize() {
        auto pid = fork();
        if (pid < 0) {
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            exit(EXIT_SUCCESS);
        }

        if (setsid() < 0) {
            exit(EXIT_FAILURE);
        }

        pid = fork();
        if (pid < 0) {
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            exit(EXIT_SUCCESS);
        }

        umask(0);
        chdir("/");

        auto maximumOpenFiles = sysconf(_SC_OPEN_MAX);
        for (auto currentFile = 0; currentFile <= maximumOpenFiles; currentFile++) {
            close(currentFile);
        }
    }

    std::string createBroadcastMessageFromInterface(const InterfaceAddress::Interface& interface) {        
        std::string message{};

        message.append(BROADCAST_HEADER);
        message.append(interface.mac);
        message.append(",");
        message.append(interface.ip);
        message.append(BROADCAST_FOOTER);

        return message;
    }

    std::string formatDeviceListToMessage(const std::vector<BackgroundService::DeviceInformation>& devices) {
        std::string message{};

        for (auto& device : devices) {
            message.append(BROADCAST_HEADER);
            message.append(device.mac);
            message.append(",");
            message.append(device.ip);
            message.append(BROADCAST_FOOTER);
        }

        return message;
    }

    bool handleLocalHostMessages(
        LocalSocket& socket, 
        const std::vector<BackgroundService::DeviceInformation>& devices
    ) {
        if (!socket.isReadable()) {
            return false;
        }

        auto message = socket.read();
        syslog(LOG_NOTICE, "Received message from localhost: %s", message.text.c_str());

        if (message.text == BackgroundService::Messages::LIST_DEVICES) {
            auto responseMessage = formatDeviceListToMessage(devices);
            syslog(LOG_NOTICE, "Sending device list response: %s", responseMessage.c_str());

            socket.sendTo(responseMessage, message.sender);
        } else if (message.text == BackgroundService::Messages::EXIT) {
            syslog(LOG_NOTICE, "Shutting down");
            return true;
        }

        return false;
    }

    void handleBroadcastMessages(
        BroadcastSocket& socket, 
        std::vector<BackgroundService::DeviceInformation>& devices, 
        const InterfaceAddress::Interface& localAddress
    ) {
        if (!socket.isReadable()) {
            return;
        }

        auto message = socket.read();
        syslog(LOG_NOTICE, "Received broadcast message: %s", message.c_str());

        auto broadcastDevices = BackgroundService::parseBroadcastDeviceList(message);
        updateDeviceInformation(devices, broadcastDevices, localAddress);

        syslog(LOG_NOTICE, "Device count after device update: %zu", devices.size());
    }

    void serviceMain() {
        LocalSocket localSocket{ LocalSocket::Usage::Server };
        syslog(LOG_NOTICE, "Created socket for local host communication");

        BroadcastSocket broadcastListener{ BroadcastSocket::Usage::Listener };
        syslog(LOG_NOTICE, "Created socket for broadcast listening");

        BroadcastSocket broadcaster{ BroadcastSocket::Usage::Broadcast };
        syslog(LOG_NOTICE, "Created socket for broadcasting");

        syslog(LOG_NOTICE, "Started");

        std::vector<BackgroundService::DeviceInformation> connectedDevices{};

        while (true) {
            auto primaryInterface = InterfaceAddress::findPrimaryInterfaceAddressInfo();
            if (primaryInterface.has_value()) {
                auto message = createBroadcastMessageFromInterface(primaryInterface.value());
                syslog(LOG_NOTICE, "Sending broadcast: %s", message.c_str());

                broadcaster.send(message);
            }

            handleBroadcastMessages(broadcastListener, connectedDevices, primaryInterface.value());
            auto isExitRequired = handleLocalHostMessages(localSocket, connectedDevices);

            if (isExitRequired) {
                return;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void BackgroundService::run() {
    daemonize();

    openlog("DeviceBackgroundService", LOG_PID, LOG_DAEMON);

    try {
        serviceMain();
    } catch (const std::runtime_error& e) {
        syslog(LOG_ERR, "%s", e.what());
    }

    closelog();
}
