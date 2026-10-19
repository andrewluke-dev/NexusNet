
#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

class TcpClient {
public:
    TcpClient();
    ~TcpClient();

    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;

    bool connectToServer(
        const std::string& host = "127.0.0.1",
        std::uint16_t port = 5555
    );

    bool sendMessage(const std::string& message);
    void disconnect();
    bool isConnected() const;

private:
    void receiveMessages();

    int socket_ = -1;
    std::atomic<bool> connected_{false};
    std::thread receiverThread_;
    std::mutex sendMutex_;
};
