#include "LocalSocket.h"

#include <stdexcept>

#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {

    constexpr auto RECEIVE_BUFFER_SIZE = 12;
}

LocalSocket::LocalSocket(Usage usage) {
    _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    _descriptor.sin_family = AF_INET;
    _descriptor.sin_port = PORT;
    _descriptor.sin_addr.s_addr = inet_addr(ADDRESS);

    if (usage == Usage::Client) {
        return;
    }

    auto errorCode = bind(_socket, reinterpret_cast<sockaddr*>(&_descriptor), sizeof(sockaddr_in));
    if (errorCode != 0) {
        throw std::runtime_error("Failed to bind local socket with error: " + std::to_string(errorCode));
    }
}

LocalSocket::~LocalSocket() {
    close(_socket);
}

void LocalSocket::sendTo(std::string_view text, sockaddr_in& target) {
    sendto(_socket, text.data(), text.size(), 0, 
        reinterpret_cast<sockaddr*>(&target), sizeof(_descriptor));
}

void LocalSocket::send(std::string_view text) {
    sendto(_socket, text.data(), text.size(), 0, 
        reinterpret_cast<sockaddr*>(&_descriptor), sizeof(sockaddr_in));
}

LocalSocket::Message LocalSocket::read() {
    Message message{};
    message.text.resize(RECEIVE_BUFFER_SIZE);

    socklen_t size = sizeof(sockaddr_in);
    recvfrom(_socket, message.text.data(), message.text.capacity(), 0, 
        reinterpret_cast<sockaddr*>(&message.sender), &size);

    auto textEnd = message.text.find_first_of('\0');
    message.text.erase(textEnd);

    return message;
}

bool LocalSocket::isReadable() {
    pollfd request{};
    request.fd = _socket;
    request.events = POLLIN;

    if (poll(&request, 1, 50) == -1) {
        throw std::runtime_error("Poll failed for local socket");
    }

    return request.revents & POLLIN;
}
