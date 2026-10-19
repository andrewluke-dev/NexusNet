
#include "TcpClient.hpp"
#include "TcpServer.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "[FAIL] " << message << '\n';
        std::exit(EXIT_FAILURE);
    }

    std::cout << "[PASS] " << message << '\n';
}

int main() {
    std::cout << "=== NexusNet Automated Tests ===\n";

    // Test 1: Initial client state
    TcpClient client;
    require(!client.isConnected(),
            "Client starts disconnected");

    // Test 2: Initial server state
    TcpServer server(5555);
    require(!server.isRunning(),
            "Server starts stopped");

    // Test 3: Invalid IPv4 address
    require(!client.connectToServer("invalid-ip", 5555),
            "Invalid IP address is rejected");

    // Test 4: Connection to an unavailable local port
    // Port 0 cannot be a normal TCP destination.
    require(!client.connectToServer("127.0.0.1", 0),
            "Connection to port zero fails");

    // Test 5: Cannot send while disconnected
    require(!client.sendMessage("Hello"),
            "Disconnected client cannot send");

    // Test 6: Disconnecting an idle client
    client.disconnect();
    require(!client.isConnected(),
            "Disconnect keeps client disconnected");

    // Test 7: Repeated disconnect
    client.disconnect();
    require(!client.isConnected(),
            "Repeated disconnect is safe");

    // Test 8: Sending after disconnect
    require(!client.sendMessage("Test"),
            "Sending after disconnect fails");

    std::cout << "\nAll 8 tests passed!\n";
    return EXIT_SUCCESS;
}
