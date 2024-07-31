#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

constexpr int BUFFER_SIZE = 1024;
constexpr int PORT = 2971;
const char* ADDRESS = "192.168.0.5";

int main() {
    struct addrinfo *res;
    struct addrinfo hints = { // type of socket that is supported
        .ai_family = AF_INET, // IPv4
        .ai_socktype = SOCK_STREAM // TCP
    };
    
    int status = getaddrinfo(ADDRESS, std::to_string(PORT).c_str(), &hints, &res); // populates res
    if (status != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
        return -1;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol); // Create socket
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        freeaddrinfo(res); // Free the address info structure on error
        return -1;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) { // Connect to the server using the resolved address
        std::cerr << "Connection Failed" << std::endl;
        close(sock); // Close the socket on error
        freeaddrinfo(res); // Free the address info structure on error
        return -1;
    }

    freeaddrinfo(res); // Free the address info structure after use
    std::string message;
    char buffer[BUFFER_SIZE] = {0};
    while (true) {
        std::cout << "Enter message: ";
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        send(sock, message.c_str(), message.length(), 0);
        std::cout << "Message sent" << std::endl;

        int valread = read(sock, buffer, BUFFER_SIZE);
        if (valread > 0) {
            std::cout << "Server: " << std::string(buffer, valread) << std::endl;
        } else if (valread < 0) {
            std::cerr << "Read error" << std::endl;
        }
    }

    close(sock);
    return 0;
}
