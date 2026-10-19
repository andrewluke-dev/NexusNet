
#include "TcpServer.hpp"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

TcpServer::TcpServer(std::uint16_t port)
    : port_(port) {}

TcpServer::~TcpServer() {
    stop();
}

bool TcpServer::sendAll(
    const ClientPtr& client,
    const std::string& message
) {
    std::lock_guard<std::mutex> lock(client->sendMutex);

    if (!client->active) {
        return false;
    }

    std::size_t sent = 0;

    while (sent < message.size()) {
        const ssize_t result = send(
            client->socket,
            message.data() + sent,
            message.size() - sent,
            MSG_NOSIGNAL | MSG_DONTWAIT
        );

        if (result < 0 && errno == EINTR) {
            continue;
        }

        if (result < 0 &&
            (errno == EAGAIN || errno == EWOULDBLOCK)) {
            pollfd descriptor{};
            descriptor.fd = client->socket;
            descriptor.events = POLLOUT;

            const int ready = poll(&descriptor, 1, 200);

            if (ready > 0 && (descriptor.revents & POLLOUT)) {
                continue;
            }

            return false;
        }

        if (result <= 0) {
            return false;
        }

        sent += static_cast<std::size_t>(result);
    }

    return true;
}

bool TcpServer::start() {
    if (running_) {
        return false;
    }

    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket_ < 0) {
        std::cerr << "[ERROR] Cannot create socket\n";
        return false;
    }

    int option = 1;

    setsockopt(
        serverSocket_,
        SOL_SOCKET,
        SO_REUSEADDR,
        &option,
        sizeof(option)
    );

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port_);

    if (bind(
        serverSocket_,
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address)
    ) < 0) {
        std::cerr << "[ERROR] Bind failed: "
                  << std::strerror(errno) << '\n';

        close(serverSocket_);
        serverSocket_ = -1;
        return false;
    }

    if (listen(serverSocket_, 10) < 0) {
        std::cerr << "[ERROR] Listen failed\n";

        close(serverSocket_);
        serverSocket_ = -1;
        return false;
    }

    running_ = true;

    std::cout << "[NexusNet] Server running on "
              << "127.0.0.1:" << port_ << '\n';

    std::cout << "[NexusNet] Waiting for clients...\n";

    while (running_) {
        pollfd descriptor{};
        descriptor.fd = serverSocket_;
        descriptor.events = POLLIN;

        const int ready = poll(&descriptor, 1, 200);

        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        if (ready == 0) {
            continue;
        }

        if (!(descriptor.revents & POLLIN)) {
            break;
        }

        sockaddr_in clientAddress{};
        socklen_t clientLength = sizeof(clientAddress);

        const int fd = accept(
            serverSocket_,
            reinterpret_cast<sockaddr*>(&clientAddress),
            &clientLength
        );

        if (fd < 0) {
            if (errno != EINTR && running_) {
                std::cerr << "[ERROR] Accept failed\n";
            }
            continue;
        }

        auto client = std::make_shared<ClientConnection>(fd);

        {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            clients_.push_back(client);
        }

        std::cout << "[NexusNet] Client connected\n";

        clientThreads_.emplace_back(
            &TcpServer::handleClient,
            this,
            client
        );
    }

    stop();
    return true;
}

void TcpServer::handleClient(ClientPtr client) {
    sendAll(client, "Welcome to NexusNet!\n");

    broadcast(
        "[SERVER] A new client joined.\n",
        client->socket
    );

    char buffer[1024];
    std::string pending;

    while (running_ && client->active) {
        pollfd descriptor{};
        descriptor.fd = client->socket;
        descriptor.events = POLLIN;

        const int ready = poll(&descriptor, 1, 200);

        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        if (ready == 0) {
            continue;
        }

        if (!(descriptor.revents & POLLIN)) {
            break;
        }

        const ssize_t received = recv(
            client->socket,
            buffer,
            sizeof(buffer),
            0
        );

        if (received < 0 && errno == EINTR) {
            continue;
        }

        if (received <= 0) {
            break;
        }

        pending.append(
            buffer,
            static_cast<std::size_t>(received)
        );

        std::size_t newline = 0;

        while ((newline = pending.find('\n'))
               != std::string::npos) {
            std::string message = pending.substr(0, newline);
            pending.erase(0, newline + 1);

            if (!message.empty() && message.back() == '\r') {
                message.pop_back();
            }

            if (message == "/quit") {
                removeClient(client);
                return;
            }

            if (!message.empty()) {
                std::cout << "[MESSAGE] " << message << '\n';

                broadcast(
                    "[CLIENT] " + message + "\n",
                    client->socket
                );
            }
        }

        if (pending.size() > 8192) {
            break;
        }
    }

    removeClient(client);
}

void TcpServer::broadcast(
    const std::string& message,
    int senderSocket
) {
    std::vector<ClientPtr> recipients;

    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        recipients = clients_;
    }

    for (const auto& client : recipients) {
        if (client->socket != senderSocket && client->active) {
            sendAll(client, message);
        }
    }
}

void TcpServer::removeClient(const ClientPtr& client) {
    if (!client->active.exchange(false)) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(clientsMutex_);

        clients_.erase(
            std::remove(
                clients_.begin(),
                clients_.end(),
                client
            ),
            clients_.end()
        );
    }

    shutdown(client->socket, SHUT_RDWR);

    {
        std::lock_guard<std::mutex> lock(client->sendMutex);
        close(client->socket);
    }

    std::cout << "[NexusNet] Client disconnected\n";

    if (running_) {
        broadcast("[SERVER] A client left.\n", -1);
    }
}

void TcpServer::stop() {
    running_ = false;

    if (serverSocket_ >= 0) {
        shutdown(serverSocket_, SHUT_RDWR);
        close(serverSocket_);
        serverSocket_ = -1;
    }

    std::vector<ClientPtr> snapshot;

    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        snapshot = clients_;
    }

    for (const auto& client : snapshot) {
        shutdown(client->socket, SHUT_RDWR);
    }

    for (auto& thread : clientThreads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    clientThreads_.clear();

    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        clients_.clear();
    }
}

bool TcpServer::isRunning() const {
    return running_;
}
