
#include "TcpServer.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::uint16_t port = 5555;

    if (argc > 1) {
        try {
            const int value = std::stoi(argv[1]);

            if (value < 1 || value > 65535) {
                std::cerr << "Invalid port number.\n";
                return 1;
            }

            port = static_cast<std::uint16_t>(value);
        } catch (const std::exception&) {
            std::cerr << "Usage: nexusnet_server [port]\n";
            return 1;
        }
    }

    std::cout << "================================\n";
    std::cout << "       NEXUSNET SERVER\n";
    std::cout << "================================\n";

    TcpServer server(port);

    if (!server.start()) {
        std::cerr << "[ERROR] Server failed to start.\n";
        return 1;
    }

    return 0;
}
