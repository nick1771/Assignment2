#include <netinet/in.h>

#include <string>

class LocalSocket {
public:
    static constexpr auto PORT = 7500;
    static constexpr auto ADDRESS = "127.0.0.1";

    enum class Usage {
        Client,
        Server,
    };

    struct Message {
        sockaddr_in sender{};
        std::string text{};
    };

    LocalSocket(Usage);
    ~LocalSocket();

    void sendTo(std::string_view, sockaddr_in&);
    void send(std::string_view);

    Message read();
    bool isReadable();
private:
    int _socket{};
    sockaddr_in _descriptor{};
};
