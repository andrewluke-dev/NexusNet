# NexusNet

**Multithreaded TCP Chat Application | C++17**

NexusNet is a command-line client-server messaging application built with modern C++17 and Linux TCP sockets.

The project demonstrates network programming, concurrent client handling, thread synchronization, socket communication, and modular software architecture.

## Features

- TCP server supporting multiple connected clients
- Real-time text messaging between clients
- Multithreaded connection handling with `std::thread`
- Thread synchronization using `std::mutex`
- Message broadcasting to connected clients
- Client connection and disconnection management
- Localhost networking (`127.0.0.1`)
- CMake build system and CTest integration
- Automated checks for basic client and server behavior

## Technology Stack

| Technology | Purpose |
|---|---|
| C++17 | Application development |
| POSIX Sockets | TCP networking |
| STL | Containers and strings |
| std::thread | Concurrent client handling |
| std::mutex | Thread synchronization |
| CMake | Build system |
| CTest | Automated testing |
| GCC / G++ | Compilation |
| Ubuntu WSL2 | Development environment |

## Project Structure

```text
NexusNet/
├── include/
│   ├── TcpServer.hpp
│   └── TcpClient.hpp
├── src/
│   ├── TcpServer.cpp
│   ├── TcpClient.cpp
│   ├── server_main.cpp
│   └── client_main.cpp
├── tests/
│   └── test_nexusnet.cpp
├── CMakeLists.txt
├── .gitignore
└── README.md
```

## Requirements

- Linux or Ubuntu WSL2
- GCC or another C++17-compatible compiler
- CMake 3.16+
- POSIX socket support

## Installation

Clone the repository:

```bash
git clone https://github.com/andrewluke-dev/NexusNet.git
cd NexusNet
```

Configure and compile:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j 4
```

## Running the Application

Start the server in the first terminal:

```bash
./build/nexusnet_server
```

The server listens on `127.0.0.1:5555`.

Start a client in another terminal:

```bash
./build/nexusnet_client
```

Open additional terminals and repeat the client command to connect more clients.

Type a message and press Enter to send it to the other connected clients.

Type `/quit` to disconnect.

## Example

**Client 1:**

```text
[NexusNet] Connected to 127.0.0.1:5555
Welcome to NexusNet!
Hello from Client 1!
[CLIENT] Hello from Client 2!
```

**Client 2:**

```text
[NexusNet] Connected to 127.0.0.1:5555
Welcome to NexusNet!
[CLIENT] Hello from Client 1!
Hello from Client 2!
```

## Architecture

**TcpServer** accepts incoming TCP connections and manages connected clients.

**TcpClient** establishes TCP connections, sends messages, and receives incoming data using a dedicated receiver thread.

The server uses separate threads for client connections and mutexes to synchronize shared resources and message transmission.

TCP messages are separated using newline delimiters.

## Automated Testing

Build the test executable and run CTest:

```bash
cmake --build build -j 4
ctest --test-dir build --output-on-failure
```

Run the test executable directly:

```bash
./build/nexusnet_tests
```

The initial test suite includes eight checks covering connection state, invalid addresses, failed connections, and disconnected-client behavior.

## Security and Limitations

NexusNet is an educational networking project intended for local development.

- The server binds to the loopback address only.
- Messages are not encrypted.
- Client authentication is not implemented.
- Graceful shutdown via operating-system signals is not yet implemented.
- Production-grade connection limits, timeouts, and load testing are not included.

Do not expose this version directly to the public internet.

## Future Improvements

- Usernames and chat rooms
- Structured message protocol
- Connection timeouts
- Graceful signal handling
- End-to-end integration tests
- TLS encryption
- GitHub Actions CI
- Cross-platform networking support

## Author

**Andrew Luke**

Software Developer

GitHub: https://github.com/andrewluke-dev

## License

No license has been specified for this project.