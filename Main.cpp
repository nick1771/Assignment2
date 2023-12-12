#include "BackgroundService.h"
#include "LocalSocket.h"
#include "BroadcastSocket.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

namespace {

    void sendBroadcast() {
        try {
            BroadcastSocket broadcaster{ BroadcastSocket::Usage::Broadcast };
            broadcaster.send("M[f8:9e:94:78:d7:42,192.168.90.211]MM[f1:9x:94:78:d7:89,111.111.111.111]M");
        } catch (const std::runtime_error& e) {
            std::cout << "Encountered an error while broadcasting: " << e.what() << std::endl;
        }
    }

    void getDeviceList() {
        try {
            LocalSocket localSocket{ LocalSocket::Usage::Client };
            localSocket.send(BackgroundService::Messages::LIST_DEVICES);

            auto deviceListMessage = localSocket.read();
            auto deviceList = BackgroundService::parseBroadcastDeviceList(deviceListMessage.text);

            for (auto& device : deviceList) {
                std::cout << device.ip << " " << device.mac << std::endl;
            }
        } catch (const std::runtime_error& e) {
            std::cout << "Encountered an error while getting devices: " << e.what() << std::endl;
        }
    }

    void stopBackgroundProcess() {
        try {
            LocalSocket localSocket{ LocalSocket::Usage::Client };
            localSocket.send(BackgroundService::Messages::EXIT);
        } catch (const std::runtime_error& e) {
            std::cout << "Encountered an error while stopping background process: " << e.what() << std::endl;
        }
    }

    void printHelp() {
        std::cout << "Usage:" << std::endl;
        std::cout << "  --start Starts the background process" << std::endl;
        std::cout << "  --stop  Attempts to stop the background process" << std::endl;
        std::cout << "  --list  Prints out neighbour devices" << std::endl;
        std::cout << "  --test  Sends a test broadcast" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1 || argc > 2) {
        printHelp();
        return 0;
    }

    if (strcmp(argv[1], "--start") == 0) {
        BackgroundService::run();
    } else if (strcmp(argv[1], "--list") == 0) {
        getDeviceList();
    } else if (strcmp(argv[1], "--stop") == 0) {
        stopBackgroundProcess();
    } else if (strcmp(argv[1], "--test") == 0) {
        sendBroadcast();
    } else {
        printHelp();
    }
}
