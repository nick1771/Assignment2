#include <string_view>

namespace BackgroundProcess::Messages {

    inline constexpr std::string_view EXIT = "Exit";
    inline constexpr std::string_view LIST_DEVICES = "ListDevices";
}

namespace BackgroundProcess {

    void run();
}
