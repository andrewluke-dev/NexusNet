
#include "TcpClient.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    std::uint16_t port = 5555;

    if (argc > 1) {
        host = argv[1];
    }

    if (argc > 2) {
        try {
            const int value = std::stoi(argv[2]);

            if (value < 1 || value > 65535) {
                std::cerr << "Invalid port number.\n";
                return 1;
            }

            port = static_cast<std::uint16_t>(value);
        } catch (const std::exception&) {
            std::cerr << "Usage: nexusnet_client [host] [port]\n";
            return 1;
        }
    }

    std::cout << "================================\n";
    std::cout << "       NEXUSNET CLIENT\n";
    std::cout << "================================\n";

    TcpClient client;

    if (!client.connectToServer(host, port)) {
        return 1;
    }

    std::cout << "Type your messages and press Enter.\n";
    std::cout << "Type /quit to disconnect.\n\n";

    std::string message;

    while (client.isConnected() && std::getline(std::cin, message)) {
        if (message == "/quit") {
            break;
        }

        if (!message.empty()) {
            if (!client.sendMessage(message)) {
                std::cerr << "[ERROR] Message could not be sent.\n";
                break;
            }
        }
    }

    client.disconnect();

    std::cout << "[NexusNet] Client closed.\n";

    return 0;
}
