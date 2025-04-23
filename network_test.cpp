#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        return 1;
    }
    std::cout << "Socket created successfully\n";

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Setsockopt failed: " << strerror(errno) << std::endl;
        return 1;
    }

    // Configure address
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8081);

    // Bind
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        return 1;
    }
    std::cout << "Bind successful on port 8081\n";

    // Listen
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed: " << strerror(errno) << std::endl;
        return 1;
    }
    std::cout << "Listening for connections...\n";

    // Accept one connection
    int new_socket = accept(server_fd, nullptr, nullptr);
    if (new_socket < 0) {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        return 1;
    }
    std::cout << "Connection accepted!\n";

    // Send a test message
    const char* msg = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from Raspberry Pi!\n";
    send(new_socket, msg, strlen(msg), 0);

    // Cleanup
    close(new_socket);
    close(server_fd);
    return 0;
}