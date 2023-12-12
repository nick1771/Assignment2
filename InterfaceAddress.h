#include <string>
#include <optional>

namespace InterfaceAddress {

    struct Interface {
        std::string name{};
        std::string ip{};
        std::string mac{};
    };

    std::optional<Interface> findPrimaryInterfaceAddressInfo();
}
