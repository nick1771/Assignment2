#include <cstring>

#include <stdexcept>
#include <iostream>

#include "BackgroundProcess.h"
#include "LocalSocket.h"

namespace {

    void getDeviceList() {
        try {
            LocalSocket localSocket{ LocalSocket::Usage::Client };
            localSocket.send(BackgroundProcess::Messages::LIST_DEVICES);

            auto deviceListMessage = localSocket.read();
            std::cout << deviceListMessage.text << std::endl;
        } catch (const std::runtime_error& e) {
            std::cout << "Encountered an error while getting devices: " << e.what() << std::endl;
        }
    }

    void stopBackgroundProcess() {
        try {
            LocalSocket localSocket{ LocalSocket::Usage::Client };
            localSocket.send(BackgroundProcess::Messages::EXIT);
        } catch (const std::runtime_error& e) {
            std::cout << "Encountered an error while stopping background process: " << e.what() << std::endl;
        }
    }

    void printHelp() {
        std::cout << "Usage:" << std::endl;
        std::cout << "  --start Starts the background process" << std::endl;
        std::cout << "  --exit  Attemtns to stop the background process" << std::endl;
        std::cout << "  --list  Prints out neighbour devices" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1 || argc > 2) {
        printHelp();
        return 0;
    }

    if (strcmp(argv[1], "--start") == 0) {
        BackgroundProcess::run();
    } else if (strcmp(argv[1], "--list") == 0) {
        getDeviceList();
    } else if (strcmp(argv[1], "--exit") == 0) {
        stopBackgroundProcess();
    } else {
        printHelp();
    }
}
