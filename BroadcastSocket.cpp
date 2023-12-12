#include "BroadcastSocket.h"

#include <stdexcept>
#include <string>

#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/syslog.h>
#include <unistd.h>

namespace {

    constexpr auto RECEIVE_BUFFER_SIZE = 512;
}

BroadcastSocket::BroadcastSocket(BroadcastSocket::Usage usage) {
    _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    _descriptor.sin_family = AF_INET;
    _descriptor.sin_port = htons(PORT);

    auto address = usage == Usage::Listener ? INADDR_ANY : INADDR_BROADCAST;
    _descriptor.sin_addr.s_addr = htonl(address);

    if (usage == Usage::Broadcast) {
        auto enabled = 1;
        if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled)) == -1) {
            throw std::runtime_error("Failed to update socket permissions for broadcast");
        };
    }

    if (usage == Usage::Listener) {
        auto errorCode = bind(_socket, reinterpret_cast<sockaddr*>(&_descriptor), sizeof(sockaddr_in));
        if (errorCode != 0) {
            throw std::runtime_error("Failed to bind local socket with error: " + std::to_string(errorCode));
        }
    }

    auto enabled = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) == -1) {
        throw std::runtime_error("Failed to update socket reuse option");
    };
}

BroadcastSocket::~BroadcastSocket() {
    close(_socket);
}

void BroadcastSocket::send(std::string_view text) {
    auto result = sendto(_socket, text.data(), text.size(), 0, 
        reinterpret_cast<sockaddr*>(&_descriptor), sizeof(sockaddr_in));
    if (result == -1) {
        throw std::runtime_error("Failed to send broadcast message: " + std::to_string(result));
    }
}

std::string BroadcastSocket::read() {
    std::string message{};
    message.resize(RECEIVE_BUFFER_SIZE);

    recvfrom(_socket, message.data(), RECEIVE_BUFFER_SIZE, 0, nullptr, nullptr);

    auto textEnd = message.find_first_of('\0');
    if (textEnd != std::string::npos) {
        message.erase(textEnd);
    }

    return message;
}

bool BroadcastSocket::isReadable() {
    pollfd request{};
    request.fd = _socket;
    request.events = POLLIN;

    if (poll(&request, 1, 50) == -1) {
        throw std::runtime_error("Poll failed for local socket");
    }

    return (request.revents & POLLIN) == POLLIN;
}
