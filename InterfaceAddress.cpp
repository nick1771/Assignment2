#include "InterfaceAddress.h"

#include <stdexcept>
#include <vector>
#include <algorithm>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <ifaddrs.h>

namespace {

    struct InterfaceList {
        std::vector<InterfaceAddress::Interface> interfaces{};

        InterfaceList(ifaddrs* interfaceInfo) {
            for (auto current = interfaceInfo; current != nullptr; current = current->ifa_next) {
                auto validFlags = (current->ifa_flags & (IFF_UP | IFF_RUNNING | IFF_LOOPBACK)) == (IFF_UP | IFF_RUNNING);
                if (!validFlags || current->ifa_addr == nullptr) {
                    continue;
                }

                if (current->ifa_addr->sa_family == AF_INET) {
                    auto& interface = findOrInsertInterfaceByName(current->ifa_name);

                    auto addressIn = reinterpret_cast<sockaddr_in*>(current->ifa_addr);
                    interface.ip = inet_ntoa(addressIn->sin_addr);
                } else if (current->ifa_addr->sa_family == AF_PACKET) {
                    auto& interface = findOrInsertInterfaceByName(current->ifa_name);

                    auto addressLl = reinterpret_cast<sockaddr_ll*>(current->ifa_addr);
                    int addressLlLength = addressLl->sll_halen;

                    interface.mac.resize(INET6_ADDRSTRLEN);

                    int macAddressLength = 0;
                    for (auto index = 0; index < addressLlLength; index++) {
                        auto seperator = index < addressLlLength - 1 ? ":" : "";
                        macAddressLength += sprintf(interface.mac.data() + macAddressLength, "%02X%s", addressLl->sll_addr[index], seperator);
                    }

                    interface.mac.erase(macAddressLength);
                }
            }
        }

        InterfaceAddress::Interface& findOrInsertInterfaceByName(std::string_view name) {
            auto position = std::find_if(interfaces.begin(), interfaces.end(), [name](auto& interface) {
                return interface.name == name;
            });

            if (position != interfaces.end()) {
                return *position;
            }

            InterfaceAddress::Interface interface{};
            interface.name = name;

            interfaces.push_back(interface);
            return interfaces.back();
        }

        std::optional<InterfaceAddress::Interface> findFirstCompleteAddress() {
            auto position = std::find_if(interfaces.begin(), interfaces.end(), [](auto& interface){
                auto ipValid = interface.ip.find_first_not_of(" \t\n\v\f\r\0");
                auto macValid = interface.mac.find_first_not_of(" \t\n\v\f\r\0");

                return ipValid != std::string::npos && macValid != std::string::npos;
            });

            if (position != interfaces.end()) {
                return { *position };
            }

            return {};
        }
    };
}

std::optional<InterfaceAddress::Interface> InterfaceAddress::findPrimaryInterfaceAddressInfo() {
    ifaddrs* interfaceInfo{};

    if (getifaddrs(&interfaceInfo) == -1) {
        freeifaddrs(interfaceInfo);
        throw std::runtime_error("Failed to query interface addresses information");
    }

    InterfaceList interfaces{ interfaceInfo };
    freeifaddrs(interfaceInfo);

    return interfaces.findFirstCompleteAddress();
}
