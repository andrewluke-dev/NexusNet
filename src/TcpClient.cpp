
#include "TcpClient.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

TcpClient::TcpClient() = default;

TcpClient::~TcpClient() {
    disconnect();
}

bool TcpClient::connectToServer(
    const std::string& host,
    std::uint16_t port
) {
    if (connected_) {
        return false;
    }

    if (receiverThread_.joinable()) {
        receiverThread_.join();
    }

    socket_ = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_ < 0) {
        std::cerr << "[ERROR] Cannot create client socket\n";
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(
            AF_INET,
            host.c_str(),
            &address.sin_addr
        ) != 1) {
        std::cerr << "[ERROR] Invalid IPv4 address\n";
        close(socket_);
        socket_ = -1;
        return false;
    }

    if (connect(
            socket_,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)
        ) < 0) {
        std::cerr << "[ERROR] Connection failed: "
                  << std::strerror(errno) << '\n';

        close(socket_);
        socket_ = -1;
        return false;
    }

    connected_ = true;

    std::cout << "[NexusNet] Connected to "
              << host << ':' << port << '\n';

    receiverThread_ = std::thread(
        &TcpClient::receiveMessages,
        this
    );

    return true;
}

bool TcpClient::sendMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(sendMutex_);

    if (!connected_) {
        return false;
    }

    const std::string data = message + "\n";
    std::size_t sent = 0;

    while (sent < data.size()) {
        ssize_t result = send(
            socket_,
            data.data() + sent,
            data.size() - sent,
            MSG_NOSIGNAL
        );

        if (result < 0 && errno == EINTR) {
            continue;
        }

        if (result <= 0) {
            connected_ = false;
            return false;
        }

        sent += static_cast<std::size_t>(result);
    }

    return true;
}

void TcpClient::receiveMessages() {
    char buffer[1024];

    while (connected_) {
        ssize_t received = recv(
            socket_,
            buffer,
            sizeof(buffer),
            0
        );

        if (received > 0) {
            std::cout.write(buffer, received);
            std::cout.flush();
        } else if (received < 0 && errno == EINTR) {
            continue;
        } else {
            break;
        }
    }

    connected_ = false;

    std::cout << "\n[NexusNet] Disconnected from server\n";
}

void TcpClient::disconnect() {
    connected_ = false;

    if (socket_ >= 0) {
        shutdown(socket_, SHUT_RDWR);
    }

    if (receiverThread_.joinable()) {
        receiverThread_.join();
    }

    if (socket_ >= 0) {
        close(socket_);
        socket_ = -1;
    }
}

bool TcpClient::isConnected() const {
    return connected_;
}
