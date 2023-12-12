#include <cstdint>
#include <netinet/in.h>
#include <string>

class BroadcastSocket {
public:
    static constexpr std::uint16_t PORT = static_cast<std::uint16_t>(75013);

    enum class Usage {
        Broadcast,
        Listener,
    };

    BroadcastSocket(Usage);
    ~BroadcastSocket();

    void send(std::string_view);

    std::string read();

    bool isReadable();
private:
    int _socket{};
    sockaddr_in _descriptor{};
};
