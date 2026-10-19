
#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class TcpServer {
public:
    explicit TcpServer(std::uint16_t port = 5555);
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    bool start();
    void stop();
    bool isRunning() const;

private:
    struct ClientConnection {
        explicit ClientConnection(int fd) : socket(fd) {}

        int socket;
        std::mutex sendMutex;
        std::atomic<bool> active{true};
    };

    using ClientPtr = std::shared_ptr<ClientConnection>;

    void handleClient(ClientPtr client);
    void broadcast(const std::string& message, int senderSocket);
    void removeClient(const ClientPtr& client);

    static bool sendAll(
        const ClientPtr& client,
        const std::string& message
    );

    std::uint16_t port_;
    int serverSocket_ = -1;
    std::atomic<bool> running_{false};

    std::mutex clientsMutex_;
    std::vector<ClientPtr> clients_;
    std::vector<std::thread> clientThreads_;
};
